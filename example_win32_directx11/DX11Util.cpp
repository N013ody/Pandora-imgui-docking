#define NANOSVG_IMPLEMENTATION 
#define NANOSVGRAST_IMPLEMENTATION 

#include "DX11Util.h"
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include "Profile.h"
#include <wrl/client.h> 
#include <filesystem>
#include <nanosvg.h>
#include <nanosvgrast.h>

using Microsoft::WRL::ComPtr;  
using namespace Window;


SVGTexture GetSVGTexture(ID3D11Device* device, NSVGimage* svgImage, float scale, int scaledWidth, int scaledHeight, bool release) {
    //�������ػ�����
    std::vector<unsigned char> pixels(scaledWidth * scaledHeight * 4);
    NSVGrasterizer* rasterizer = nsvgCreateRasterizer();
    //��դ�������ź�ĳߴ�
    nsvgRasterize(rasterizer, svgImage, 0, 0, scale, pixels.data(), scaledWidth, scaledHeight, scaledWidth * 4);
    nsvgDeleteRasterizer(rasterizer);


    //����DX11����
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = scaledWidth;
    desc.Height = scaledHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = (scaledWidth * 4); //128�ֽڶ���

    ComPtr<ID3D11Texture2D> texture;
    HRESULT hr = device->CreateTexture2D(&desc, &initData, &texture);
    if (FAILED(hr)) {
        nsvgDelete(svgImage);
        return {};
    }

    SVGTexture result;
    hr = device->CreateShaderResourceView(texture.Get(), nullptr, &result.srv);
    if (FAILED(hr)) {
        return {};
    }

    result.size = ImVec2(static_cast<float>(scaledWidth), static_cast<float>(scaledHeight));

    if(release)nsvgDelete(svgImage);
    return result;

}

SVGTexture CreateSVGTexture(ID3D11Device* device, NSVGimage* svgImage, float scale,bool release) {

    if (!svgImage || svgImage->width <= 0 || svgImage->height <= 0) {
        return {};
    }

    int width = static_cast<int>(svgImage->width * scale);
    int height = static_cast<int>(svgImage->height * scale);
    if (width == 0 || height == 0) {
        nsvgDelete(svgImage);
        return {};
    }
    SVGTexture result=GetSVGTexture(device,svgImage,scale,width,height,release);
    return result;
}

SVGTexture CreateSVGTexture(ID3D11Device* device, const char* svgPath, float scale,bool release) {
    //�ļ�����
    if (!std::filesystem::exists(svgPath)) {
        return {};
    }

    //SVG����
    NSVGimage* svgImage = nsvgParseFromFile(svgPath, "px", 96.0f);
    if (!svgImage || svgImage->width <= 0 || svgImage->height <= 0) {
        return {};
    }

    //�������ųߴ�
    int width = static_cast<int>(svgImage->width * scale);
    int height = static_cast<int>(svgImage->height * scale);
    if (width == 0 || height == 0) {
        nsvgDelete(svgImage);
        return {};
    }
    SVGTexture result = GetSVGTexture(device, svgImage, scale, width, height, release);
    return result;
}

// 使用WIC加载PNG图片
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")

ImageTexture LoadPNGTexture(ID3D11Device* device, const char* filePath) {
    ImageTexture result = {};
    
    // 检查文件是否存在
    if (!std::filesystem::exists(filePath)) {
        printf("PNG file not found: %s\n", filePath);
        return result;
    }
    
    // 初始化WIC
    IWICImagingFactory* wicFactory = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
    if (FAILED(hr)) {
        printf("Failed to create WIC factory\n");
        return result;
    }
    
    // 创建解码器
    IWICBitmapDecoder* decoder = nullptr;
    wchar_t wFilePath[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, filePath, -1, wFilePath, MAX_PATH);
    hr = wicFactory->CreateDecoderFromFilename(wFilePath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
    if (FAILED(hr)) {
        printf("Failed to create PNG decoder for: %s\n", filePath);
        wicFactory->Release();
        return result;
    }
    
    // 获取第一帧
    IWICBitmapFrameDecode* frame = nullptr;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) {
        decoder->Release();
        wicFactory->Release();
        return result;
    }
    
    // 获取图片尺寸
    UINT width, height;
    frame->GetSize(&width, &height);
    
    // 转换为32位RGBA格式
    IWICFormatConverter* converter = nullptr;
    hr = wicFactory->CreateFormatConverter(&converter);
    if (FAILED(hr)) {
        frame->Release();
        decoder->Release();
        wicFactory->Release();
        return result;
    }
    
    hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) {
        converter->Release();
        frame->Release();
        decoder->Release();
        wicFactory->Release();
        return result;
    }
    
    // 读取像素数据
    std::vector<BYTE> pixels(width * height * 4);
    hr = converter->CopyPixels(nullptr, width * 4, static_cast<UINT>(pixels.size()), pixels.data());
    if (FAILED(hr)) {
        converter->Release();
        frame->Release();
        decoder->Release();
        wicFactory->Release();
        return result;
    }
    
    // 创建D3D11纹理
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pixels.data();
    initData.SysMemPitch = width * 4;
    
    ComPtr<ID3D11Texture2D> texture;
    hr = device->CreateTexture2D(&desc, &initData, &texture);
    if (FAILED(hr)) {
        converter->Release();
        frame->Release();
        decoder->Release();
        wicFactory->Release();
        return result;
    }
    
    // 创建Shader Resource View
    hr = device->CreateShaderResourceView(texture.Get(), nullptr, &result.srv);
    if (SUCCEEDED(hr)) {
        result.size = ImVec2(static_cast<float>(width), static_cast<float>(height));
        result.valid = true;
        printf("Loaded PNG: %s (%dx%d)\n", filePath, width, height);
    }
    
    // 清理
    converter->Release();
    frame->Release();
    decoder->Release();
    wicFactory->Release();
    
    return result;
}

