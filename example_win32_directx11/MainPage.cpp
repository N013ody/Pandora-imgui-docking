#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include "MainPage.h"
#include "EasingAnimationSystem.h"
#include "DX11Util.h"
#include <cmath>

// 外部图片纹理（在GuiMain.cpp中定义）
extern ImageTexture g_icon_search;
extern ImageTexture g_icon_game;
extern ImageTexture g_icon_cpu;
extern ImageTexture g_icon_chrome;
extern ImageTexture g_icon_cmd;
extern ImageTexture g_icon_process;
extern ImageTexture g_icon_protect;
extern ImageTexture g_icon_path;
extern ImageTexture g_icon_file;
extern ImageTexture g_icon_inject;
extern ImageTexture g_icon_setting;
extern ImageTexture g_icon_question;
extern ImageTexture g_icon_dump;
extern ImageTexture g_icon_viewdump;

// 外部字体（在GuiMain.cpp中定义）
extern ImFont* g_font_manrope_bold_11;

// 颜色定义 (从 MAINPAGE.html 提取)
#define MP_COLOR_BACKGROUND IM_COL32(248, 249, 250, 255)      // #f8f9fa
#define MP_COLOR_SURFACE IM_COL32(255, 255, 255, 255)         // #ffffff
#define MP_COLOR_SURFACE_LOW IM_COL32(243, 244, 245, 255)     // #f3f4f5
#define MP_COLOR_SURFACE_HIGH IM_COL32(231, 232, 233, 255)    // #e7e8e9
#define MP_COLOR_SURFACE_HIGHEST IM_COL32(225, 227, 228, 255) // #e1e3e4
#define MP_COLOR_PRIMARY IM_COL32(0, 88, 188, 255)            // #0058bc
#define MP_COLOR_PRIMARY_CONTAINER IM_COL32(0, 112, 235, 255) // #0070eb
#define MP_COLOR_ON_SURFACE IM_COL32(25, 28, 29, 255)         // #191c1d
#define MP_COLOR_ON_SURFACE_VARIANT IM_COL32(65, 71, 85, 255) // #414755
#define MP_COLOR_OUTLINE IM_COL32(113, 119, 134, 255)         // #717786
#define MP_COLOR_OUTLINE_VARIANT IM_COL32(193, 198, 215, 255) // #c1c6d7

// 渲染主页面
void RenderMainPage(MainPageState& state, float window_width, float window_height,
                    ImFont* font_logo,           // Inter Semibold 18 - LOGO
                    ImFont* font_nav,            // Inter Semibold 12 - 导航
                    ImFont* font_title,          // Manrope ExtraBold 48 - 进程标题
                    ImFont* font_body,           // Inter Medium 16 - PID等一行
                    ImFont* font_small,          // Inter Semibold 10 - 小字体
                    ImFont* font_mono,           // 等宽字体（日志）
                    // 新增字体参数
                    ImFont* font_process_selected,   // Inter Semibold 20 - 选中项
                    ImFont* font_process_unselected, // Inter Medium 14 - 未选中项
                    ImFont* font_inject_title,       // Inter Semibold 11 - INJECTION TARGET
                    ImFont* font_dll_name,           // Inter Semibold 14 - DLL名
                    ImFont* font_browse,             // Inter Semibold 12 - BROWSE
                    ImFont* font_file_info,          // Inter Regular 11 - Size等文件信息
                    ImFont* font_button_inject,      // Manrope Bold 18 - 注入按钮
                    ImFont* font_button_other) {     // Manrope Bold 14 - 其他按钮
    // 更新标签指示器动画
    const float anim_speed = 0.15f;
    state.tab_indicator_x += (state.tab_indicator_target - state.tab_indicator_x) * anim_speed;
    
    ImVec2 window_pos = ImGui::GetWindowPos();
    
    // 渲染顶部导航栏 (固定高度 60)
    RenderTopBar(state, window_width, window_pos, font_logo, font_nav);
    
    // 计算布局高度（留出底部栏空间）
    float bottom_bar_height = 28.0f;
    float content_height = window_height - 60.0f - bottom_bar_height;
    float sidebar_width = window_width / 3.0f;
    
    ImGui::SetCursorPos(ImVec2(0, 60));
    
    // 侧边栏 Child Window
    ImGui::PushStyleColor(ImGuiCol_ChildBg, MP_COLOR_SURFACE_LOW);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0);
    if (ImGui::BeginChild("Sidebar", ImVec2(sidebar_width, content_height), false)) {
        RenderSidebarContent(state, sidebar_width, content_height, font_title, font_body, font_small,
                             font_process_selected, font_process_unselected);
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    
    // 主内容区域 Child Window
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_ChildBg, MP_COLOR_BACKGROUND);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0);
    if (ImGui::BeginChild("MainContent", ImVec2(window_width - sidebar_width, content_height), false)) {
        RenderMainContentArea(state, window_width - sidebar_width, content_height, 
                              font_title, font_body, font_small, font_mono,
                              font_inject_title, font_dll_name, font_browse, font_file_info,
                              font_button_inject, font_button_other);
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    
    // 渲染底部栏（独立于主内容区域）
    RenderBottomBar(state, window_width, window_height, bottom_bar_height, window_pos, font_small);
    
    // 渲染模态对话框（如果打开）
    if (state.modal_dialog_state.is_open || state.modal_dialog_state.open_animation > 0.0f) {
        float delta_time = ImGui::GetIO().DeltaTime;
        state.modal_dialog_state.UpdateAnimation(delta_time);
        RenderModalDialog(state.modal_dialog_state, font_title, font_body, font_small);
    }
}

