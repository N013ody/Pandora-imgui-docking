#define IMGUI_DEFINE_MATH_OPERATORS
#include <GuiMain.h>
#include <pimgui.h>
#include <imgui.h>
#include<Profile.h>
#include <string>
#include <EasingAnimationSystem.h>
#include <WICTextureLoader.h>
#include <Shaders.h>
#include "DX11Util.h"
#include "InitializingPage.h"
#include "MainPage.h"
using namespace ImEasing;
using namespace DirectX;
using namespace Global;
using namespace Window;

// 字体全局变量定义
ImFont* g_font_inter_regular_12 = nullptr;
ImFont* g_font_inter_regular_10 = nullptr;
ImFont* g_font_inter_semibold_10 = nullptr;
ImFont* g_font_manrope_bold_14 = nullptr;
ImFont* g_font_manrope_bold_18 = nullptr;
ImFont* g_font_inter_semibold_12 = nullptr;
ImFont* g_font_inter_regular_16 = nullptr;
ImFont* g_font_inter_semibold_16 = nullptr;
ImFont* g_font_manrope_extrabold_32 = nullptr;
ImFont* g_font_inter_regular_14 = nullptr;

// 主页面专用字体
ImFont* g_font_manrope_extrabold_48 = nullptr;    // 进程标题 48号
ImFont* g_font_inter_medium_16 = nullptr;         // PID等一行 16号
ImFont* g_font_inter_medium_14 = nullptr;         // 未选中项 14号
ImFont* g_font_inter_semibold_20 = nullptr;       // 选中项 20号
ImFont* g_font_inter_semibold_18 = nullptr;       // LOGO 18号
ImFont* g_font_inter_semibold_11 = nullptr;       // INJECTION TARGET 11号
ImFont* g_font_inter_semibold_14 = nullptr;       // DLL名 14号
ImFont* g_font_inter_regular_11 = nullptr;        // Size等文件信息 11号

// 图片纹理全局变量
ImageTexture g_logo_texture;
ImageTexture g_key_texture;
ImageTexture g_email_texture;

// 主页面图标纹理
ImageTexture g_icon_search;
ImageTexture g_icon_game;
ImageTexture g_icon_cpu;
ImageTexture g_icon_chrome;
ImageTexture g_icon_cmd;
ImageTexture g_icon_process;
ImageTexture g_icon_protect;
ImageTexture g_icon_path;
ImageTexture g_icon_file;
ImageTexture g_icon_inject;
ImageTexture g_icon_setting;
ImageTexture g_icon_dump;
ImageTexture g_icon_viewdump;
bool g_main_images_loaded = false;

// 静态变量用于登录页面
static char g_license_key[64] = "";
static char g_email[128] = "";
bool g_images_loaded = false;

// 动画相关
Animation<float> g_fade_animation;
bool g_animation_initialized = false;

// 输入框聚焦动画
struct InputFocusAnim {
    Animation<float> focus_anim;
    bool was_focused = false;
    float current_alpha = 0.0f; // 0.0 = 未聚焦, 1.0 = 聚焦
};
static InputFocusAnim g_license_focus_anim;
static InputFocusAnim g_email_focus_anim;

// 页面状态枚举
enum AppPage {
    PAGE_LOGIN = 0,
    PAGE_INITIALIZING,
    PAGE_MAIN,
};
static AppPage g_current_page = PAGE_LOGIN;
static AppPage g_target_page = PAGE_LOGIN;

// 灵动岛风格页面切换动画系统
// 特点：流畅形变、弹性回弹、内容无缝切换

// 动画阶段枚举
enum TransitionPhase {
    PHASE_NONE = 0,
    PHASE_EXITING,      // 当前页面退出
    PHASE_MORPHING,     // 形变过渡
    PHASE_ENTERING      // 新页面进入
};

// 页面切换动画状态
struct PageTransitionState {
    Animation<float> anim;
    bool active = false;
    float progress = 0.0f;
    TransitionPhase phase = PHASE_NONE;
};
static PageTransitionState g_transition;

// 灵动岛风格动画参数 - 统一控制
struct MorphAnimation {
    // 整体形变参数
    float global_scale = 1.0f;          // 整体缩放
    float global_alpha = 1.0f;          // 整体透明度
    float global_blur = 0.0f;           // 模糊程度（模拟深度）
    
    // 登录页面特定参数
    float login_content_y = 0.0f;       // 内容垂直偏移
    float login_content_scale = 1.0f;   // 内容缩放
    float login_content_alpha = 1.0f;   // 内容透明度
    float login_form_stretch = 1.0f;    // 表单拉伸（形变效果）
    
    // 初始化页面特定参数
    float init_content_y = 50.0f;       // 内容初始垂直偏移
    float init_content_scale = 0.9f;    // 内容初始缩放
    float init_content_alpha = 0.0f;    // 内容初始透明度
    float init_logo_bounce = 0.0f;      // Logo弹跳偏移
};
static MorphAnimation g_morph;

// 初始化页面状态（使用新的组件化结构）
static InitializingPageState g_init_page_state;
static MainPageState g_main_page_state;

HRESULT GuiMainInit() {
    return CreateWICTextureFromMemory(
        Window::WindowDatas.g_pd3dDevice,      
        nullptr,                               
        reinterpret_cast<const uint8_t*>(TestPlayImage),
        sizeof(TestPlayImage),                 
        &textureResource,                     
        &playImagePtr,                         
        0                                      
    );
}

