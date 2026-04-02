#pragma once
#include <imgui.h>
#include "DX11Util.h"
#include <EasingAnimationSystem.h>

using namespace ImEasing;

// 初始化页面状态
struct InitializingPageState {
    ImageTexture logo_texture;
    bool images_loaded = false;
    Animation<float> progress_anim;
    float progress = 0.0f;
    bool started = false;
    bool completed = false;  // 新增：标记初始化是否完成
};

// 灵动岛风格形变动画参数 - 前向声明
struct MorphAnimation;

// 渲染初始化页面
// 参数：
//   state - 页面状态（需要在外部保持持久性）
//   window_width - 窗口宽度
//   window_height - 窗口高度
//   fonts - 字体指针（需要外部传入）
//   morph - 灵动岛风格形变动画参数
void RenderInitializingPage(InitializingPageState& state,
                              float window_width, float window_height,
                              ImFont* font_title,      // Manrope ExtraBold 40
                              ImFont* font_subtitle,   // Inter Regular 12
                              ImFont* font_status,     // Inter Regular 14
                              ImFont* font_server,     // Inter Regular 12
                              ImFont* font_version,    // Inter Regular 10
                              const MorphAnimation& morph);

// 启动初始化动画
void StartInitializingAnimation(InitializingPageState& state, float duration = 3.0f);