// 渲染顶部导航栏
void RenderTopBar(MainPageState& state, float window_width, ImVec2 window_pos,
                  ImFont* font_logo, ImFont* font_nav) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float y = window_pos.y;
    
    // TopBar 背景 - 白色，底部边框
    float topbar_height = 60.0f;
    draw_list->AddRectFilled(
        ImVec2(window_pos.x, y),
        ImVec2(window_pos.x + window_width, y + topbar_height),
        MP_COLOR_SURFACE
    );
    draw_list->AddLine(
        ImVec2(window_pos.x, y + topbar_height - 1),
        ImVec2(window_pos.x + window_width, y + topbar_height - 1),
        MP_COLOR_SURFACE_HIGHEST,
        1.0f
    );
    
    // LOGO - NOVOCAINES - Inter Semibold 18
    float logo_x = window_pos.x + 20.0f;
    float logo_y = y + (topbar_height - 18.0f) / 2.0f;
    if (font_logo) {
        draw_list->AddText(font_logo, 18.0f, ImVec2(logo_x, logo_y), MP_COLOR_ON_SURFACE, "NOVOCAINES");
    } else {
        draw_list->AddText(ImVec2(logo_x, logo_y), MP_COLOR_ON_SURFACE, "NOVOCAINES");
    }
    
    // 导航标签 - DASHBOARD, PROCESSES, RESOURCES - Inter Semibold 12
    const char* tabs[] = {"DASHBOARD", "PROCESSES", "RESOURCES"};
    float tab_start_x = logo_x + 140.0f;
    float tab_base_y = y + (topbar_height - 12.0f) / 2.0f;
    float tab_spacing = 24.0f;
    
    float tab_positions[3];
    float tab_widths[3];
    
    float current_x = tab_start_x;
    for (int i = 0; i < 3; i++) {
        const char* tab_name = tabs[i];
        ImVec2 text_size = font_nav ? font_nav->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, tab_name) : ImGui::CalcTextSize(tab_name);
        
        tab_positions[i] = current_x;
        tab_widths[i] = text_size.x;
        
        float tab_y = tab_base_y;
        if (i == state.selected_tab) {
            tab_y -= 3.0f;
        }
        
        ImRect tab_rect(ImVec2(current_x, y + 10), ImVec2(current_x + text_size.x, y + topbar_height - 10));
        bool hovered = ImGui::IsMouseHoveringRect(tab_rect.Min, tab_rect.Max);
        bool clicked = hovered && ImGui::IsMouseClicked(0);
        
        if (clicked) {
            state.selected_tab = i;
        }
        
        ImU32 text_color = (i == state.selected_tab) ? MP_COLOR_PRIMARY : MP_COLOR_OUTLINE;
        
        if (font_nav) {
            draw_list->AddText(font_nav, 12.0f, ImVec2(current_x, tab_y), text_color, tab_name);
        } else {
            draw_list->AddText(ImVec2(current_x, tab_y), text_color, tab_name);
        }
        
        current_x += text_size.x + tab_spacing;
    }
    
    // 绘制选中标签的下划线指示器 - 更靠近标签（2px间距）
    int selected = state.selected_tab;
    float indicator_x = tab_positions[selected] - window_pos.x;
    float indicator_width = tab_widths[selected];
    float indicator_y = tab_base_y + 12.0f + 2.0f;  // 文字底部 + 2px
    
    state.tab_indicator_target = indicator_x;
    
    draw_list->AddRectFilled(
        ImVec2(window_pos.x + state.tab_indicator_x, indicator_y),
        ImVec2(window_pos.x + state.tab_indicator_x + indicator_width, indicator_y + 2.0f),
        MP_COLOR_PRIMARY,
        1.0f
    );
    
    // 右侧帮助和设置按钮
    float button_size = 32.0f;
    float button_y = y + (topbar_height - button_size) / 2.0f;
    float right_x = window_pos.x + window_width - 100.0f;
    
    // 第一个按钮 - Icon_question.png (17x17)
    ImRect help_rect(ImVec2(right_x, button_y), ImVec2(right_x + button_size, button_y + button_size));
    bool help_hovered = ImGui::IsMouseHoveringRect(help_rect.Min, help_rect.Max);
    draw_list->AddRectFilled(help_rect.Min, help_rect.Max, 
        help_hovered ? MP_COLOR_SURFACE_HIGH : MP_COLOR_SURFACE, 8.0f);
    if (g_icon_question.valid) {
        float icon_x = floorf(right_x + (button_size - 17.0f) / 2.0f);
        float icon_y = floorf(button_y + (button_size - 17.0f) / 2.0f);
        draw_list->AddImage(g_icon_question.srv, 
            ImVec2(icon_x, icon_y), 
            ImVec2(icon_x + 17.0f, icon_y + 17.0f));
    }
    
    right_x += button_size + 10.0f;
    // 第二个设置按钮 - Icon_setting.png (17x17)
    ImRect settings_rect(ImVec2(right_x, button_y), ImVec2(right_x + button_size, button_y + button_size));
    bool settings_hovered = ImGui::IsMouseHoveringRect(settings_rect.Min, settings_rect.Max);
    draw_list->AddRectFilled(settings_rect.Min, settings_rect.Max,
        settings_hovered ? MP_COLOR_SURFACE_HIGH : MP_COLOR_SURFACE, 8.0f);
    if (g_icon_setting.valid) {
        float icon_x = floorf(right_x + (button_size - 17.0f) / 2.0f);
        float icon_y = floorf(button_y + (button_size - 17.0f) / 2.0f);
        draw_list->AddImage(g_icon_setting.srv, 
            ImVec2(icon_x, icon_y), 
            ImVec2(icon_x + 17.0f, icon_y + 17.0f));
    }
}