// 切换到指定页面（灵动岛风格过渡动画）
void SwitchToPage(AppPage target_page, float duration = 0.8f) {
    if (g_current_page == target_page || g_transition.active) return;
    
    g_target_page = target_page;
    g_transition.active = true;
    g_transition.progress = 0.0f;
    g_transition.phase = PHASE_EXITING;
    
    // 启动过渡动画 - 使用Spring弹性缓动
    Params transition_params;
    transition_params.duration = duration;
    transition_params.easing = Ease::OutElastic;  // 弹性缓动，类似灵动岛
    g_transition.anim.Start(0.0f, 1.0f, transition_params);
}

static bool show_login_window = true;

// 渲染登录页面内容 - 使用灵动岛风格形变参数
void RenderLoginContent(const MorphAnimation& morph);

// 渲染初始化页面内容 - 使用灵动岛风格形变参数
void RenderInitializingContent(const MorphAnimation& morph);

// 渲染主页面内容
void RenderMainContentPage(const MorphAnimation& morph);

void RenderMainWindow() {
    // 检查初始化是否完成，如果完成则切换到主页面
    if (g_current_page == PAGE_INITIALIZING && g_init_page_state.completed && !g_transition.active) {
        SwitchToPage(PAGE_MAIN, 0.5f);
    }
    
    // 更新页面切换动画
    if (g_transition.active) {
        g_transition.anim.Update();
        g_transition.progress = g_transition.anim.GetCurrentValue();
        float t = g_transition.progress;
        
        // ===== 灵动岛风格形变动画 =====
        // 阶段1: 0.0-0.4 当前页面收缩并淡出
        // 阶段2: 0.3-0.7 形变过渡（类似灵动岛的形态变化）
        // 阶段3: 0.6-1.0 新页面展开并淡入
        
        if (t < 0.4f) {
            // 阶段1: 登录页面收缩退出
            float exit_t = t / 0.4f;
            float smooth_exit = Ease::InOutCubic(exit_t);
            
            g_morph.login_content_scale = 1.0f - 0.15f * smooth_exit;
            g_morph.login_content_alpha = 1.0f - smooth_exit;
            g_morph.login_content_y = -30.0f * smooth_exit;
            g_morph.global_scale = 1.0f - 0.05f * smooth_exit;
            
            g_morph.init_content_alpha = 0.0f;
            g_transition.phase = PHASE_EXITING;
        }
        else if (t < 0.6f) {
            // 阶段2: 形变过渡 - 类似灵动岛的形态变化
            float morph_t = (t - 0.4f) / 0.2f;
            float smooth_morph = Ease::InOutSine(morph_t);
            
            // 整体轻微收缩再展开（弹性效果）
            float bounce = sin(morph_t * IM_EASING_PI) * 0.03f;
            g_morph.global_scale = 0.95f + bounce;
            
            g_morph.login_content_alpha = 0.0f;
            g_morph.init_content_alpha = 0.0f;
            g_transition.phase = PHASE_MORPHING;
        }
        else {
            // 阶段3: 初始化页面弹性进入
            float enter_t = (t - 0.6f) / 0.4f;
            // 使用OutBack缓动产生回弹效果
            float back_enter = Ease::OutBack(enter_t);
            
            g_morph.init_content_scale = 0.85f + 0.15f * back_enter;
            g_morph.init_content_alpha = back_enter;
            g_morph.init_content_y = 40.0f * (1.0f - back_enter);
            g_morph.init_logo_bounce = -10.0f * sin(enter_t * IM_EASING_PI * 2.0f) * (1.0f - enter_t);
            
            g_morph.global_scale = 0.95f + 0.05f * back_enter;
            g_transition.phase = PHASE_ENTERING;
        }
        
        // 动画完成，切换页面
        if (g_transition.progress >= 1.0f) {
            g_current_page = g_target_page;
            g_transition.active = false;
            g_transition.progress = 0.0f;
            g_transition.phase = PHASE_NONE;
            
            // 重置形变参数
            g_morph.global_scale = 1.0f;
            g_morph.login_content_scale = 1.0f;
            g_morph.login_content_alpha = 1.0f;
            g_morph.login_content_y = 0.0f;
            g_morph.init_content_scale = 0.9f;
            g_morph.init_content_alpha = 0.0f;
            g_morph.init_content_y = 50.0f;
            g_morph.init_logo_bounce = 0.0f;
            
            // 如果切换到初始化页面，启动初始化动画
            if (g_current_page == PAGE_INITIALIZING) {
                StartInitializingAnimation(g_init_page_state, 3.0f);
            }
        }
    } else {
        // 非动画状态，保持默认参数
        g_morph.global_scale = 1.0f;
        g_morph.global_alpha = 1.0f;
        g_morph.login_content_scale = 1.0f;
        g_morph.login_content_alpha = 1.0f;
        g_morph.login_content_y = 0.0f;
        g_morph.init_content_scale = (g_current_page == PAGE_INITIALIZING) ? 1.0f : 0.9f;
        g_morph.init_content_alpha = (g_current_page == PAGE_INITIALIZING) ? 1.0f : 0.0f;
        g_morph.init_content_y = (g_current_page == PAGE_INITIALIZING) ? 0.0f : 50.0f;
    }

    // 设置窗口大小和位置 - 全屏窗口
    ImGui::SetNextWindowSize(ImVec2(1280, 1024));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(248.0f/255.0f, 249.0f/255.0f, 250.0f/255.0f, 1.0f));
    
    ImGui::Begin("NOVOCAINES", &show_login_window,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
    
    // 应用全局形变
    ImVec2 window_center = ImVec2(640.0f, 512.0f); // 窗口中心点
    
    // 根据当前状态和动画渲染页面
    if (g_transition.active) {
        // 过渡期间根据阶段渲染不同页面，实现交叉淡入淡出效果
        if (g_transition.phase == PHASE_EXITING) {
            // 退出阶段 - 只渲染登录页面
            RenderLoginContent(g_morph);
        }
        else if (g_transition.phase == PHASE_MORPHING) {
            // 形变阶段 - 同时渲染两个页面，实现无缝过渡
            // 使用不同的ID作用域避免控件ID冲突
            ImGui::PushID("Morph_Login");
            RenderLoginContent(g_morph);
            ImGui::PopID();
            
            ImGui::PushID("Morph_Init");
            RenderInitializingContent(g_morph);
            ImGui::PopID();
        }
        else if (g_transition.phase == PHASE_ENTERING) {
            // 进入阶段 - 根据目标页面渲染相应内容
            if (g_target_page == PAGE_INITIALIZING) {
                RenderInitializingContent(g_morph);
            } else if (g_target_page == PAGE_MAIN) {
                RenderMainContentPage(g_morph);
            }
        }
    } else {
        // 正常渲染当前页面
        switch (g_current_page) {
            case PAGE_LOGIN:
                RenderLoginContent(g_morph);
                break;
            case PAGE_INITIALIZING:
                RenderInitializingContent(g_morph);
                break;
            case PAGE_MAIN:
                RenderMainContentPage(g_morph);
                break;
        }
    }
    
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::End();

    if (!show_login_window) Application::Exit = true;
}



