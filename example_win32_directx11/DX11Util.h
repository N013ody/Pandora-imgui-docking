#pragma once
#include <d3d11.h>
#include "imgui.h"
#include <nanosvg.h>




struct DirectXData {

    ID3D11Device* g_pd3dDevice = nullptr;
    ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
    IDXGISwapChain* g_pSwapChain = nullptr;
    bool                     g_SwapChainOccluded = false;
    UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
    ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

    ID3D11RasterizerState* g_origRasterizer = nullptr;
    ID3D11BlendState* g_origBlendState = nullptr;
    ID3D11DepthStencilState* g_origDepthStencil = nullptr;
    FLOAT g_origBlendFactor[4] = { 0 };
    UINT g_origSampleMask = 0;
    UINT g_origStencilRef = 0;

    ID3D11Texture2D* g_pRenderTargetTexture = nullptr;
    ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
    ID3D11ShaderResourceView* g_pShaderResourceView = nullptr;

};

struct SVGTexture {
    ID3D11ShaderResourceView* srv;
    ImVec2 size;
};

struct ImageTexture {
    ID3D11ShaderResourceView* srv = nullptr;
    ImVec2 size = ImVec2(0, 0);
    bool valid = false;
};

SVGTexture GetSVGTexture(ID3D11Device* device, NSVGimage* svgImage,float scale, int scaledWidth, int scaledHeight, bool release);

SVGTexture CreateSVGTexture(ID3D11Device* device, NSVGimage* svgImage, float scale=1.0f, bool release = false);

SVGTexture CreateSVGTexture(ID3D11Device* device, const char* svgPath, float scale=1.0f, bool release = false);

// 加载PNG图片纹理
ImageTexture LoadPNGTexture(ID3D11Device* device, const char* filePath);
void UnloadTexture(ImageTexture& texture);

bool CreateShaderTarget(ID3D11Device* g_pd3dDevice, ID3D11Texture2D** g_pRenderTargetTexture, ID3D11RenderTargetView** g_pRenderTargetView, ID3D11ShaderResourceView** g_pShaderResourceView);

void Cleanup(IDXGISwapChain* swapChain, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11RenderTargetView* renderTargetView);

bool CreateDeviceD3D(HWND hWnd);

void CleanupDeviceD3D(ID3D11Device* g_pd3dDevice, IDXGISwapChain* g_pSwapChain, ID3D11DeviceContext* g_pd3dDeviceContext);

void CleanupRenderTarget(ID3D11RenderTargetView* g_renderView);

void CreateRenderTarget(ID3D11Device* g_pd3dDevice, IDXGISwapChain* g_pSwapChain, ID3D11RenderTargetView* g_renderView);

bool InitDirectX(HWND hwnd, WNDCLASSEXW wc);

void Shutdown(HWND BackendWindow, DirectXData WindowDats);