// 渲染底部栏（独立于主内容区域）
void RenderBottomBar(MainPageState& state, float window_width, float window_height, 
                     float bar_height, ImVec2 window_pos, ImFont* font_small) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float y = window_pos.y + window_height - bar_height;
    
    draw_list->AddRectFilled(
        ImVec2(window_pos.x, y),
        ImVec2(window_pos.x + window_width, y + bar_height),
        MP_COLOR_BACKGROUND
    );
    
    draw_list->AddLine(
        ImVec2(window_pos.x, y),
        ImVec2(window_pos.x + window_width, y),
        MP_COLOR_SURFACE_HIGHEST,
        1.0f
    );
    
    // 左侧标签 - LOGS更靠左，SECURITY和NETWORKS有间距 - Inter Semibold 10
    float footer_y = y + (bar_height - 10.0f) / 2.0f;
    
    // LOGS - 靠左
    const char* logs_text = "LOGS";
    float footer_x = window_pos.x + 24.0f;
    if (font_small) {
        draw_list->AddText(font_small, 10.0f, ImVec2(footer_x, footer_y), MP_COLOR_PRIMARY, logs_text);
    } else {
        draw_list->AddText(ImVec2(footer_x, footer_y), MP_COLOR_PRIMARY, logs_text);
    }
    
    // NETWORKS - 与LOGS有间距
    const char* networks_text = "NETWORKS";
    if (font_small) {
        footer_x += font_small->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, logs_text).x + 40.0f;
        draw_list->AddText(font_small, 10.0f, ImVec2(footer_x, footer_y), MP_COLOR_OUTLINE, networks_text);
    } else {
        footer_x += 70.0f;
        draw_list->AddText(ImVec2(footer_x, footer_y), MP_COLOR_OUTLINE, networks_text);
    }
    
    // SECURITY - 与NETWORKS有间距
    const char* security_text = "SECURITY";
    if (font_small) {
        footer_x += font_small->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, networks_text).x + 40.0f;
        draw_list->AddText(font_small, 10.0f, ImVec2(footer_x, footer_y), MP_COLOR_OUTLINE, security_text);
    } else {
        footer_x += 90.0f;
        draw_list->AddText(ImVec2(footer_x, footer_y), MP_COLOR_OUTLINE, security_text);
    }
    
    // 右侧版本信息 - 更靠右 - Inter Semibold 10
    const char* version = "SYSTEM READY | V2.4.0";
    if (font_small) {
        ImVec2 version_size = font_small->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, version);
        draw_list->AddText(font_small, 10.0f, 
            ImVec2(window_pos.x + window_width - version_size.x - 24.0f, footer_y), MP_COLOR_OUTLINE, version);
    }
}