void RenderLoginContent(const MorphAnimation& morph) {
    ImGui::PushID("LoginPage");
    
    // 加载图片（只加载一次）
    if (!g_images_loaded) {
        g_logo_texture = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Login_logo.png");
        g_key_texture = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Login_key.png");
        g_email_texture = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Login_email.png");
        g_images_loaded = true;
    }

    // 初始化淡入动画（只执行一次）
    if (!g_animation_initialized) {
        Params fade_params;
        fade_params.duration = 0.8f; // 0.8秒淡入
        fade_params.easing = Ease::OutCubic; // 使用OutCubic缓动
        g_fade_animation.Start(0.0f, 1.0f, fade_params);
        g_animation_initialized = true;
    }

    // 更新动画
    g_fade_animation.Update();
    float base_fade = g_fade_animation.GetCurrentValue();

    // 内容区域最大宽度420px,居中显示
    float content_width = 420.0f * morph.login_content_scale;
    float window_width = ImGui::GetWindowWidth();
    float start_x = (window_width - content_width) / 2.0f;
    float base_y = 155.0f + morph.login_content_y; // Logo Y轴从155开始，应用形变偏移

    // ========== Logo区域 ==========
    // 64x64px 蓝色背景矩形(无圆角),居中
    float logo_size = 64.0f * morph.login_content_scale;
    float logo_y = base_y;
    ImGui::SetCursorPos(ImVec2(start_x + (content_width - logo_size) / 2.0f, logo_y));
    ImVec2 logo_pos = ImGui::GetCursorScreenPos();

    // 计算各层透明度（统一使用内容透明度）
    float content_fade = base_fade * morph.login_content_alpha;
    float footer_fade = base_fade * morph.login_content_alpha;
    
    // primary color: #0058bc，应用淡入透明度
    int alpha_byte = static_cast<int>(255 * content_fade);
    ImU32 primary_color = IM_COL32(0, 88, 188, alpha_byte);
    ImU32 white_color = IM_COL32(255, 255, 255, 255);
    ImU32 on_surface_variant = IM_COL32(65, 71, 85, alpha_byte); // #414755
    ImU32 outline_color = IM_COL32(113, 119, 134, alpha_byte); // #717786
    ImU32 outline_variant = IM_COL32(193, 198, 215, alpha_byte); // #c1c6d7
    ImU32 secondary_color = IM_COL32(64, 94, 150, alpha_byte); // #405e96
    

    // 绘制Logo图片 (64x64)，应用淡入透明度
    ImU32 image_alpha = IM_COL32(255, 255, 255, alpha_byte);
    if (g_logo_texture.valid) {
        ImGui::GetWindowDrawList()->AddImage(
            g_logo_texture.srv,
            logo_pos,
            logo_pos + ImVec2(logo_size, logo_size),
            ImVec2(0, 0),
            ImVec2(1, 1),
            image_alpha
        );
    } else {
        // 图标占位符 (中央小矩形代表biotech图标) - 无圆角
        ImGui::GetWindowDrawList()->AddRectFilled(
            logo_pos + ImVec2(20, 20),
            logo_pos + ImVec2(44, 44),
            image_alpha,
            0.0f // 无圆角
        );
    }
    
    // ========== 标题区域 ==========
    float title_y = logo_y + logo_size + 16 * morph.login_content_scale;
    int title_alpha_byte = static_cast<int>(255 * content_fade);
    
    // NOVOCAINES 主标题 - Manrope ExtraBold 32
    ImGui::PushFont(g_font_manrope_extrabold_32);
    const char* title = "NOVOCAINES";
    ImVec2 title_size = ImGui::CalcTextSize(title);
    ImGui::SetCursorPos(ImVec2(start_x + (content_width - title_size.x) / 2.0f, title_y));
    ImGui::TextColored(ImVec4(0, 0, 0, content_fade), title);
    ImGui::PopFont();
    
    // Clinical Injection Systems 副标题 - Inter Regular 14
    float subtitle_y = title_y + title_size.y + 4;
    ImGui::PushFont(g_font_inter_regular_14);
    const char* subtitle = "CLINICAL INJECTION SYSTEMS";
    ImVec2 subtitle_size = ImGui::CalcTextSize(subtitle);
    ImGui::SetCursorPos(ImVec2(start_x + (content_width - subtitle_size.x) / 2.0f, subtitle_y));
    ImGui::TextColored(ImVec4(65.0f/255.0f, 71.0f/255.0f, 85.0f/255.0f, 0.7f * content_fade), subtitle);
    ImGui::PopFont();
    
    // ========== License Key输入框 ==========
    float form_y = subtitle_y + subtitle_size.y + 48;
    int form_alpha_byte = static_cast<int>(255 * content_fade);
    
    // 标签 - Inter Semibold 12
    ImGui::PushFont(g_font_inter_semibold_12);
    const char* license_label = "LICENSE KEY";
    ImGui::SetCursorPos(ImVec2(start_x + 4, form_y)); // px-1
    ImGui::TextColored(ImVec4(65.0f/255.0f, 71.0f/255.0f, 85.0f/255.0f, content_fade), license_label);
    ImGui::PopFont();

    float current_y = form_y + ImGui::GetTextLineHeight() + 8;

    // 输入框容器
    ImGui::SetCursorPos(ImVec2(start_x, current_y));

    // 输入框样式 - 420 x 52，圆角8
    float input_height = 52.0f;
    // 使用Inter Regular 16字体计算padding
    ImGui::PushFont(g_font_inter_regular_16);
    float frame_padding_y = (input_height - ImGui::GetTextLineHeight()) / 2.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(48.0f, frame_padding_y)); // pl-12, 垂直居中
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f); // 圆角8
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, content_fade)); // bg-surface-container-lowest
    
    // 检测输入框聚焦状态并触发动画
    bool is_license_focused = ImGui::IsItemFocused();
    if (is_license_focused != g_license_focus_anim.was_focused) {
        g_license_focus_anim.was_focused = is_license_focused;
        Params focus_params;
        focus_params.duration = 0.25f;
        focus_params.easing = Ease::OutQuad;
        if (is_license_focused) {
            g_license_focus_anim.focus_anim.Start(g_license_focus_anim.current_alpha, 1.0f, focus_params);
        } else {
            g_license_focus_anim.focus_anim.Start(g_license_focus_anim.current_alpha, 0.0f, focus_params);
        }
    }
    g_license_focus_anim.focus_anim.Update();
    g_license_focus_anim.current_alpha = g_license_focus_anim.focus_anim.GetCurrentValue();
    
    // 动态边框颜色 - 从未聚焦到聚焦的过渡 (灰色 -> 蓝色)
    float unfocused_r = 193.0f/255.0f, unfocused_g = 198.0f/255.0f, unfocused_b = 215.0f/255.0f;
    float focused_r = 0.0f/255.0f, focused_g = 88.0f/255.0f, focused_b = 188.0f/255.0f;
    float border_r = unfocused_r + (focused_r - unfocused_r) * g_license_focus_anim.current_alpha;
    float border_g = unfocused_g + (focused_g - unfocused_g) * g_license_focus_anim.current_alpha;
    float border_b = unfocused_b + (focused_b - unfocused_b) * g_license_focus_anim.current_alpha;
    float border_alpha = (0.3f + 0.7f * g_license_focus_anim.current_alpha) * content_fade;
    
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(border_r, border_g, border_b, border_alpha));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(193.0f/255.0f, 198.0f/255.0f, 215.0f/255.0f, content_fade)); // 占位符颜色
    
    // 使用PImGui::InputLicenseKeyAnimated - 带动画效果的输入框
    PImGui::InputLicenseKeyAnimated("##Login_license_key", g_license_key, sizeof(g_license_key), "XXXX-XXXX-XXXX-XXXX", content_width, content_fade);
    
    // 再次检测聚焦状态（在InputText之后）
    is_license_focused = ImGui::IsItemActive();
    if (is_license_focused != g_license_focus_anim.was_focused) {
        g_license_focus_anim.was_focused = is_license_focused;
        Params focus_params;
        focus_params.duration = 0.25f;
        focus_params.easing = Ease::OutQuad;
        if (is_license_focused) {
            g_license_focus_anim.focus_anim.Start(g_license_focus_anim.current_alpha, 1.0f, focus_params);
        } else {
            g_license_focus_anim.focus_anim.Start(g_license_focus_anim.current_alpha, 0.0f, focus_params);
        }
    }
    
    ImGui::PopFont();
    ImVec2 license_input_rect = ImGui::GetItemRectMin();

    // 左侧Key图标 (19x10)，应用淡入透明度
    float key_icon_width = 19.0f;
    float key_icon_height = 10.0f;
    float key_icon_y_offset = (input_height - key_icon_height) / 2.0f; // 图标垂直居中
    ImVec2 key_icon_pos = license_input_rect + ImVec2(16, key_icon_y_offset);
    if (g_key_texture.valid) {
        ImGui::GetWindowDrawList()->AddImage(
            g_key_texture.srv,
            key_icon_pos,
            key_icon_pos + ImVec2(key_icon_width, key_icon_height),
            ImVec2(0, 0),
            ImVec2(1, 1),
            image_alpha
        );
    } else {
        // 占位符
        ImGui::GetWindowDrawList()->AddRectFilled(
            key_icon_pos,
            key_icon_pos + ImVec2(key_icon_width, key_icon_height),
            outline_color,
            0.0f
        );
    }
    
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    
    current_y += input_height + 24; // 输入框高度 + 间距
    
    // ========== Email输入框 ==========
    // 标签 - Inter Semibold 12
    ImGui::PushFont(g_font_inter_semibold_12);
    const char* email_label = "ACCOUNT CREDENTIALS";
    ImGui::SetCursorPos(ImVec2(start_x + 4, current_y));
    ImGui::TextColored(ImVec4(65.0f/255.0f, 71.0f/255.0f, 85.0f/255.0f, content_fade), email_label);
    ImGui::PopFont();
    
    current_y += ImGui::GetTextLineHeight() + 8;
    
    // 输入框 - Inter Regular 16
    ImGui::SetCursorPos(ImVec2(start_x, current_y));
    ImGui::PushFont(g_font_inter_regular_16);
    
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(48.0f, frame_padding_y));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f); // 圆角8
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, content_fade));
    
    // 检测Email输入框聚焦状态并触发动画
    bool is_email_focused = ImGui::IsItemFocused();
    if (is_email_focused != g_email_focus_anim.was_focused) {
        g_email_focus_anim.was_focused = is_email_focused;
        Params focus_params;
        focus_params.duration = 0.25f;
        focus_params.easing = Ease::OutQuad;
        if (is_email_focused) {
            g_email_focus_anim.focus_anim.Start(g_email_focus_anim.current_alpha, 1.0f, focus_params);
        } else {
            g_email_focus_anim.focus_anim.Start(g_email_focus_anim.current_alpha, 0.0f, focus_params);
        }
    }
    g_email_focus_anim.focus_anim.Update();
    g_email_focus_anim.current_alpha = g_email_focus_anim.focus_anim.GetCurrentValue();
    
    // 动态边框颜色 - 从未聚焦到聚焦的过渡 (灰色 -> 蓝色)
    float email_border_r = unfocused_r + (focused_r - unfocused_r) * g_email_focus_anim.current_alpha;
    float email_border_g = unfocused_g + (focused_g - unfocused_g) * g_email_focus_anim.current_alpha;
    float email_border_b = unfocused_b + (focused_b - unfocused_b) * g_email_focus_anim.current_alpha;
    float email_border_alpha = (0.3f + 0.7f * g_email_focus_anim.current_alpha) * content_fade;
    
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(email_border_r, email_border_g, email_border_b, email_border_alpha));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(193.0f/255.0f, 198.0f/255.0f, 215.0f/255.0f, content_fade)); // 占位符颜色
    ImGui::PushItemWidth(content_width);

    ImGui::InputTextWithHint("##Login_email", "admin@precision.io", g_email, sizeof(g_email));
    
    // 再次检测聚焦状态（在InputText之后）
    is_email_focused = ImGui::IsItemActive();
    if (is_email_focused != g_email_focus_anim.was_focused) {
        g_email_focus_anim.was_focused = is_email_focused;
        Params focus_params;
        focus_params.duration = 0.25f;
        focus_params.easing = Ease::OutQuad;
        if (is_email_focused) {
            g_email_focus_anim.focus_anim.Start(g_email_focus_anim.current_alpha, 1.0f, focus_params);
        } else {
            g_email_focus_anim.focus_anim.Start(g_email_focus_anim.current_alpha, 0.0f, focus_params);
        }
    }
    
    ImGui::PopFont();
    ImVec2 email_input_rect = ImGui::GetItemRectMin();

    // 左侧Email图标 (17x17)，应用淡入透明度
    float email_icon_width = 15.0f;
    float email_icon_height = 15.0f;
    float email_icon_y_offset = (input_height - email_icon_height) / 2.0f; // 图标垂直居中
    ImVec2 email_icon_pos = email_input_rect + ImVec2(16, email_icon_y_offset);
    if (g_email_texture.valid) {
        ImGui::GetWindowDrawList()->AddImage(
            g_email_texture.srv,
            email_icon_pos,
            email_icon_pos + ImVec2(email_icon_width, email_icon_height),
            ImVec2(0, 0),
            ImVec2(1, 1),
            image_alpha
        );
    } else {
        // 占位符
        ImGui::GetWindowDrawList()->AddRectFilled(
            email_icon_pos,
            email_icon_pos + ImVec2(email_icon_width, email_icon_height),
            outline_color,
            0.0f
        );
    }
    
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
    ImGui::PopItemWidth();
    
    current_y += input_height + 32; // 输入框高度 + pt-2
    
    // ========== LOGIN按钮 ==========
    ImGui::SetCursorPos(ImVec2(start_x, current_y));

    // 按钮样式 - 420 x 52，圆角8 - Manrope Bold 18
    ImGui::PushFont(g_font_manrope_bold_18);
    float button_height = 52.0f;
    float button_padding_y = (button_height - ImGui::GetTextLineHeight()) / 2.0f;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, button_padding_y)); // 垂直居中
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f); // 圆角8
    ImGui::PushStyleColor(ImGuiCol_Button, primary_color); // bg-primary，已应用form_fade
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 91, 193, alpha_byte)); // hover:bg-surface-tint
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 78, 169, alpha_byte));
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, alpha_byte)); // text-on-primary，应用淡入透明度
    
    ImGui::PushFont(g_font_inter_semibold_16);
    if (ImGui::Button("LOGIN##Login_button", ImVec2(content_width, button_height))) {
        // 登录逻辑
        // 验证License Key和Email
        if (strlen(g_license_key) > 0 && strlen(g_email) > 0) {
            // 切换到初始化页面（带动画过渡）
            SwitchToPage(PAGE_INITIALIZING, 0.5f);
            printf("Login attempt with License: %s, Email: %s\n", g_license_key, g_email);
        }
    }
    
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);
    ImGui::PopFont(); // Inter Semibold 16
    ImGui::PopFont(); // Manrope Bold 18
    
    current_y += button_height + 44; // 按钮高度 + 间距（增加Forgot Password?和按钮的间距）
    
    // ========== Forgot Password? 链接 ==========
    const char* forgot_text = "Forgot Password?";
    ImVec2 forgot_size = ImGui::CalcTextSize(forgot_text);
    ImGui::SetCursorPos(ImVec2(start_x + (content_width - forgot_size.x) / 2.0f, current_y));
    
    // 可点击的文本按钮样式
    ImVec2 forgot_button_pos = ImGui::GetCursorScreenPos();
    ImRect forgot_button_bb(forgot_button_pos, forgot_button_pos + ImVec2(forgot_size.x + 10, forgot_size.y + 5));
    bool forgot_hovered = ImGui::IsMouseHoveringRect(forgot_button_bb.Min, forgot_button_bb.Max);
    bool forgot_clicked = ImGui::IsMouseClicked(0) && forgot_hovered;
    
    // 根据悬停状态改变颜色
    ImU32 forgot_text_color = forgot_hovered ? IM_COL32(0, 88, 188, 255) : IM_COL32(65, 71, 85, 255);
    ImGui::GetWindowDrawList()->AddText(
        forgot_button_pos + ImVec2(5, 2),
        forgot_text_color,
        forgot_text
    );
    
    if (forgot_clicked) {
        printf("Forgot Password clicked\n");
        // TODO: 实现忘记密码逻辑，如打开浏览器或显示重置密码弹窗
    }
    
    current_y += forgot_size.y + 32; // 增加Forgot Password?和分隔线的间距
    
    // ========== 分隔线和"or"文本 ==========
    float separator_padding = 20.0f;
    ImGui::PushFont(g_font_inter_semibold_10);
    float or_text_width = ImGui::CalcTextSize("OR").x;
    ImGui::PopFont();
    float line_width = (content_width - or_text_width - separator_padding * 2) / 2.0f;
    
    // 使用SetCursorPos来正确对齐分割线
    ImGui::SetCursorPos(ImVec2(start_x, current_y));
    ImVec2 separator_screen_pos = ImGui::GetCursorScreenPos();
    
    // 左侧分隔线
    ImGui::GetWindowDrawList()->AddLine(
        separator_screen_pos,
        separator_screen_pos + ImVec2(line_width, 0),
        IM_COL32(193, 198, 215, 51), // outline-variant/20
        1.0f
    );
    
    // "or"文本 - 垂直居中于分割线 - Inter Semibold 12
    ImVec2 or_pos = separator_screen_pos + ImVec2(line_width + separator_padding, -6); // 调整垂直位置
    if (g_font_inter_semibold_10) {
        ImGui::GetWindowDrawList()->AddText(g_font_inter_semibold_10, 12.0f, or_pos, outline_variant, "OR");
    } else {
        ImGui::GetWindowDrawList()->AddText(or_pos, outline_variant, "OR");
    }
    
    // 右侧分隔线
    ImGui::GetWindowDrawList()->AddLine(
        separator_screen_pos + ImVec2(line_width + or_text_width + separator_padding * 2, 0),
        separator_screen_pos + ImVec2(content_width, 0),
        IM_COL32(193, 198, 215, 51),
        1.0f
    );
    
    current_y += 8 + 24; // 分隔线高度 + 间距（增加or和Register Account的间距）
    
    // ========== Register Account链接 ==========
    const char* register_text = "Register Account";
    ImVec2 register_size = ImGui::CalcTextSize(register_text);
    ImGui::SetCursorPos(ImVec2(start_x + (content_width - register_size.x) / 2.0f, current_y));
    
    // 可点击的文本按钮样式 - 蓝色文字
    ImVec2 register_button_pos = ImGui::GetCursorScreenPos();
    ImRect register_button_bb(register_button_pos, register_button_pos + ImVec2(register_size.x + 10, register_size.y + 5));
    bool register_hovered = ImGui::IsMouseHoveringRect(register_button_bb.Min, register_button_bb.Max);
    bool register_clicked = ImGui::IsMouseClicked(0) && register_hovered;
    
    // 根据悬停状态改变颜色
    ImU32 register_text_color = register_hovered ? IM_COL32(0, 88, 188, 255) : IM_COL32(64, 94, 150, 255);
    ImGui::GetWindowDrawList()->AddText(
        register_button_pos + ImVec2(5, 2),
        register_text_color,
        register_text
    );
    
    if (register_clicked) {
        printf("Register Account clicked\n");
        // TODO: 实现注册逻辑，如切换到注册界面或打开浏览器
    }
    
    current_y += register_size.y + 94; // 增加Register Account和System Status的间距
    
    // ========== 系统状态信息 ==========
    // System Status - Inter Regular 10, Operational - Inter Semibold 10
    const char* status_label = "System Status: ";
    const char* status_value = "Operational";
    
    ImGui::PushFont(g_font_inter_regular_10);
    ImVec2 status_label_size = ImGui::CalcTextSize(status_label);
    ImGui::PopFont();
    
    ImGui::PushFont(g_font_inter_semibold_10);
    ImVec2 status_value_size = ImGui::CalcTextSize(status_value);
    ImGui::PopFont();
    
    float status_total_width = status_label_size.x + status_value_size.x;
    float status_x = start_x + (content_width - status_total_width) / 2.0f;
    
    // 绘制System Status标签
    ImGui::SetCursorPos(ImVec2(status_x, current_y));
    ImGui::PushFont(g_font_inter_regular_10);
    ImGui::TextColored(ImVec4(113.0f/255.0f, 119.0f/255.0f, 134.0f/255.0f, 1.0f), status_label);
    ImGui::PopFont();
    
    // 绘制Operational值
    ImGui::SameLine(0, 0);
    ImGui::PushFont(g_font_inter_semibold_10);
    ImGui::TextColored(ImVec4(0.0f/255.0f, 88.0f/255.0f, 188.0f/255.0f, 1.0f), status_value);
    ImGui::PopFont();
    
    current_y += status_label_size.y + 16;
    
    // Build版本 - Inter Regular 10
    const char* build_text = "Build v4.2.0-Alpha";
    ImGui::PushFont(g_font_inter_regular_10);
    ImVec2 build_size = ImGui::CalcTextSize(build_text);
    ImGui::SetCursorPos(ImVec2(start_x + (content_width - build_size.x) / 2.0f, current_y));
    ImGui::TextColored(ImVec4(113.0f/255.0f, 119.0f/255.0f, 134.0f/255.0f, 0.4f), build_text);
    ImGui::PopFont();
    
    // ========== 底部页脚 ==========
    float footer_y = 1024.0f - 60.0f; // 距离窗口底部60px
    const char* copyright_text = "© 2026 NOVOCAINE Clinical Systems";
    const char* support_text = "Support";
    const char* doc_text = "Documentation";
    const char* terms_text = "Terms of Service";
    
    // 计算总宽度 - 使用Inter Regular 12字体
    float spacing = 24.0f;
    ImGui::PushFont(g_font_inter_regular_12);
    ImVec2 copyright_size = ImGui::CalcTextSize(copyright_text);
    ImVec2 support_size = ImGui::CalcTextSize(support_text);
    ImVec2 doc_size = ImGui::CalcTextSize(doc_text);
    ImVec2 terms_size = ImGui::CalcTextSize(terms_text);
    ImGui::PopFont();
    
    float footer_total_width = copyright_size.x + spacing + support_size.x + spacing + doc_size.x + spacing + terms_size.x;
    float footer_start_x = (window_width - footer_total_width) / 2.0f;
    
    // 绘制页脚文本 - 可点击的链接
    ImU32 footer_normal_color = IM_COL32(113, 119, 134, 153); // 0.6透明度
    ImU32 footer_hover_color = IM_COL32(0, 88, 188, 255); // 悬停变蓝色
    
    // 获取窗口的屏幕位置和高度
    ImVec2 window_pos = ImGui::GetWindowPos();
    float window_height = ImGui::GetWindowHeight();
    
    // 计算每个文本的屏幕坐标X位置
    float current_footer_x = window_pos.x + start_x + (content_width - footer_total_width) / 2.0f;
    // 使用窗口高度计算页脚Y位置，使其跟随窗口移动
    float footer_text_y = window_pos.y + window_height - 40.0f; // 距离窗口底部40px
    
    // Copyright文本（不可点击）- Inter Regular 12
    ImVec2 copyright_pos = ImVec2(current_footer_x, footer_text_y);
    if (g_font_inter_regular_12) {
        ImGui::GetWindowDrawList()->AddText(g_font_inter_regular_12, 12.0f, copyright_pos, footer_normal_color, copyright_text);
    } else {
        ImGui::GetWindowDrawList()->AddText(copyright_pos, footer_normal_color, copyright_text);
    }
    current_footer_x += copyright_size.x + spacing;
    
    // Support链接 - Inter Regular 12
    ImVec2 support_pos = ImVec2(current_footer_x, footer_text_y);
    ImRect support_bb(support_pos, support_pos + ImVec2(support_size.x, support_size.y + 4));
    bool support_hovered = ImGui::IsMouseHoveringRect(support_bb.Min, support_bb.Max);
    bool support_clicked = ImGui::IsMouseClicked(0) && support_hovered;
    if (g_font_inter_regular_12) {
        ImGui::GetWindowDrawList()->AddText(g_font_inter_regular_12, 12.0f, support_pos, support_hovered ? footer_hover_color : footer_normal_color, support_text);
    } else {
        ImGui::GetWindowDrawList()->AddText(support_pos, support_hovered ? footer_hover_color : footer_normal_color, support_text);
    }
    if (support_clicked) {
        printf("Support clicked\n");
        // TODO: 打开支持页面或弹窗
    }
    current_footer_x += support_size.x + spacing;
    
    // Documentation链接 - Inter Regular 12
    ImVec2 doc_pos = ImVec2(current_footer_x, footer_text_y);
    ImRect doc_bb(doc_pos, doc_pos + ImVec2(doc_size.x, doc_size.y + 4));
    bool doc_hovered = ImGui::IsMouseHoveringRect(doc_bb.Min, doc_bb.Max);
    bool doc_clicked = ImGui::IsMouseClicked(0) && doc_hovered;
    if (g_font_inter_regular_12) {
        ImGui::GetWindowDrawList()->AddText(g_font_inter_regular_12, 12.0f, doc_pos, doc_hovered ? footer_hover_color : footer_normal_color, doc_text);
    } else {
        ImGui::GetWindowDrawList()->AddText(doc_pos, doc_hovered ? footer_hover_color : footer_normal_color, doc_text);
    }
    if (doc_clicked) {
        printf("Documentation clicked\n");
        // TODO: 打开文档页面
    }
    current_footer_x += doc_size.x + spacing;
    
    // Terms of Service链接 - Inter Regular 12
    ImVec2 terms_pos = ImVec2(current_footer_x, footer_text_y);
    ImRect terms_bb(terms_pos, terms_pos + ImVec2(terms_size.x, terms_size.y + 4));
    bool terms_hovered = ImGui::IsMouseHoveringRect(terms_bb.Min, terms_bb.Max);
    bool terms_clicked = ImGui::IsMouseClicked(0) && terms_hovered;
    if (g_font_inter_regular_12) {
        ImGui::GetWindowDrawList()->AddText(g_font_inter_regular_12, 12.0f, terms_pos, terms_hovered ? footer_hover_color : footer_normal_color, terms_text);
    } else {
        ImGui::GetWindowDrawList()->AddText(terms_pos, terms_hovered ? footer_hover_color : footer_normal_color, terms_text);
    }
    if (terms_clicked) {
        printf("Terms of Service clicked\n");
        // TODO: 打开服务条款页面
    }
    
    ImGui::PopID();
 }
 
 // 渲染主页面内容
