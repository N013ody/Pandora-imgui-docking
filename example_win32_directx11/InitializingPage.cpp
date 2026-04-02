#define IMGUI_DEFINE_MATH_OPERATORS
#include "InitializingPage.h"
#include "backends/pimgui.h"
#include "Profile/Profile.h"

void StartInitializingAnimation(InitializingPageState& state, float duration) {
    if (!state.started) {
        Params progress_params;
        progress_params.duration = duration;
        progress_params.easing = Ease::InOutQuad;
        state.progress_anim.Start(0.0f, 1.0f, progress_params);
        state.started = true;
    }
}

// 灵动岛风格形变动画参数定义
struct MorphAnimation {
    // 整体形变参数
    float global_scale = 1.0f;
    float global_alpha = 1.0f;
    float global_blur = 0.0f;
    
    // 登录页面特定参数
    float login_content_y = 0.0f;
    float login_content_scale = 1.0f;
    float login_content_alpha = 1.0f;
    float login_form_stretch = 1.0f;
    
    // 初始化页面特定参数
    float init_content_y = 50.0f;
    float init_content_scale = 0.9f;
    float init_content_alpha = 0.0f;
    float init_logo_bounce = 0.0f;
};

void RenderInitializingPage(InitializingPageState& state,
                              float window_width, float window_height,
                              ImFont* font_title,
                              ImFont* font_subtitle,
                              ImFont* font_status,
                              ImFont* font_server,
                              ImFont* font_version,
                              const MorphAnimation& morph)
{
    ImGui::PushID("InitPage");
    
    // 加载Logo图片（只加载一次）
    if (!state.images_loaded) {
        state.logo_texture = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Loading_logo.png");
        state.images_loaded = true;
    }

    // 更新进度条动画
    state.progress_anim.Update();
    state.progress = state.progress_anim.GetCurrentValue();
    
    // 检查是否完成
    if (state.progress >= 1.0f && !state.completed) {
        state.completed = true;
    }
    
    // 获取窗口位置用于相对定位
    ImVec2 window_pos = ImGui::GetWindowPos();
    float center_x = window_pos.x + window_width / 2.0f;
    
    // 基础Y位置（窗口中心上方150px）
    float base_y = window_pos.y + window_height / 2.0f - 150.0f;
    
    // 应用内容形变
    base_y += morph.init_content_y;
    float content_scale = morph.init_content_scale;
    float content_alpha = morph.init_content_alpha;
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // ========== Logo图片 ==========
    float logo_size = 80.0f * content_scale;
    float logo_y = base_y + morph.init_logo_bounce;
    ImVec2 logo_pos = ImVec2(center_x - logo_size / 2.0f, logo_y);
    int logo_alpha_byte = static_cast<int>(255 * content_alpha);
    
    if (state.logo_texture.valid) {
        draw_list->AddImage(
            state.logo_texture.srv,
            logo_pos,
            ImVec2(logo_pos.x + logo_size, logo_pos.y + logo_size),
            ImVec2(0, 0),
            ImVec2(1, 1),
            IM_COL32(255, 255, 255, logo_alpha_byte)
        );
    } else {
        // 占位符 - 蓝色背景
        draw_list->AddRectFilled(
            logo_pos,
            ImVec2(logo_pos.x + logo_size, logo_pos.y + logo_size),
            IM_COL32(0, 88, 188, logo_alpha_byte),
            0.0f
        );
    }

    // ========== NOVOCAINES 标题 ==========
    float title_y = logo_y + logo_size + 24 * content_scale;
    int title_alpha_byte = static_cast<int>(255 * content_alpha);
    
    const char* title = "NOVOCAINES";
    if (font_title) {
        ImVec2 title_size = font_title->CalcTextSizeA(40.0f * content_scale, FLT_MAX, 0.0f, title);
        ImVec2 title_pos = ImVec2(center_x - title_size.x / 2.0f, title_y);
        draw_list->AddText(font_title, 40.0f * content_scale, title_pos, IM_COL32(0, 0, 0, title_alpha_byte), title);
    } else {
        ImVec2 title_size = ImGui::CalcTextSize(title);
        title_size.x *= content_scale;
        ImVec2 title_pos = ImVec2(center_x - title_size.x / 2.0f, title_y);
        draw_list->AddText(title_pos, IM_COL32(0, 0, 0, title_alpha_byte), title);
    }

    // ========== Advanced Injection Protocol 副标题 ==========
    float subtitle_y = title_y + 48 * content_scale + 4;
    int subtitle_alpha_byte = static_cast<int>(255 * content_alpha);
    
    const char* subtitle = "ADVANCED INJECTION PROTOCOL";
    if (font_subtitle) {
        ImVec2 subtitle_size = font_subtitle->CalcTextSizeA(12.0f * content_scale, FLT_MAX, 0.0f, subtitle);
        ImVec2 subtitle_pos = ImVec2(center_x - subtitle_size.x / 2.0f, subtitle_y);
        draw_list->AddText(font_subtitle, 12.0f * content_scale, subtitle_pos, IM_COL32(113, 119, 134, subtitle_alpha_byte), subtitle);
    } else {
        ImVec2 subtitle_size = ImGui::CalcTextSize(subtitle);
        subtitle_size.x *= content_scale;
        ImVec2 subtitle_pos = ImVec2(center_x - subtitle_size.x / 2.0f, subtitle_y);
        draw_list->AddText(subtitle_pos, IM_COL32(113, 119, 134, subtitle_alpha_byte), subtitle);
    }

    // ========== 渐变进度条 ==========
    float progress_y = subtitle_y + 20 * content_scale + 8;
    int progress_alpha_byte = static_cast<int>(255 * content_alpha);
    
    float progress_width = 400.0f * content_scale;
    float progress_height = 4.0f;
    
    // 使用组件绘制进度条（应用alpha）
    ImGui::SetCursorScreenPos(ImVec2(center_x - progress_width / 2.0f, progress_y));
    ImGui::PushID("Init_progress_bar");
    PImGui::GradientProgressBar(
        state.progress,
        progress_width,
        progress_height,
        IM_COL32(0, 88, 188, progress_alpha_byte),    // #0058BC
        IM_COL32(0, 112, 235, progress_alpha_byte),   // #0070EB
        IM_COL32(225, 227, 228, progress_alpha_byte)  // #E1E3E4
    );
    ImGui::PopID();

    // ========== Initializing Core Modules... ==========
    float status_y = progress_y + progress_height + 32 * content_scale;
    int status_alpha_byte = static_cast<int>(255 * content_alpha);
    
    const char* init_text = "Initializing Core Modules...";
    if (font_status) {
        ImVec2 init_text_size = font_status->CalcTextSizeA(14.0f * content_scale, FLT_MAX, 0.0f, init_text);
        ImVec2 init_text_pos = ImVec2(center_x - init_text_size.x / 2.0f, status_y);
        draw_list->AddText(font_status, 14.0f * content_scale, init_text_pos, IM_COL32(0, 0, 0, status_alpha_byte), init_text);
    } else {
        ImVec2 init_text_size = ImGui::CalcTextSize(init_text);
        init_text_size.x *= content_scale;
        ImVec2 init_text_pos = ImVec2(center_x - init_text_size.x / 2.0f, status_y);
        draw_list->AddText(init_text_pos, IM_COL32(0, 0, 0, status_alpha_byte), init_text);
    }

    // ========== 服务器状态指示器 ==========
    float server_y = status_y + 20 * content_scale + 4;
    const char* status_text = "Status: Connected to Secure Server";
    float dot_size = 8.0f * content_scale;
    float dot_spacing = 8.0f * content_scale;
    
    if (font_server) {
        ImVec2 status_size = font_server->CalcTextSizeA(12.0f * content_scale, FLT_MAX, 0.0f, status_text);
        float total_status_width = dot_size + dot_spacing + status_size.x;
        float status_start_x = center_x - total_status_width / 2.0f;
        
        ImVec2 dot_center = ImVec2(status_start_x + dot_size / 2.0f, server_y + status_size.y / 2.0f);
        draw_list->AddCircleFilled(dot_center, dot_size / 2.0f, IM_COL32(0, 88 ,188, status_alpha_byte), 12);
        
        // 绘制状态文本
        ImVec2 status_pos = ImVec2(status_start_x + dot_size + dot_spacing, server_y);
        draw_list->AddText(font_server, 12.0f * content_scale, status_pos, IM_COL32(65, 71, 85, status_alpha_byte), status_text);
    } else {
        ImVec2 status_size = ImGui::CalcTextSize(status_text);
        status_size.x *= content_scale;
        float total_status_width = dot_size + dot_spacing + status_size.x;
        float status_start_x = center_x - total_status_width / 2.0f;
        
        ImVec2 dot_center = ImVec2(status_start_x + dot_size / 2.0f, server_y + status_size.y / 2.0f);
        draw_list->AddCircleFilled(dot_center, dot_size / 2.0f, IM_COL32(0, 200, 83, status_alpha_byte), 12);
        
        ImVec2 status_pos = ImVec2(status_start_x + dot_size + dot_spacing, server_y);
        draw_list->AddText(status_pos, IM_COL32(65, 71, 85, status_alpha_byte), status_text);
    }

    // ========== 版本号 ==========
    int version_alpha_byte = static_cast<int>(255 * content_alpha);
    const char* version_text = "v4.0.2 STABLE BUILD";
    float version_y = window_pos.y + window_height - 60.0f;
    
    if (font_version) {
        ImVec2 version_size = font_version->CalcTextSizeA(10.0f * content_scale, FLT_MAX, 0.0f, version_text);
        ImVec2 version_pos = ImVec2(center_x - version_size.x / 2.0f, version_y);
        draw_list->AddText(font_version, 10.0f * content_scale, version_pos, IM_COL32(113, 119, 134, (int)(153 * content_alpha)), version_text);
    } else {
        ImVec2 version_size = ImGui::CalcTextSize(version_text);
        version_size.x *= content_scale;
        ImVec2 version_pos = ImVec2(center_x - version_size.x / 2.0f, version_y);
        draw_list->AddText(version_pos, IM_COL32(113, 119, 134, (int)(153 * content_alpha)), version_text);
    }
    
    ImGui::PopID();
}