// 渲染侧边栏内容
void RenderSidebarContent(MainPageState& state, float width, float height,
                          ImFont* font_title, ImFont* font_body, ImFont* font_small,
                          ImFont* font_process_selected, ImFont* font_process_unselected) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    
    float content_x = 24.0f;
    float current_y = 24.0f;
    
    // Active Environments - 使用Manrope Bold 14号字体
    const char* title = "ACTIVE ENVIRONMENTS";
    if (g_font_manrope_bold_11) {
        // 使用Manrope Bold 14号字体
        float title_font_size = 14.0f;
        float title_x = pos.x + content_x;
        float char_spacing = 3.0f;  // 字间距
        
        // 手动绘制每个字符，使用对齐的坐标避免模糊
        for (size_t i = 0; i < strlen(title); i++) {
            char c[2] = {title[i], '\0'};
            // 使用floor对齐到像素边界，避免子像素模糊
            float fx = floorf(title_x);
            float fy = floorf(pos.y + current_y);
            draw_list->AddText(g_font_manrope_bold_11, title_font_size, ImVec2(fx, fy), MP_COLOR_ON_SURFACE_VARIANT, c);
            ImVec2 char_size = g_font_manrope_bold_11->CalcTextSizeA(title_font_size, FLT_MAX, 0.0f, c);
            title_x += char_size.x + char_spacing;
        }
    } else {
        draw_list->AddText(ImVec2(pos.x + content_x, pos.y + current_y), MP_COLOR_ON_SURFACE_VARIANT, title);
    }
    current_y += 38.0f;
    
    float search_width = width - 48.0f;
    float search_height = 40.0f;
    ImRect search_rect(ImVec2(pos.x + content_x, pos.y + current_y), 
                       ImVec2(pos.x + content_x + search_width, pos.y + current_y + search_height));
    
    draw_list->AddRectFilled(search_rect.Min, search_rect.Max, MP_COLOR_SURFACE_HIGHEST, 12.0f);
    
    // 搜索图标 - Icon_search.png (11x11)
    if (g_icon_search.valid) {
        float icon_x = floorf(pos.x + content_x + 14.0f);
        float icon_y = floorf(pos.y + current_y + (search_height - 11.0f) / 2.0f);
        draw_list->AddImage(g_icon_search.srv, ImVec2(icon_x, icon_y), ImVec2(icon_x + 11.0f, icon_y + 11.0f));
    }
    
    ImGui::SetCursorScreenPos(ImVec2(pos.x + content_x + 40.0f, pos.y + current_y + 8.0f));
    ImGui::PushItemWidth(search_width - 50.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    ImGui::InputTextWithHint("##search", "Filter processes...", state.search_buffer, sizeof(state.search_buffer));
    ImGui::PopStyleColor();
    ImGui::PopItemWidth();
    
    current_y += search_height + 16.0f;
    
    float list_x = 12.0f;
    
    // 搜索过滤
    std::string search_filter = state.search_buffer;
    // 转换为小写进行不区分大小写的搜索
    auto to_lower = [](std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    };
    std::string search_lower = to_lower(search_filter);
    
    for (size_t i = 0; i < state.processes.size(); i++) {
        const auto& proc = state.processes[i];
        
        // 搜索过滤：如果输入了搜索关键字，只显示匹配的进程
        if (!search_filter.empty()) {
            std::string proc_name_lower = to_lower(proc.name);
            std::string proc_pid_lower = to_lower(proc.pid);
            if (proc_name_lower.find(search_lower) == std::string::npos && 
                proc_pid_lower.find(search_lower) == std::string::npos) {
                continue; // 跳过不匹配的进程
            }
        }
        
        float item_height = 48.0f;
        float item_width = width - 24.0f;
        
        ImRect item_rect(ImVec2(pos.x + list_x, pos.y + current_y), 
                         ImVec2(pos.x + list_x + item_width, pos.y + current_y + item_height));
        bool hovered = ImGui::IsMouseHoveringRect(item_rect.Min, item_rect.Max);
        bool clicked = hovered && ImGui::IsMouseClicked(0);
        
        if (clicked) {
            state.selected_process = (int)i;
        }
        
        if ((int)i == state.selected_process) {
            draw_list->AddRectFilled(item_rect.Min, item_rect.Max, MP_COLOR_SURFACE, 12.0f);
        } else if (hovered) {
            draw_list->AddRectFilled(item_rect.Min, item_rect.Max, MP_COLOR_SURFACE_HIGH, 12.0f);
        }
        
        // 进程类型图标
        ImageTexture* type_icon = nullptr;
        float icon_w = 17.0f, icon_h = 15.0f;
        switch (proc.type) {
            case MainPageState::PROCESS_TYPE_GAME:
                type_icon = &g_icon_game;
                icon_w = 17.0f; icon_h = 12.0f;
                break;
            case MainPageState::PROCESS_TYPE_SYSTEM:
                type_icon = &g_icon_cpu;
                icon_w = 15.0f; icon_h = 15.0f;
                break;
            case MainPageState::PROCESS_TYPE_APP:
                if (proc.name.find("chrome") != std::string::npos) {
                    type_icon = &g_icon_chrome;
                    icon_w = 17.0f; icon_h = 15.0f;
                } else {
                    type_icon = &g_icon_cmd;
                    icon_w = 17.0f; icon_h = 14.0f;
                }
                break;
            default:
                type_icon = &g_icon_cmd;
                icon_w = 17.0f; icon_h = 14.0f;
        }
        
        if (type_icon && type_icon->valid) {
            float icon_x = floorf(pos.x + list_x + 16.0f);
            float icon_y = floorf(pos.y + current_y + (item_height - icon_h) / 2.0f);
            ImVec2 icon_pos = ImVec2(icon_x, icon_y);
            // 选中状态使用 #0058BC，未选中使用 #717786
            bool is_proc_selected = ((int)i == state.selected_process);
            ImU32 icon_tint = is_proc_selected ? IM_COL32(0, 88, 188, 255) : IM_COL32(113, 119, 134, 255);
            draw_list->AddImage(type_icon->srv, icon_pos, ImVec2(icon_pos.x + icon_w, icon_pos.y + icon_h), 
                ImVec2(0, 0), ImVec2(1, 1), icon_tint);
        }
        
        float name_x = pos.x + list_x + 48.0f;
        float name_y = pos.y + current_y + (item_height - 16.0f) / 2.0f;
        float process_text_y = name_y;  // 进程名实际Y坐标
        float process_text_height = 16.0f;  // 默认文本高度
        
        // 参考长度：VALORANT-Win64-Shipping.exe
        const char* ref_text = "VALORANT-Win64-Shipping.exe";
        
        // 根据选中状态使用不同字体，并处理超长进程名
        if ((int)i == state.selected_process) {
            // 选中项 - Inter Semibold 20
            if (font_process_selected) {
                float font_size = 20.0f;
                float max_width = font_process_selected->CalcTextSizeA(font_size, FLT_MAX, 0.0f, ref_text).x;
                
                // 如果进程名超过参考长度，添加省略号
                std::string display_name = proc.name;
                ImVec2 text_size = font_process_selected->CalcTextSizeA(font_size, FLT_MAX, 0.0f, display_name.c_str());
                if (text_size.x > max_width) {
                    // 逐步截断并添加...
                    display_name = proc.name;
                    while (display_name.length() > 3) {
                        display_name.pop_back();
                        std::string test_name = display_name + "...";
                        ImVec2 test_size = font_process_selected->CalcTextSizeA(font_size, FLT_MAX, 0.0f, test_name.c_str());
                        if (test_size.x <= max_width) {
                            display_name = test_name;
                            break;
                        }
                    }
                    if (display_name.length() <= 3) {
                        display_name = "...";
                    }
                    text_size = font_process_selected->CalcTextSizeA(font_size, FLT_MAX, 0.0f, display_name.c_str());
                }
                
                process_text_y = pos.y + current_y + (item_height - text_size.y) / 2.0f;
                process_text_height = text_size.y;
                draw_list->AddText(font_process_selected, font_size, ImVec2(name_x, process_text_y), MP_COLOR_ON_SURFACE, display_name.c_str());
            } else {
                draw_list->AddText(ImVec2(name_x, name_y), MP_COLOR_ON_SURFACE, proc.name.c_str());
            }
        } else {
            // 未选中项 - Inter Medium 14
            if (font_process_unselected) {
                float font_size = 14.0f;
                float max_width = font_process_unselected->CalcTextSizeA(font_size, FLT_MAX, 0.0f, ref_text).x;
                
                // 如果进程名超过参考长度，添加省略号
                std::string display_name = proc.name;
                ImVec2 text_size = font_process_unselected->CalcTextSizeA(font_size, FLT_MAX, 0.0f, display_name.c_str());
                if (text_size.x > max_width) {
                    // 逐步截断并添加...
                    display_name = proc.name;
                    while (display_name.length() > 3) {
                        display_name.pop_back();
                        std::string test_name = display_name + "...";
                        ImVec2 test_size = font_process_unselected->CalcTextSizeA(font_size, FLT_MAX, 0.0f, test_name.c_str());
                        if (test_size.x <= max_width) {
                            display_name = test_name;
                            break;
                        }
                    }
                    if (display_name.length() <= 3) {
                        display_name = "...";
                    }
                    text_size = font_process_unselected->CalcTextSizeA(font_size, FLT_MAX, 0.0f, display_name.c_str());
                }
                
                process_text_y = pos.y + current_y + (item_height - text_size.y) / 2.0f;
                process_text_height = text_size.y;
                draw_list->AddText(font_process_unselected, font_size, ImVec2(name_x, process_text_y), MP_COLOR_ON_SURFACE_VARIANT, display_name.c_str());
            } else {
                draw_list->AddText(ImVec2(name_x, name_y), MP_COLOR_ON_SURFACE_VARIANT, proc.name.c_str());
            }
        }
        
        // PID 居中对齐进程名
        float pid_x = pos.x + list_x + item_width - 80.0f;
        ImU32 pid_color = ((int)i == state.selected_process) ? MP_COLOR_PRIMARY : MP_COLOR_OUTLINE;
        std::string pid_text = "PID: " + proc.pid;
        
        if (font_small) {
            ImVec2 pid_size = font_small->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, pid_text.c_str());
            float pid_y = process_text_y + (process_text_height - pid_size.y) / 2.0f;
            draw_list->AddText(font_small, 10.0f, ImVec2(pid_x, pid_y), pid_color, pid_text.c_str());
        } else {
            float pid_y = process_text_y + (process_text_height - 10.0f) / 2.0f;
            draw_list->AddText(ImVec2(pid_x, pid_y), pid_color, pid_text.c_str());
        }
        
        current_y += item_height + 4.0f;
    }
}