void RenderMainContentPage(const MorphAnimation& morph) {
    ImGui::PushID("MainPage");
    
    // 加载主页面图片（只加载一次）
    if (!g_main_images_loaded) {
        g_icon_search = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_search.png");
        g_icon_game = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_game.png");
        g_icon_cpu = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_cpu.png");
        g_icon_chrome = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_chrome.png");
        g_icon_cmd = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_cmd.png");
        g_icon_process = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_process.png");
        g_icon_protect = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_protect.png");
        g_icon_path = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_path.png");
        g_icon_file = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_file.png");
        g_icon_inject = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_inject.png");
        g_icon_setting = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_setting.png");
        g_icon_dump = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_dump.png");
        g_icon_viewdump = LoadPNGTexture(Window::WindowDatas.g_pd3dDevice, "images/Icon_viewdump.png");
        g_main_images_loaded = true;
    }
    
    // 主页面始终渲染（不使用alpha检查，因为主页面是最终状态）
    RenderMainPage(
        g_main_page_state,
        ImGui::GetWindowWidth(),
        ImGui::GetWindowHeight(),
        g_font_inter_semibold_18,       // LOGO字体 - Inter Semibold 18
        g_font_inter_semibold_12,       // 导航字体 - Inter Semibold 12
        g_font_manrope_extrabold_48,    // 进程标题 - Manrope ExtraBold 48
        g_font_inter_medium_16,         // PID等一行 - Inter Medium 16
        g_font_inter_semibold_10,       // 小字体 - Inter Semibold 10
        g_font_inter_regular_10,        // 等宽字体（日志）- Inter Regular 10
        // 新增字体
        g_font_inter_semibold_20,       // 选中项 - Inter Semibold 20
        g_font_inter_medium_14,         // 未选中项 - Inter Medium 14
        g_font_inter_semibold_11,       // INJECTION TARGET - Inter Semibold 11
        g_font_inter_semibold_14,       // DLL名 - Inter Semibold 14
        g_font_inter_semibold_12,       // BROWSE - Inter Semibold 12
        g_font_inter_regular_11,        // Size等文件信息 - Inter Regular 11
        g_font_manrope_bold_18,         // 注入按钮 - Manrope Bold 18
        g_font_manrope_bold_14          // 其他按钮 - Manrope Bold 14
    );
    
    ImGui::PopID();
}
 
 // 渲染初始化页面内容
 void RenderInitializingContent(const MorphAnimation& morph) {
     ImGui::PushID("InitPage");
     
     // 如果内容不可见，跳过渲染但仍需PopID
     if (morph.init_content_alpha >= 0.01f) {
         // 使用组件化初始化页面，传入形变动画参数
         RenderInitializingPage(
             g_init_page_state,
             ImGui::GetWindowWidth(),
             ImGui::GetWindowHeight(),
             g_font_manrope_extrabold_32,  // 标题字体 (40px)
             g_font_inter_regular_12,       // 副标题字体 (12px)
             g_font_inter_regular_14,       // 状态字体 (14px)
             g_font_inter_regular_12,       // 服务器状态字体 (12px)
             g_font_inter_regular_10,       // 版本号字体 (10px)
             morph                          // 形变动画参数
         );
     }
     
     ImGui::PopID();
 }