void UnloadTexture(ImageTexture& texture) {
    if (texture.srv) {
        texture.srv->Release();
        texture.srv = nullptr;
    }
    texture.valid = false;
    texture.size = ImVec2(0, 0);
}

void Shutdown(HWND BackendWindow, DirectXData WindowDats) {
    Cleanup(WindowDatas.g_pSwapChain, WindowDatas.g_pd3dDevice, WindowDatas.g_pd3dDeviceContext, WindowDatas.g_mainRenderTargetView);
    CleanupDeviceD3D(WindowDatas.g_pd3dDevice, WindowDatas.g_pSwapChain, WindowDatas.g_pd3dDeviceContext);
    ::DestroyWindow(BackendWindow);
    ::UnregisterClassW(BackendWC.lpszClassName, BackendWC.hInstance);
}


void Cleanup(IDXGISwapChain* swapChain, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11RenderTargetView* renderTargetView) {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}


bool CreateShaderTarget(ID3D11Device* g_pd3dDevice, ID3D11Texture2D** g_pRenderTargetTexture, ID3D11RenderTargetView** g_pRenderTargetView, ID3D11ShaderResourceView** g_pShaderResourceView) {
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width = 800;
    texDesc.Height = 600;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    HRESULT hr = g_pd3dDevice->CreateTexture2D(&texDesc, nullptr, g_pRenderTargetTexture);
    if (FAILED(hr)) {
        return false;
    }

    hr = g_pd3dDevice->CreateRenderTargetView(*g_pRenderTargetTexture, nullptr, g_pRenderTargetView);
    if (FAILED(hr)) {
        if (*g_pRenderTargetTexture) {
            (*g_pRenderTargetTexture)->Release();
            *g_pRenderTargetTexture = nullptr;
        }
        return false;
    }

    hr = g_pd3dDevice->CreateShaderResourceView(*g_pRenderTargetTexture, nullptr, g_pShaderResourceView);
    if (FAILED(hr)) {
        if (*g_pRenderTargetView) {
            (*g_pRenderTargetView)->Release();
            *g_pRenderTargetView = nullptr;
        }
        if (*g_pRenderTargetTexture) {
            (*g_pRenderTargetTexture)->Release();
            *g_pRenderTargetTexture = nullptr;
        }
        return false;
    }

    return true;
}


bool CreateDeviceD3D(HWND hWnd)
{

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &WindowDatas.g_pSwapChain, &WindowDatas.g_pd3dDevice, &featureLevel, &WindowDatas.g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) 
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &WindowDatas.g_pSwapChain, &WindowDatas.g_pd3dDevice, &featureLevel, &WindowDatas.g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget(WindowDatas.g_pd3dDevice, WindowDatas.g_pSwapChain, WindowDatas.g_mainRenderTargetView);
    return true;
}

bool InitDirectX(HWND hwnd,WNDCLASSEXW wc)
{

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D(WindowDatas.g_pd3dDevice, WindowDatas.g_pSwapChain, WindowDatas.g_pd3dDeviceContext);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }


    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(WindowDatas.g_pd3dDevice, WindowDatas.g_pd3dDeviceContext);


}

void CleanupDeviceD3D(ID3D11Device* g_pd3dDevice,IDXGISwapChain* g_pSwapChain, ID3D11DeviceContext* g_pd3dDeviceContext)
{
    CleanupRenderTarget( WindowDatas.g_mainRenderTargetView);
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget(ID3D11Device* g_pd3dDevice,IDXGISwapChain* g_pSwapChain, ID3D11RenderTargetView* g_renderView)
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_renderView);
    g_pd3dDevice->CreateShaderResourceView(WindowDatas.g_pRenderTargetTexture, nullptr, &WindowDatas.g_pShaderResourceView);

    pBackBuffer->Release();
}

void CleanupRenderTarget(ID3D11RenderTargetView* g_renderView)
{
    if (WindowDatas.g_pShaderResourceView) { WindowDatas.g_pShaderResourceView->Release(); WindowDatas.g_pShaderResourceView = nullptr; }
    if (g_renderView) { g_renderView->Release(); g_renderView = nullptr; }
}

