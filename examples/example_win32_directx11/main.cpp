
// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <imgui_internal.h>

// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;


// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#include <oleidl.h>
#include <string>
#include <vector>
#include <EasingAnimationSystem.h>

class DropManager : public IDropTarget
{
public:
    std::vector<std::string> files;

    //--- implement the IUnknown parts
    // you could do this the proper way with InterlockedIncrement etc,
    // but I've left out stuff that's not exactly necessary for brevity
    ULONG AddRef() { return 1; }
    ULONG Release() { return 0; }

    // we handle drop targets, let others know
    HRESULT QueryInterface(REFIID riid, void** ppvObject)
    {
        if (riid == IID_IDropTarget)
        {
            *ppvObject = this;	// or static_cast<IUnknown*> if preferred
            // AddRef() if doing things properly
                        // but then you should probably handle IID_IUnknown as well;
            return S_OK;
        }

        *ppvObject = NULL;
        return E_NOINTERFACE;
    };


    //--- implement the IDropTarget parts

    // occurs when we drag files into our applications view
    HRESULT DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
    {
        // TODO: check whether we can handle this type of object at all and set *pdwEffect &= DROPEFFECT_NONE if not;

        // do something useful to flag to our application that files have been dragged from the OS into our application


            // trigger MouseDown for button 1 within ImGui
    

            * pdwEffect &= DROPEFFECT_COPY;
        return S_OK;
    }

    // occurs when we drag files out from our applications view
    HRESULT DragLeave() { return S_OK; }

    // occurs when we drag the mouse over our applications view whilst carrying files (post Enter, pre Leave)
    HRESULT DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
    {
        // trigger MouseMove within ImGui, position is within pt.x and pt.y
        // grfKeyState contains flags for control, alt, shift etc


        *pdwEffect &= DROPEFFECT_COPY;
        return S_OK;
    }

    // occurs when we release the mouse button to finish the drag-drop operation
    HRESULT Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
    {
        // grfKeyState contains flags for control, alt, shift etc

        // render the data into stgm using the data description in fmte
        FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM stgm;

        if (SUCCEEDED(pDataObj->GetData(&fmte, &stgm)))
        {
            HDROP hdrop = (HDROP)stgm.hGlobal; // or reinterpret_cast<HDROP> if preferred
            UINT file_count = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);

            // 遍历得到的文件
            for (UINT i = 0; i < file_count; i++)
            {
                TCHAR szFile[MAX_PATH];
                UINT cch = DragQueryFile(hdrop, i, szFile, MAX_PATH);
                if (cch > 0 && cch < MAX_PATH)
                {
                    std::wstring wpath(szFile);
                    std::string utf8Path;
                    int bufSize = WideCharToMultiByte(
                        CP_UTF8, 0, wpath.c_str(), -1,
                        NULL, 0, NULL, NULL
                    );

                    if (bufSize > 0)
                    {
                        utf8Path.resize(bufSize);
                        WideCharToMultiByte(
                            CP_UTF8, 0, wpath.c_str(), -1,
                            &utf8Path[0], bufSize, NULL, NULL
                        );
                    }

                    this->files.push_back(utf8Path);
                }
            }

            // we have to release the data when we're done with it
            ReleaseStgMedium(&stgm);

            // notify our application somehow that we've finished dragging the files (provide the data somehow)

        }



        *pdwEffect &= DROPEFFECT_COPY;
        return S_OK;
    }
};


// Main code
int main(int, char**)
{

 
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
   // HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);
    HWND hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT|WS_EX_TOOLWINDOW,
        L"STATIC", L"",
        WS_POPUP, 
        0, 0, 1, 1, 
        NULL, NULL, GetModuleHandle(NULL), NULL);

    // 设置完全透明
    SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;
    //io.ConfigViewportsNoDefaultParent = true;
    //io.ConfigDockingAlwaysTabBar = true;
    //io.ConfigDockingTransparentPayload = true;
    //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: Experimental. THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
    //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 10.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool show_demo_window = true;
    bool show_another_window = false;
    //ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 0.f);
    ImGuiContext& g = *GImGui;
    HWND hwndWindow=0;

    OleInitialize(NULL);


    DropManager dm;
   //RegisterDragDrop(hwnd, &dm);
    static ImEasing::Animation<ImVec2> m_posAnim;
    static ImEasing::Animation<ImVec4> m_colorAnim;
    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();


        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;
            static bool init = false;
            ImGui::Begin("Hello, world!X", &show_demo_window);
            ImGui::EndChild();
            ImGui::Begin("Hello, world!" , &show_demo_window);

            if (!init) {
                if (hwndWindow == NULL)
                    hwndWindow = g.CurrentWindow->Viewport->PlatformHandleRaw ? (HWND)g.CurrentWindow->Viewport->PlatformHandleRaw : (HWND)g.CurrentWindow->Viewport->PlatformHandle;
                if (hwndWindow != nullptr) { 
                    init = true;
                //    ImGui_ImplWin32_EnableAlphaCompositing(hwndWindow);
                    RegisterDragDrop(hwndWindow, &dm);//添加拖拽功能
                  //  SetWindowLongPtr(hwndWindow, GWL_EXSTYLE, GetWindowLongPtr(hwndWindow, GWL_EXSTYLE) | WS_EX_LAYERED);
                  //  SetLayeredWindowAttributes(hwndWindow, 0, 255, LWA_ALPHA); // 初始不透明
                  //  MARGINS margins = { -10, -10, -10, -10 }; // 四周扩展 10 像素
                   // DwmExtendFrameIntoClientArea(hwndWindow, &margins);
                 //   ::UpdateWindow(hwndWindow);
               //     SetWindowPos(hwndWindow, nullptr, 0, 0, 0, 0,
              //          SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
                    counter++;
                }

            }


            ImGui::BeginChild("HELLO WOLRD!!",ImVec2(100, 100));
            ImGui::Text("This is some useful text.");
            ImGui::EndChild();
            

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Start Position Animation")) {
                m_posAnim.Start(
                    ImVec2(100, 100),
                    ImVec2(400, 100),
                    {1.5f,0,1,true,ImEasing::Ease::InBounce }
                );
            }
            if (ImGui::Button("Start Color Animation")) {
                m_colorAnim.Start(
                    ImVec4(0, 0, 0, 0),
                    ImVec4(255, 123, 255, 1),
                    { 2.0f,0,-1, true,  ImEasing::Ease::InOutCubic }
                );
            }

            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
            for (const std::string file : dm.files) {
                ImGui::Text("files = %s", file.c_str());
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            
            ImVec2 m_currentPos = m_posAnim.Update();
            ImVec4 color=m_colorAnim.Update();
        
            ImGui::SetCursorPos(m_currentPos);
            ImVec2 pos = ImGui::GetWindowPos();

            ImGui::GetWindowDrawList()->AddRectFilled(pos+m_currentPos, pos+m_currentPos+ImVec2(50,50), ImGui::GetColorU32(color));
            ImGui::Button("Buttonxx", ImVec2(100, 100));
            ImGui::End();
           // counter++;
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

  
        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
       
    
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
        //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    OleUninitialize();
    return 0;
}

// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
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
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
