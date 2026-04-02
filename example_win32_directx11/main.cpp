
#pragma comment(lib, "d3dcompiler.lib")

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp
#include<Profile.h>
#include <WindowUtil.h>
#include <imgui.h>
#include <CallBackManager.h>
#include <DropManager.hpp>
#include <misc/freetype/imgui_freetype.h>
#include <GuiMain.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <Shaders.h>


using namespace Window;
using namespace Global;



int main(int, char**){

    BackendWC=GetWindowClass("Pandora ImGui");

    RegisterClassExW(&BackendWC);

    BackendWindow= CreateWindowW(BackendWC.lpszClassName, L"Pandora ImGui Example", WS_POPUP, 100, 100, 1280, 1024, NULL, NULL, BackendWC.hInstance, NULL);
    ImGui::CreateContext();
    InitDirectX(BackendWindow, BackendWC);

    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
    ::ShowWindow(BackendWindow, SW_HIDE);
    ::UpdateWindow(BackendWindow);

    IMGUI_CHECKVERSION();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;     
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;      
    io.ConfigViewportsNoAutoMerge = true;
    io.ConfigViewportsNoDecoration = true;
    io.ConfigViewportsNoDefaultParent = true;

    //样式初始化
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;

    Procs::ResourceInit();

    cacheManager.SetDevice(Window::WindowDatas.g_pd3dDevice);
    cacheManager.LoadSVG("testsvg.svg");

    if (!g_animator.Init("animation_test.json", Window::WindowDatas.g_pd3dDevice, 212, 212,false)) {
        return 0;
    }



    ImGuiContext& g_context = *GImGui;
    ImGuiIO g_io = ImGui::GetIO();
    (void)g_io;

    CallBackManager& callBackManager = CallBackManager::GetInstance(&g_context.PlatformIO);

    InitImGuiWithDragDrop();

    callBackManager.SetUpCallBack();


    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor;

    // 加载所有需要的字体
    // 使用相对路径从运行目录下的fonts文件夹加载
    const char* font_dir = "fonts/";
    
    // Inter font 16 Regular - 底部栏 (大小12)
    extern ImFont* g_font_inter_regular_12;
    cfg.SizePixels = 12.0f;
    g_font_inter_regular_12 = io.Fonts->AddFontFromFileTTF("fonts/Inter-Regular.ttf", 12.0f, &cfg);
    if (!g_font_inter_regular_12) {
        printf("Failed to load fonts/Inter-Regular.ttf (12px)\n");
    }
    
    // Inter font 16 Regular - System Status, Build (大小12)
    extern ImFont* g_font_inter_regular_10;
    cfg.SizePixels = 12.0f;
    g_font_inter_regular_10 = io.Fonts->AddFontFromFileTTF("fonts/Inter-Regular.ttf", 12.0f, &cfg);
    if (!g_font_inter_regular_10) {
        printf("Failed to load fonts/Inter-Regular.ttf (12px)\n");
    }

    // Inter Semibold - OR, Operational (大小12)
    extern ImFont* g_font_inter_semibold_10;
    cfg.SizePixels = 12.0f;
    g_font_inter_semibold_10 = io.Fonts->AddFontFromFileTTF("fonts/Inter-SemiBold.ttf", 12.0f, &cfg);
    if (!g_font_inter_semibold_10) {
        printf("Failed to load fonts/Inter-SemiBold.ttf (12px)\n");
    }
    
    // Manrope Bold 11 - ACTIVE ENVIRONMENTS
    extern ImFont* g_font_manrope_bold_11;
    cfg.SizePixels = 11.0f;
    g_font_manrope_bold_11 = io.Fonts->AddFontFromFileTTF("fonts/Manrope-Bold.ttf", 11.0f, &cfg);
    if (!g_font_manrope_bold_11) {
        printf("Failed to load fonts/Manrope-Bold.ttf (11px)\n");
    }

    // Manrope Bold - LOGIN (大小16)
    extern ImFont* g_font_manrope_bold_14;
    cfg.SizePixels = 16.0f;
    g_font_manrope_bold_14 = io.Fonts->AddFontFromFileTTF("fonts/Manrope-Bold.ttf", 16.0f, &cfg);
    if (!g_font_manrope_bold_14) {
        printf("Failed to load fonts/Manrope-Bold.ttf (16px)\n");
    }

    extern ImFont* g_font_manrope_bold_18;
    cfg.SizePixels = 18.0f;
    g_font_manrope_bold_18 = io.Fonts->AddFontFromFileTTF("fonts/Manrope-Bold.ttf", 18.0f, &cfg);
    if (!g_font_manrope_bold_18) {
        printf("Failed to load fonts/Manrope-Bold.ttf (18px)\n");
    }
    
    // Inter Semibold - Account Credentials, License Key 标签 (大小12)
    extern ImFont* g_font_inter_semibold_12;
    cfg.SizePixels = 12.0f;
    g_font_inter_semibold_12 = io.Fonts->AddFontFromFileTTF("fonts/Inter-SemiBold.ttf", 12.0f, &cfg);
    if (!g_font_inter_semibold_12) {
        printf("Failed to load fonts/Inter-SemiBold.ttf (12px)\n");
    }
    
    // Inter font 16 Regular - 编辑栏 (大小16)
    extern ImFont* g_font_inter_regular_16;
    cfg.SizePixels = 16.0f;
    g_font_inter_regular_16 = io.Fonts->AddFontFromFileTTF("fonts/Inter-Regular.ttf", 16.0f, &cfg);
    if (!g_font_inter_regular_16) {
        printf("Failed to load fonts/Inter-Regular.ttf (16px)\n");
    }
    
    // Inter Semibold - LOGIN按钮 (大小16)
    extern ImFont* g_font_inter_semibold_16;
    cfg.SizePixels = 16.0f;
    g_font_inter_semibold_16 = io.Fonts->AddFontFromFileTTF("fonts/Inter-SemiBold.ttf", 16.0f, &cfg);
    if (!g_font_inter_semibold_16) {
        printf("Failed to load fonts/Inter-SemiBold.ttf (16px)\n");
    }
    
    // Manrope ExtraBold - NOVOCAINES (大小32)
    extern ImFont* g_font_manrope_extrabold_32;
    cfg.SizePixels = 32.0f;
    g_font_manrope_extrabold_32 = io.Fonts->AddFontFromFileTTF("fonts/Manrope-ExtraBold.ttf", 32.0f, &cfg);
    if (!g_font_manrope_extrabold_32) {
        printf("Failed to load fonts/Manrope-ExtraBold.ttf (32px)\n");
    }
    
    // Inter font 16 Regular - Clinical Injection Systems (大小14)
    extern ImFont* g_font_inter_regular_14;
    cfg.SizePixels = 14.0f;
    g_font_inter_regular_14 = io.Fonts->AddFontFromFileTTF("fonts/Inter-Regular.ttf", 14.0f, &cfg);
    if (!g_font_inter_regular_14) {
        printf("Failed to load fonts/Inter-Regular.ttf (14px)\n");
    }
    
    // ===== 主页面专用字体 =====
    
    // Manrope ExtraBold 48 - 进程标题
    extern ImFont* g_font_manrope_extrabold_48;
    cfg.SizePixels = 48.0f;
    g_font_manrope_extrabold_48 = io.Fonts->AddFontFromFileTTF("fonts/Manrope-ExtraBold.ttf", 48.0f, &cfg);
    if (!g_font_manrope_extrabold_48) {
        printf("Failed to load fonts/Manrope-ExtraBold.ttf (48px)\n");
    }
    
    // Inter Medium 16 - PID等一行
    extern ImFont* g_font_inter_medium_16;
    cfg.SizePixels = 16.0f;
    g_font_inter_medium_16 = io.Fonts->AddFontFromFileTTF("fonts/Inter-Medium.ttf", 16.0f, &cfg);
    if (!g_font_inter_medium_16) {
        printf("Failed to load fonts/Inter-Medium.ttf (16px)\n");
    }
    
    // Inter Medium 14 - 未选中项
    extern ImFont* g_font_inter_medium_14;
    cfg.SizePixels = 14.0f;
    g_font_inter_medium_14 = io.Fonts->AddFontFromFileTTF("fonts/Inter-Medium.ttf", 14.0f, &cfg);
    if (!g_font_inter_medium_14) {
        printf("Failed to load fonts/Inter-Medium.ttf (14px)\n");
    }
    
    // Inter Semibold 20 - 选中项
    extern ImFont* g_font_inter_semibold_20;
    cfg.SizePixels = 20.0f;
    g_font_inter_semibold_20 = io.Fonts->AddFontFromFileTTF("fonts/Inter-SemiBold.ttf", 20.0f, &cfg);
    if (!g_font_inter_semibold_20) {
        printf("Failed to load fonts/Inter-SemiBold.ttf (20px)\n");
    }
    
    // Inter Semibold 18 - LOGO
    extern ImFont* g_font_inter_semibold_18;
    cfg.SizePixels = 18.0f;
    g_font_inter_semibold_18 = io.Fonts->AddFontFromFileTTF("fonts/Inter-SemiBold.ttf", 18.0f, &cfg);
    if (!g_font_inter_semibold_18) {
        printf("Failed to load fonts/Inter-SemiBold.ttf (18px)\n");
    }
    
    // Inter Semibold 11 - INJECTION TARGET
    extern ImFont* g_font_inter_semibold_11;
    cfg.SizePixels = 11.0f;
    g_font_inter_semibold_11 = io.Fonts->AddFontFromFileTTF("fonts/Inter-SemiBold.ttf", 11.0f, &cfg);
    if (!g_font_inter_semibold_11) {
        printf("Failed to load fonts/Inter-SemiBold.ttf (11px)\n");
    }
    
    // Inter Semibold 14 - DLL名
    extern ImFont* g_font_inter_semibold_14;
    cfg.SizePixels = 14.0f;
    g_font_inter_semibold_14 = io.Fonts->AddFontFromFileTTF("fonts/Inter-SemiBold.ttf", 14.0f, &cfg);
    if (!g_font_inter_semibold_14) {
        printf("Failed to load fonts/Inter-SemiBold.ttf (14px)\n");
    }
    
    // Inter Semibold 12 - BROWSE (使用已存在的 g_font_inter_semibold_12)
    // 这个字体已经在前面加载过了，不需要重复加载
    
    // Inter Regular 11 - Size等文件信息
    extern ImFont* g_font_inter_regular_11;
    cfg.SizePixels = 11.0f;
    g_font_inter_regular_11 = io.Fonts->AddFontFromFileTTF("fonts/Inter-Regular.ttf", 11.0f, &cfg);
    if (!g_font_inter_regular_11) {
        printf("Failed to load fonts/Inter-Regular.ttf (11px)\n");
    }
    
    // 默认字体
    io.FontDefault = g_font_inter_regular_14;

    float clearColor[4] = { 0.f, 0.f, 0.0f, 0.f };


    while (!Application::Exit)
    {

        MSG msg;
       
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {

            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
          
        }
     
        if (WindowDatas.g_SwapChainOccluded && (WindowDatas.g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED))
        {
            ::Sleep(10);
            continue;
        }
        WindowDatas.g_SwapChainOccluded = false;


        if (WindowDatas.g_ResizeWidth != 0 && WindowDatas.g_ResizeHeight != 0)
        {
            CleanupRenderTarget(WindowDatas.g_mainRenderTargetView);
            WindowDatas.g_pSwapChain->ResizeBuffers(0, WindowDatas.g_ResizeWidth, WindowDatas.g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            WindowDatas.g_ResizeWidth = WindowDatas.g_ResizeHeight = 0;
            CreateRenderTarget(WindowDatas.g_pd3dDevice, WindowDatas.g_pSwapChain,WindowDatas.g_mainRenderTargetView);
        }

     
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame(); {
            RenderMainWindow();
        }
        ImGui::Render();

      
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
           ImGui::UpdatePlatformWindows();
           ImGui::RenderPlatformWindowsDefault();
        }
        

         HRESULT hr = WindowDatas.g_pSwapChain->Present(1, 0);
         WindowDatas.g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

      }

      
END:
    Shutdown(BackendWindow,WindowDatas);
        
    return 0;
}


