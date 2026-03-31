
// ============================================================================
// Pandora ImGui - 使用示例
// ============================================================================
// 这个文件展示了如何使用重构后的Pandora扩展
// 关键：不修改任何imgui核心文件！
// ============================================================================

#include <imgui.h>
#include <pandora/pandora.h>

// ============================================================================
// 1. 基础示例 - 使用动画系统
// ============================================================================
void Example_AnimationSystem() {
    using namespace Pandora::Animation;

    static auto anim = std::make_shared<Animation<float>>();
    static float value = 0.0f;

    // 开始动画
    if (ImGui::Button("Start Animation")) {
        Params params;
        params.duration = 2.0f;
        params.easing = Ease::InOutQuad;
        params.loopCount = -1;
        params.pingPong = true;
        anim->Start(0.0f, 100.0f, params);
    }

    // 更新动画
    anim->Update();
    value = anim->GetCurrentValue();

    ImGui::SliderFloat("Animated Value", &value, 0.0f, 100.0f);
}

// ============================================================================
// 2. 使用WindowManager (替代修改imgui.h)
// ============================================================================
void Example_WindowManager() {
    auto& winMgr = Pandora::WindowManager::GetInstance();
    static bool dropEnabled = true;
    static bool blurEnabled = false;

    // 创建窗口
    ImGui::Begin("Pandora Window Demo");

    ImGuiID windowId = ImGui::GetID("Pandora Window Demo");

    // 设置窗口属性（不再用ImGuiWindowFlags_！）
    if (ImGui::Checkbox("Enable Drop Files", &dropEnabled)) {
        winMgr.SetDropFilesEnabled(windowId, dropEnabled);
    }

    if (ImGui::Checkbox("Enable Blur Background", &blurEnabled)) {
        winMgr.SetBlurBackground(windowId, blurEnabled);
    }

    // 获取拖放的文件
    if (dropEnabled) {
#ifdef _WIN32
        auto files = Pandora::Platform::DropManager::GetDroppedFiles(windowId);
        if (!files.empty()) {
            ImGui::Text("Dropped %zu files:", files.size());
            for (auto& file : files) {
                ImGui::BulletText("%s", file.c_str());
            }
        }
#endif
    }

    ImGui::End();
}

// ============================================================================
// 3. 初始化示例
// ============================================================================
void Example_Initialize() {
    // 1. 初始化Imgui (照常)
    // ImGui::CreateContext();
    // ImGui_ImplWin32_Init(hwnd);
    // ImGui_ImplDX11_Init(device, context);

#ifdef _WIN32
    // 2. 初始化Pandora扩展 (按需)
    Pandora::Platform::CallbackManager::GetInstance(&ImGui::GetPlatformIO());
    Pandora::Platform::DropManager::Initialize();
#endif
}

// ============================================================================
// 主渲染循环示例
// ============================================================================
void Example_MainLoop() {
    // 开始ImGui帧
    // ImGui_ImplDX11_NewFrame();
    // ImGui_ImplWin32_NewFrame();
    // ImGui::NewFrame();

    // 渲染Pandora示例
    Example_AnimationSystem();
    Example_WindowManager();

    // 结束ImGui帧
    // ImGui::Render();
    // ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

// ============================================================================
// 关键点总结
// ============================================================================
/*
✅ 好的做法：
   - #include <pandora/pandora.h>
   - 使用 Pandora::WindowManager 管理窗口属性
   - 使用 Pandora::Animation 做动画
   - imgui.h, imgui.cpp 保持纯净！

❌ 不好的做法（以前）：
   - 修改 imgui.h 添加 ImGuiWindowFlags_DropFiles
   - 修改 imgui.cpp
   - 导致无法更新imgui版本
*/