// 渲染主内容区域
void RenderMainContentArea(MainPageState& state, float width, float height,
                           ImFont* font_title, ImFont* font_body, 
                           ImFont* font_small, ImFont* font_mono,
                           ImFont* font_inject_title, ImFont* font_dll_name,
                           ImFont* font_browse, ImFont* font_file_info,
                           ImFont* font_button_inject, ImFont* font_button_other) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetWindowPos();
    
    float content_x = pos.x + 40.0f;
    float current_y = pos.y + 30.0f;
    
    if (state.selected_process >= 0 && state.selected_process < (int)state.processes.size()) {
        const auto& proc = state.processes[state.selected_process];
        
        // 状态指示器 - Inter Semibold 10
        float dot_size = 8.0f;
        // System Ready 蓝点 - X对齐进程名，Y轴向下靠近进程名标题
        float dot_y = current_y + 12.0f;
        draw_list->AddCircleFilled(
            ImVec2(content_x + 4.0f, dot_y),
            dot_size / 2.0f,
            MP_COLOR_PRIMARY,
            8
        );
        const char* status_text = "SYSTEM READY";
        if (font_small) {
            draw_list->AddText(font_small, 10.0f, ImVec2(content_x + 16.0f, current_y + 8.0f), MP_COLOR_PRIMARY, status_text);
        } else {
            draw_list->AddText(ImVec2(content_x + 16.0f, current_y + 8.0f), MP_COLOR_PRIMARY, status_text);
        }
        current_y += 17.0f;  // 减少15px，让进程主标题往上移动
        
        // 进程名称 - 限制在 399 x 106 区域内，优先排满第一行再换行
        // 使用 Manrope ExtraBold 48
        std::string proc_name = proc.name;
        float title_max_width = 399.0f;
        float title_max_height = 106.0f;
        float title_font_size = 48.0f;
        // 默认行高（当font_title为null时使用）
        float line_height = title_font_size * 0.85f;
        
        if (font_title) {
            // 使用字体的实际行高，而不是固定值
            ImVec2 sample_char_size = font_title->CalcTextSizeA(title_font_size, FLT_MAX, 0.0f, "A");
            line_height = sample_char_size.y * 0.85f;  // 更紧凑的行间距
            // 计算完整文本尺寸
            ImVec2 full_text_size = font_title->CalcTextSizeA(title_font_size, FLT_MAX, 0.0f, proc_name.c_str());
            
            // 如果文本宽度超过399，尝试换行
            if (full_text_size.x > title_max_width) {
                // 策略：优先排满第一行（尽量接近399），然后在分隔符处换行
                std::string first_line, second_line;
                float current_width = 0.0f;
                int split_pos = -1;
                int last_separator_pos = -1;
                
                // 第一遍：找到最佳分割点（尽量排满第一行）
                for (size_t i = 0; i < proc_name.length(); i++) {
                    char c[2] = {proc_name[i], '\0'};
                    ImVec2 char_size = font_title->CalcTextSizeA(title_font_size, FLT_MAX, 0.0f, c);
                    current_width += char_size.x;
                    
                    // 记录最后一个分隔符位置
                    if (proc_name[i] == '-' || proc_name[i] == '_' || proc_name[i] == '.' || proc_name[i] == ' ') {
                        last_separator_pos = (int)i;
                    }
                    
                    // 当超过399时，使用最后一个分隔符作为分割点
                    if (current_width > title_max_width && split_pos == -1) {
                        if (last_separator_pos > 0) {
                            split_pos = last_separator_pos;
                        } else {
                            // 如果没有找到分隔符，就在当前位置硬分割
                            split_pos = (int)i;
                        }
                        break;
                    }
                }
                
                // 如果找到分割点
                if (split_pos > 0 && split_pos < (int)proc_name.length()) {
                    // 第一行包含分隔符（+1表示包含分隔符本身）
                    first_line = proc_name.substr(0, split_pos + 1);
                    // 第二行从分隔符后开始
                    size_t second_start = split_pos + 1;
                    second_line = proc_name.substr(second_start);
                    
                    // 计算两行的高度
                    float total_height = line_height * 2.0f;
                    
                    // 检查两行是否都能容纳在399宽度内
                    ImVec2 first_size = font_title->CalcTextSizeA(title_font_size, FLT_MAX, 0.0f, first_line.c_str());
                    ImVec2 second_size = font_title->CalcTextSizeA(title_font_size, FLT_MAX, 0.0f, second_line.c_str());
                    
                    // 如果两行高度超过106，需要处理
                    if (total_height > title_max_height) {
                        // 高度不够，显示两行但第二行末尾省略
                        // 如果第二行超过宽度，截断并添加省略号
                        if (second_size.x > title_max_width) {
                            std::string truncated = second_line;
                            while (!truncated.empty()) {
                                std::string test = truncated + "...";
                                ImVec2 test_size = font_title->CalcTextSizeA(title_font_size, FLT_MAX, 0.0f, test.c_str());
                                if (test_size.x <= title_max_width) {
                                    second_line = test;
                                    break;
                                }
                                truncated.pop_back();
                            }
                        }
                        
                        // 绘制两行
                        draw_list->AddText(font_title, title_font_size, ImVec2(content_x, current_y), MP_COLOR_ON_SURFACE, first_line.c_str());
                        current_y += line_height;
                        draw_list->AddText(font_title, title_font_size, ImVec2(content_x, current_y), MP_COLOR_ON_SURFACE, second_line.c_str());
                        current_y += 0.0f;  // 无额外间距
                    } else {
                        // 高度够，显示两行
                        // 如果第二行超过宽度，截断并添加省略号
                        if (second_size.x > title_max_width) {
                            std::string truncated = second_line;
                            while (!truncated.empty()) {
                                std::string test = truncated + "...";
                                ImVec2 test_size = font_title->CalcTextSizeA(title_font_size, FLT_MAX, 0.0f, test.c_str());
                                if (test_size.x <= title_max_width) {
                                    second_line = test;
                                    break;
                                }
                                truncated.pop_back();
                            }
                        }
                        
                        // 绘制两行
                        draw_list->AddText(font_title, title_font_size, ImVec2(content_x, current_y), MP_COLOR_ON_SURFACE, first_line.c_str());
                        current_y += line_height;
                        draw_list->AddText(font_title, title_font_size, ImVec2(content_x, current_y), MP_COLOR_ON_SURFACE, second_line.c_str());
                        current_y += 0.0f;  // 无额外间距
                    }
                } else {
                    // 无法分割，直接截断并添加省略号
                    std::string truncated = proc_name;
                    while (!truncated.empty()) {
                        std::string test = truncated + "...";
                        ImVec2 test_size = font_title->CalcTextSizeA(title_font_size, FLT_MAX, 0.0f, test.c_str());
                        if (test_size.x <= title_max_width) {
                            truncated = test;
                            break;
                        }
                        truncated.pop_back();
                    }
                    draw_list->AddText(font_title, title_font_size, ImVec2(content_x, current_y), MP_COLOR_ON_SURFACE, truncated.c_str());
                    current_y += line_height;
                }
            } else {
                // 文本在宽度范围内，直接显示
                draw_list->AddText(font_title, title_font_size, ImVec2(content_x, current_y), MP_COLOR_ON_SURFACE, proc_name.c_str());
                current_y += line_height;
            }
        } else {
            draw_list->AddText(ImVec2(content_x, current_y), MP_COLOR_ON_SURFACE, proc_name.c_str());
            current_y += line_height;
        }
        
        // 进程信息行 - Inter Medium 16，带图标
        current_y += 64.0f;  // 进程名和PID行之间的间距，总共往下偏移60px (原来4px + 60px = 64px)
        float info_y = current_y;
        float icon_y_offset = 2.0f;  // 图标垂直偏移微调
        float text_x = content_x;
        
        // PID 图标 (11x12)
        if (g_icon_process.valid) {
            float pid_icon_x = floorf(text_x);
            float pid_icon_y = floorf(info_y + icon_y_offset);
            draw_list->AddImage(g_icon_process.srv, 
                ImVec2(pid_icon_x, pid_icon_y), 
                ImVec2(pid_icon_x + 11.0f, pid_icon_y + 12.0f));
            text_x += 16.0f;
        }
        
        // PID 文本
        char pid_text[64];
        snprintf(pid_text, sizeof(pid_text), "PID %s", proc.pid.c_str());
        if (font_body) {
            draw_list->AddText(font_body, 16.0f, ImVec2(text_x, info_y), MP_COLOR_ON_SURFACE_VARIANT, pid_text);
            text_x += font_body->CalcTextSizeA(16.0f, FLT_MAX, 0.0f, pid_text).x + 24.0f;
        } else {
            draw_list->AddText(ImVec2(text_x, info_y), MP_COLOR_ON_SURFACE_VARIANT, pid_text);
            text_x += 80.0f;
        }
        
        // 保护图标 (10x12) - x64 Native前
        if (g_icon_protect.valid) {
            float protect_icon_x = floorf(text_x);
            float protect_icon_y = floorf(info_y + icon_y_offset);
            draw_list->AddImage(g_icon_protect.srv, 
                ImVec2(protect_icon_x, protect_icon_y), 
                ImVec2(protect_icon_x + 10.0f, protect_icon_y + 12.0f));
            text_x += 15.0f;
        }
        
        // x64 Native 文本
        const char* arch_text = "x64 Native";
        if (font_body) {
            draw_list->AddText(font_body, 16.0f, ImVec2(text_x, info_y), MP_COLOR_ON_SURFACE_VARIANT, arch_text);
            text_x += font_body->CalcTextSizeA(16.0f, FLT_MAX, 0.0f, arch_text).x + 24.0f;
        } else {
            draw_list->AddText(ImVec2(text_x, info_y), MP_COLOR_ON_SURFACE_VARIANT, arch_text);
            text_x += 90.0f;
        }
        
        // 路径图标 (10x12)
        if (g_icon_path.valid) {
            draw_list->AddImage(g_icon_path.srv, 
                ImVec2(text_x, info_y + icon_y_offset), 
                ImVec2(text_x + 10.0f, info_y + icon_y_offset + 12.0f));
            text_x += 15.0f;
        }
        
        // 路径文本
        const char* path_text = "C:\\Games\\Riot\\VALORANT...";
        if (font_body) {
            draw_list->AddText(font_body, 16.0f, ImVec2(text_x, info_y), MP_COLOR_ON_SURFACE_VARIANT, path_text);
        } else {
            draw_list->AddText(ImVec2(text_x, info_y), MP_COLOR_ON_SURFACE_VARIANT, path_text);
        }
        current_y += 58.0f;  // 增加30px，让注入按钮等再往上移动20px (原来78px - 20px = 58px)
        
        // ===== 左侧：白色矩形底色 =====
        float left_box_width = 370.0f;
        float left_box_height = 148.0f;
        float left_box_y = current_y;
        
        ImRect white_box_rect(ImVec2(content_x, left_box_y), 
                              ImVec2(content_x + left_box_width, left_box_y + left_box_height));
        draw_list->AddRectFilled(white_box_rect.Min, white_box_rect.Max, MP_COLOR_SURFACE, 12.0f);
        
        // INJECTION TARGET - Inter Semibold 11
        const char* injection_title = "INJECTION TARGET";
        float title_y = left_box_y + 20.0f;
        if (font_inject_title) {
            draw_list->AddText(font_inject_title, 11.0f, ImVec2(content_x + 24.0f, title_y), MP_COLOR_ON_SURFACE_VARIANT, injection_title);
        }
        
        float dll_box_y = left_box_y + 45.0f;
        float dll_box_height = 80.0f;
        ImRect dll_box_rect(ImVec2(content_x + 24.0f, dll_box_y), 
                            ImVec2(content_x + left_box_width - 24.0f, dll_box_y + dll_box_height));
        draw_list->AddRectFilled(dll_box_rect.Min, dll_box_rect.Max, MP_COLOR_SURFACE_HIGH, 8.0f);
        
        // DLL文件图标 (14x17)
        if (g_icon_file.valid) {
            draw_list->AddImage(g_icon_file.srv, 
                ImVec2(content_x + 44.0f, dll_box_y + (dll_box_height - 17.0f) / 2.0f),
                ImVec2(content_x + 44.0f + 14.0f, dll_box_y + (dll_box_height - 17.0f) / 2.0f + 17.0f));
        }
        
        // DLL名 - Inter Semibold 14
        const char* dll_name = "internal_bypass_v2.dll";
        if (font_dll_name) {
            draw_list->AddText(font_dll_name, 14.0f, ImVec2(content_x + 74.0f, dll_box_y + 20.0f), MP_COLOR_ON_SURFACE, dll_name);
        } else {
            draw_list->AddText(ImVec2(content_x + 74.0f, dll_box_y + 20.0f), MP_COLOR_ON_SURFACE, dll_name);
        }
        
        // Size等文件信息 - Inter Regular 11
        const char* dll_info = "Size: 4.2 MB | MD5: 4f1a...9e2";
        if (font_file_info) {
            draw_list->AddText(font_file_info, 11.0f, ImVec2(content_x + 74.0f, dll_box_y + 42.0f), MP_COLOR_ON_SURFACE_VARIANT, dll_info);
        } else {
            draw_list->AddText(ImVec2(content_x + 74.0f, dll_box_y + 42.0f), MP_COLOR_ON_SURFACE_VARIANT, dll_info);
        }
        
        // BROWSE - Inter Semibold 12
        float browse_x = content_x + left_box_width - 100.0f;
        float browse_y = dll_box_y + 28.0f;
        ImRect browse_rect(ImVec2(browse_x, browse_y), ImVec2(browse_x + 60.0f, browse_y + 24.0f));
        bool browse_hovered = ImGui::IsMouseHoveringRect(browse_rect.Min, browse_rect.Max);
        if (font_browse) {
            draw_list->AddText(font_browse, 12.0f, ImVec2(browse_x, browse_y), 
                browse_hovered ? MP_COLOR_PRIMARY : MP_COLOR_PRIMARY_CONTAINER, "BROWSE");
        } else {
            draw_list->AddText(ImVec2(browse_x, browse_y), 
                browse_hovered ? MP_COLOR_PRIMARY : MP_COLOR_PRIMARY_CONTAINER, "BROWSE");
        }
        
        // ===== 右侧：操作按钮 =====
        float buttons_x = content_x + left_box_width + 30.0f;
        float button_y = left_box_y;
        
        // INJECT DLL 按钮 - Manrope Bold 18
        float inject_width = 370.0f;
        float inject_height = 68.0f;
        ImRect inject_rect(ImVec2(buttons_x, button_y), ImVec2(buttons_x + inject_width, button_y + inject_height));
        bool inject_hovered = ImGui::IsMouseHoveringRect(inject_rect.Min, inject_rect.Max);
        bool inject_clicked = inject_hovered && ImGui::IsMouseClicked(0);
        
        ImU32 inject_color = inject_hovered ? MP_COLOR_PRIMARY_CONTAINER : MP_COLOR_PRIMARY;
        draw_list->AddRectFilled(inject_rect.Min, inject_rect.Max, inject_color, 12.0f);
        
        // 注入按钮文本和图标
        const char* inject_text = "INJECT DLL";
        ImVec2 inject_text_size = font_button_inject ? font_button_inject->CalcTextSizeA(18.0f, FLT_MAX, 0.0f, inject_text) : ImGui::CalcTextSize(inject_text);
        
        // 计算总宽度（图标 + 间距 + 文本）
        float icon_total_width = 0.0f;
        if (g_icon_inject.valid) {
            icon_total_width = 14.0f + 8.0f;  // 图标宽度 + 间距
        }
        float total_width = icon_total_width + inject_text_size.x;
        float start_x = buttons_x + (inject_width - total_width) / 2.0f;
        float inject_text_y = button_y + (inject_height - inject_text_size.y) / 2.0f;
        
        // 绘制注入图标 (14x17)
        if (g_icon_inject.valid) {
            float icon_y = button_y + (inject_height - 17.0f) / 2.0f;
            draw_list->AddImage(g_icon_inject.srv,
                ImVec2(start_x, icon_y),
                ImVec2(start_x + 14.0f, icon_y + 17.0f));
            start_x += icon_total_width;
        }
        
        // 绘制文本
        if (font_button_inject) {
            draw_list->AddText(font_button_inject, 18.0f, ImVec2(start_x, inject_text_y), IM_COL32(255, 255, 255, 255), inject_text);
        } else {
            draw_list->AddText(ImVec2(start_x, inject_text_y), IM_COL32(255, 255, 255, 255), inject_text);
        }
        
        if (inject_clicked) {
            printf("INJECT DLL clicked for process: %s\n", proc.name.c_str());
        }
        
        // DUMP 和 VIEW MAP 按钮 - Manrope Bold 14
        float small_button_width = 172.0f;
        float small_button_height = 52.0f;
        float small_button_y = button_y + inject_height + 16.0f;
        float small_button_spacing = 26.0f;
        
        // DUMP按钮 - 带Icon_dump.png图标 (10x10)
        ImRect dump_rect(ImVec2(buttons_x, small_button_y), ImVec2(buttons_x + small_button_width, small_button_y + small_button_height));
        bool dump_hovered = ImGui::IsMouseHoveringRect(dump_rect.Min, dump_rect.Max);
        draw_list->AddRectFilled(dump_rect.Min, dump_rect.Max, MP_COLOR_SURFACE_HIGHEST, 12.0f);
        const char* dump_text = "DUMP";
        ImVec2 dump_text_size = font_button_other ? font_button_other->CalcTextSizeA(14.0f, FLT_MAX, 0.0f, dump_text) : ImGui::CalcTextSize(dump_text);
        
        // 计算DUMP按钮的总宽度（图标 + 间距 + 文本）
        float dump_icon_width = g_icon_dump.valid ? 10.0f + 6.0f : 0.0f;
        float dump_total_width = dump_icon_width + dump_text_size.x;
        float dump_start_x = buttons_x + (small_button_width - dump_total_width) / 2.0f;
        float dump_text_y = small_button_y + (small_button_height - dump_text_size.y) / 2.0f;
        
        // 绘制DUMP图标
        if (g_icon_dump.valid) {
            float icon_y = small_button_y + (small_button_height - 10.0f) / 2.0f;
            draw_list->AddImage(g_icon_dump.srv,
                ImVec2(dump_start_x, icon_y),
                ImVec2(dump_start_x + 10.0f, icon_y + 10.0f));
            dump_start_x += dump_icon_width;
        }
        
        if (font_button_other) {
            draw_list->AddText(font_button_other, 14.0f, ImVec2(dump_start_x, dump_text_y), MP_COLOR_ON_SURFACE, dump_text);
        }
        
        // DUMP按钮点击 - 打开模态对话框
        if (dump_hovered && ImGui::IsMouseClicked(0)) {
            state.modal_dialog_state.Open();
        }
        
        // VIEW MAP按钮 - 带Icon_viewdump.png图标 (13x9)
        float view_x = buttons_x + small_button_width + small_button_spacing;
        ImRect view_rect(ImVec2(view_x, small_button_y), ImVec2(view_x + small_button_width, small_button_y + small_button_height));
        bool view_hovered = ImGui::IsMouseHoveringRect(view_rect.Min, view_rect.Max);
        draw_list->AddRectFilled(view_rect.Min, view_rect.Max, MP_COLOR_SURFACE_HIGHEST, 12.0f);
        const char* view_text = "VIEW MAP";
        ImVec2 view_text_size = font_button_other ? font_button_other->CalcTextSizeA(14.0f, FLT_MAX, 0.0f, view_text) : ImGui::CalcTextSize(view_text);
        
        // 计算VIEW MAP按钮的总宽度（图标 + 间距 + 文本）
        float view_icon_width = g_icon_viewdump.valid ? 13.0f + 6.0f : 0.0f;
        float view_total_width = view_icon_width + view_text_size.x;
        float view_start_x = view_x + (small_button_width - view_total_width) / 2.0f;
        float view_text_y = small_button_y + (small_button_height - view_text_size.y) / 2.0f;
        
        // 绘制VIEW MAP图标
        if (g_icon_viewdump.valid) {
            float icon_y = small_button_y + (small_button_height - 9.0f) / 2.0f;
            draw_list->AddImage(g_icon_viewdump.srv,
                ImVec2(view_start_x, icon_y),
                ImVec2(view_start_x + 13.0f, icon_y + 9.0f));
            view_start_x += view_icon_width;
        }
        
        if (font_button_other) {
            draw_list->AddText(font_button_other, 14.0f, ImVec2(view_start_x, view_text_y), MP_COLOR_ON_SURFACE, view_text);
        }
        
        // ===== Buffer Integrity - 100% VERIFIED靠右对齐 - Inter Semibold 10 =====
        float integrity_y = left_box_y + left_box_height + 30.0f;
        const char* integrity_label = "BUFFER INTEGRITY";
        const char* integrity_value = "100% VERIFIED";
        
        float integrity_label_x = content_x;
        
        if (font_small) {
            // 计算100% VERIFIED的宽度，靠右对齐到进度条右端
            ImVec2 value_size = font_small->CalcTextSizeA(10.0f, FLT_MAX, 0.0f, integrity_value);
            float integrity_value_x = content_x + 370.0f - value_size.x;
            
            draw_list->AddText(font_small, 10.0f, ImVec2(integrity_label_x, integrity_y), MP_COLOR_ON_SURFACE_VARIANT, integrity_label);
            draw_list->AddText(font_small, 10.0f, ImVec2(integrity_value_x, integrity_y), MP_COLOR_ON_SURFACE_VARIANT, integrity_value);
        }
        
        // 滑块条
        float progress_y = integrity_y + 18.0f;
        float progress_width = 370.0f;
        float progress_height = 4.0f;
        
        draw_list->AddRectFilled(
            ImVec2(content_x, progress_y),
            ImVec2(content_x + progress_width, progress_y + progress_height),
            MP_COLOR_SURFACE_HIGHEST,
            2.0f
        );
        draw_list->AddRectFilled(
            ImVec2(content_x, progress_y),
            ImVec2(content_x + progress_width, progress_y + progress_height),
            MP_COLOR_PRIMARY,
            2.0f
        );
        
        // ===== Live Audit Log =====
        float log_height = 140.0f;
        float log_margin_bottom = 20.0f;
        float log_y = pos.y + height - log_height - log_margin_bottom;
        
        float min_log_y = progress_y + 40.0f;
        if (log_y < min_log_y) {
            log_y = min_log_y;
        }
        
        float log_width = width - 80.0f;
        ImRect log_rect(ImVec2(content_x, log_y), ImVec2(content_x + log_width, log_y + log_height));
        draw_list->AddRectFilled(log_rect.Min, log_rect.Max, MP_COLOR_SURFACE_LOW, 12.0f);
        
        const char* log_title = "LIVE AUDIT LOG";
        if (font_small) {
            draw_list->AddText(font_small, 11.0f, ImVec2(content_x + 16.0f, log_y + 10.0f), MP_COLOR_ON_SURFACE_VARIANT, log_title);
        }
        
        float log_entry_y = log_y + 30.0f;
        float log_line_height = 18.0f;
        int max_visible_logs = (int)((log_y + log_height - 20.0f - log_entry_y) / log_line_height);
        
        // 计算要从哪条日志开始显示（显示最新的日志）
        int start_idx = 0;
        if ((int)state.logs.size() > max_visible_logs) {
            start_idx = (int)state.logs.size() - max_visible_logs;
        }
        
        // 渲染日志条目
        for (int i = start_idx; i < (int)state.logs.size() && log_entry_y < log_y + log_height - 20.0f; i++) {
            const auto& log = state.logs[i];
            
            // 组合完整日志文本
            std::string full_log = log.timestamp + "  " + log.message;
            
            if (font_mono) {
                draw_list->AddText(font_mono, 11.0f, ImVec2(content_x + 16.0f, log_entry_y), 
                    MP_COLOR_ON_SURFACE_VARIANT, full_log.c_str());
            } else {
                draw_list->AddText(ImVec2(content_x + 16.0f, log_entry_y), MP_COLOR_ON_SURFACE_VARIANT, full_log.c_str());
            }
            
            // 渲染状态标签
            if (!log.status.empty()) {
                ImU32 status_color = (log.status == "SUCCESS") ? MP_COLOR_PRIMARY : 
                                     (log.status == "ERROR") ? IM_COL32(255, 0, 0, 255) : MP_COLOR_OUTLINE;
                if (font_small) {
                    draw_list->AddText(font_small, 10.0f, ImVec2(content_x + log_width - 80.0f, log_entry_y), 
                        status_color, log.status.c_str());
                }
            }
            
            log_entry_y += log_line_height;
        }
    }
}

// 为了保持兼容性，保留旧函数名但调用新实现
void RenderSidebar(MainPageState& state, float width, float height,
                   ImFont* font_title, ImFont* font_body, ImFont* font_small) {
    RenderSidebarContent(state, width, height, font_title, font_body, font_small, nullptr, nullptr);
}

void RenderMainContent(MainPageState& state, float width, float height,
                       ImFont* font_title, ImFont* font_body, 
                       ImFont* font_small, ImFont* font_mono) {
    RenderMainContentArea(state, width, height, font_title, font_body, font_small, font_mono,
                          nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
}
