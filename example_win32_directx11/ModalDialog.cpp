#include "ModalDialog.h"
#include <imgui_internal.h>
#include <algorithm>
#include <cstring>

// 颜色定义 (从 Modal Dialog.html 提取)
#define MD_COLOR_BACKGROUND IM_COL32(248, 249, 250, 255)      // #f8f9fa
#define MD_COLOR_SURFACE IM_COL32(255, 255, 255, 255)         // #ffffff
#define MD_COLOR_SURFACE_LOW IM_COL32(243, 244, 245, 255)     // #f3f4f5
#define MD_COLOR_SURFACE_HIGH IM_COL32(231, 232, 233, 255)    // #e7e8e9
#define MD_COLOR_SURFACE_HIGHEST IM_COL32(225, 227, 228, 255) // #e1e3e4
#define MD_COLOR_PRIMARY IM_COL32(0, 88, 188, 255)            // #0058bc
#define MD_COLOR_PRIMARY_CONTAINER IM_COL32(0, 112, 235, 255) // #0070eb
#define MD_COLOR_ON_SURFACE IM_COL32(25, 28, 29, 255)         // #191c1d
#define MD_COLOR_ON_SURFACE_VARIANT IM_COL32(65, 71, 85, 255) // #414755
#define MD_COLOR_OUTLINE IM_COL32(113, 119, 134, 255)         // #717786
#define MD_COLOR_OUTLINE_VARIANT IM_COL32(225, 227, 228, 255) // #e1e3e4

// 缓动函数
static float EaseOutCubic(float t) {
    return 1.0f - powf(1.0f - t, 3.0f);
}

ModalDialogState::ModalDialogState() {
    // 初始化示例模块数据
    modules = {
        {"ntdll.dll", "1.94 MB", "0x7FFE2110", true},
        {"kernel32.dll", "724 KB", "0x7FFE0940", true},
        {"user32.dll", "1.52 MB", "0x7FFDB400", false},
        {"gdi32.dll", "158 KB", "0x7FFE1420", false},
        {"advapi32.dll", "492 KB", "0x7FFDD290", false},
        {"shell32.dll", "2.31 MB", "0x7FFDD6A0", false},
        {"msvcrt.dll", "632 KB", "0x7FFDF8C0", false},
        {"ole32.dll", "1.18 MB", "0x7FFDF1E0", false},
        {"rpcrt4.dll", "856 KB", "0x7FFDE5B0", false},
        {"combase.dll", "1.45 MB", "0x7FFDE890", false}
    };
}

void ModalDialogState::Open() {
    is_open = true;
    target_animation = 1.0f;
}

void ModalDialogState::Close() {
    target_animation = 0.0f;
}

void ModalDialogState::UpdateAnimation(float delta_time) {
    float speed = 8.0f * delta_time;
    if (open_animation < target_animation) {
        open_animation += speed;
        if (open_animation > target_animation) {
            open_animation = target_animation;
        }
    } else if (open_animation > target_animation) {
        open_animation -= speed;
        if (open_animation < target_animation) {
            open_animation = target_animation;
        }
    }
    
    if (open_animation <= 0.0f && target_animation <= 0.0f) {
        is_open = false;
    }
}

std::vector<size_t> ModalDialogState::GetFilteredIndices() const {
    std::vector<size_t> result;
    std::string search_lower = search_buffer;
    std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), ::tolower);
    
    for (size_t i = 0; i < modules.size(); i++) {
        if (search_lower.empty()) {
            result.push_back(i);
        } else {
            std::string name_lower = modules[i].name;
            std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), ::tolower);
            std::string addr_lower = modules[i].baseAddress;
            std::transform(addr_lower.begin(), addr_lower.end(), addr_lower.begin(), ::tolower);
            
            if (name_lower.find(search_lower) != std::string::npos ||
                addr_lower.find(search_lower) != std::string::npos) {
                result.push_back(i);
            }
        }
    }
    return result;
}

int ModalDialogState::GetSelectedCount() const {
    int count = 0;
    for (const auto& mod : modules) {
        if (mod.selected) count++;
    }
    return count;
}

void ModalDialogState::ToggleSelectAll() {
    select_all = !select_all;
    auto filtered = GetFilteredIndices();
    for (size_t idx : filtered) {
        modules[idx].selected = select_all;
    }
}

bool RenderModalDialog(ModalDialogState& state, ImFont* font_title, ImFont* font_body, ImFont* font_small) {
    bool result = false;
    
    if (!state.is_open && state.open_animation <= 0.0f) {
        return result;
    }
    
    // 动画值
    float anim = EaseOutCubic(state.open_animation);
    
    // 获取主窗口位置和大小
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();
    
    // 对话框尺寸 672 x 631
    float dialog_width = 672.0f;
    float dialog_height = 631.0f;
    float dialog_x = window_pos.x + (window_size.x - dialog_width) / 2.0f;
    float dialog_y = window_pos.y + (window_size.y - dialog_height) / 2.0f + (1.0f - anim) * 30.0f;
    
    // 创建模态遮罩窗口 - 阻止鼠标事件传递到下层
    ImGui::SetNextWindowPos(window_pos);
    ImGui::SetNextWindowSize(window_size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(25, 28, 29, (int)(76 * anim)));
    
    ImGui::Begin("##modal_overlay", nullptr, 
        ImGuiWindowFlags_NoTitleBar | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoScrollbar | 
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoBringToFrontOnFocus);
    
    // 点击遮罩关闭
    if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImRect dialog_rect(ImVec2(dialog_x, dialog_y), ImVec2(dialog_x + dialog_width, dialog_y + dialog_height));
        if (!dialog_rect.Contains(mouse_pos)) {
            state.Close();
        }
    }
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    // 对话框背景 (白色圆角)
    float rounding = 12.0f;
    ImRect dialog_rect(ImVec2(dialog_x, dialog_y), ImVec2(dialog_x + dialog_width, dialog_y + dialog_height));
    draw_list->AddRectFilled(dialog_rect.Min, dialog_rect.Max, MD_COLOR_SURFACE, rounding);
    
    // 对话框阴影
    ImU32 shadow_color = IM_COL32(0, 0, 0, (int)(20 * anim));
    draw_list->AddRect(dialog_rect.Min, dialog_rect.Max, shadow_color, rounding, 0, 2.0f);
    
    float content_x = dialog_x + 32.0f;
    float current_y = dialog_y + 32.0f;
    float content_width = dialog_width - 64.0f;
    
    // 标题
    const char* title = "Select Modules to Dump";
    if (font_title) {
        draw_list->AddText(font_title, 24.0f, ImVec2(content_x, current_y), MD_COLOR_ON_SURFACE, title);
    } else {
        draw_list->AddText(ImVec2(content_x, current_y), MD_COLOR_ON_SURFACE, title);
    }
    current_y += 36.0f;
    
    // 副标题
    const char* subtitle = "Choose internal components to export from process memory.";
    if (font_body) {
        draw_list->AddText(font_body, 14.0f, ImVec2(content_x, current_y), MD_COLOR_ON_SURFACE_VARIANT, subtitle);
    } else {
        draw_list->AddText(ImVec2(content_x, current_y), MD_COLOR_ON_SURFACE_VARIANT, subtitle);
    }
    current_y += 32.0f;
    
    // 搜索栏背景
    float search_height = 44.0f;
    ImRect search_rect(ImVec2(content_x, current_y), ImVec2(content_x + content_width, current_y + search_height));
    draw_list->AddRectFilled(search_rect.Min, search_rect.Max, MD_COLOR_SURFACE_LOW, 12.0f);
    
    // 搜索输入框
    ImGui::SetCursorScreenPos(ImVec2(content_x + 40.0f, current_y + 10.0f));
    ImGui::PushItemWidth(content_width - 60.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Text, MD_COLOR_ON_SURFACE);
    ImGui::InputTextWithHint("##module_search", "Filter modules by name or address...", state.search_buffer, sizeof(state.search_buffer));
    ImGui::PopStyleColor(2);
    ImGui::PopItemWidth();
    
    current_y += search_height + 24.0f;
    
    // 表头
    float checkbox_col_width = 40.0f;
    float name_col_width = 280.0f;
    float size_col_width = 100.0f;
    float address_col_width = 140.0f;
    
    float header_y = current_y;
    const char* header_name = "Module Name";
    const char* header_size = "Size";
    const char* header_addr = "Base Address";
    
    // 表头文字
    if (font_small) {
        draw_list->AddText(font_small, 11.0f, ImVec2(content_x + checkbox_col_width + 8.0f, header_y), MD_COLOR_ON_SURFACE_VARIANT, header_name);
        
        ImVec2 size_size = font_small->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, header_size);
        draw_list->AddText(font_small, 11.0f, ImVec2(content_x + checkbox_col_width + name_col_width + 60.0f - size_size.x, header_y), MD_COLOR_ON_SURFACE_VARIANT, header_size);
        
        ImVec2 addr_size = font_small->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, header_addr);
        draw_list->AddText(font_small, 11.0f, ImVec2(content_x + checkbox_col_width + name_col_width + size_col_width + 80.0f - addr_size.x, header_y), MD_COLOR_ON_SURFACE_VARIANT, header_addr);
    }
    
    // 表头分隔线
    current_y += 28.0f;
    draw_list->AddLine(ImVec2(content_x, current_y), ImVec2(content_x + content_width, current_y), MD_COLOR_OUTLINE_VARIANT);
    current_y += 8.0f;
    
    // 模块列表区域
    float list_height = 320.0f;
    float row_height = 48.0f;
    
    // 获取过滤后的模块索引
    auto filtered_indices = state.GetFilteredIndices();
    
    // 创建列表区域 - 使用Child窗口来裁剪内容
    ImGui::SetCursorScreenPos(ImVec2(content_x, current_y));
    ImGui::BeginChild("##module_list", ImVec2(content_width, list_height), false, ImGuiWindowFlags_NoScrollbar);
    
    ImVec2 list_pos = ImGui::GetWindowPos();
    ImDrawList* list_draw = ImGui::GetWindowDrawList();
    
    for (size_t filtered_i = 0; filtered_i < filtered_indices.size(); filtered_i++) {
        size_t i = filtered_indices[filtered_i];
        auto& mod = state.modules[i];
        float row_y = list_pos.y + filtered_i * row_height;
        
        // 行背景 (悬停效果)
        ImRect row_rect(ImVec2(list_pos.x, row_y), ImVec2(list_pos.x + content_width, row_y + row_height));
        bool row_hovered = ImGui::IsMouseHoveringRect(row_rect.Min, row_rect.Max);
        
        if (row_hovered) {
            list_draw->AddRectFilled(row_rect.Min, row_rect.Max, MD_COLOR_SURFACE_LOW, 8.0f);
        }
        
        // 复选框
        float checkbox_x = list_pos.x + 12.0f;
        float checkbox_y = row_y + (row_height - 16.0f) / 2.0f;
        ImRect checkbox_rect(ImVec2(checkbox_x, checkbox_y), ImVec2(checkbox_x + 16.0f, checkbox_y + 16.0f));
        
        // 绘制复选框
        list_draw->AddRectFilled(checkbox_rect.Min, checkbox_rect.Max, 
            mod.selected ? MD_COLOR_PRIMARY : MD_COLOR_SURFACE_HIGHEST, 4.0f);
        if (!mod.selected) {
            list_draw->AddRect(checkbox_rect.Min, checkbox_rect.Max, MD_COLOR_OUTLINE, 4.0f, 0, 1.0f);
        }
        
        // 选中标记
        if (mod.selected) {
            ImVec2 check_center = ImVec2(checkbox_x + 8.0f, checkbox_y + 8.0f);
            list_draw->AddLine(ImVec2(check_center.x - 3.0f, check_center.y), ImVec2(check_center.x - 1.0f, check_center.y + 2.0f), MD_COLOR_SURFACE, 2.0f);
            list_draw->AddLine(ImVec2(check_center.x - 1.0f, check_center.y + 2.0f), ImVec2(check_center.x + 3.0f, check_center.y - 2.0f), MD_COLOR_SURFACE, 2.0f);
        }
        
        // 点击复选框
        if (ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(checkbox_rect.Min, checkbox_rect.Max)) {
            mod.selected = !mod.selected;
        }
        
        // 模块图标
        float icon_x = list_pos.x + checkbox_col_width + 8.0f;
        float icon_y = row_y + (row_height - 20.0f) / 2.0f;
        list_draw->AddRectFilled(ImVec2(icon_x, icon_y), ImVec2(icon_x + 16.0f, icon_y + 20.0f), MD_COLOR_SURFACE_HIGHEST, 4.0f);
        list_draw->AddRect(ImVec2(icon_x, icon_y), ImVec2(icon_x + 16.0f, icon_y + 20.0f), MD_COLOR_OUTLINE, 4.0f, 0, 1.0f);
        
        // 模块名称
        float name_x = icon_x + 24.0f;
        float name_y = row_y + (row_height - 16.0f) / 2.0f;
        if (font_body) {
            list_draw->AddText(font_body, 14.0f, ImVec2(name_x, name_y), MD_COLOR_ON_SURFACE, mod.name.c_str());
        } else {
            list_draw->AddText(ImVec2(name_x, name_y), MD_COLOR_ON_SURFACE, mod.name.c_str());
        }
        
        // 大小
        if (font_small) {
            ImVec2 size_text_size = font_small->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, mod.size.c_str());
            float size_x = list_pos.x + checkbox_col_width + name_col_width + 60.0f - size_text_size.x;
            list_draw->AddText(font_small, 12.0f, ImVec2(size_x, name_y), MD_COLOR_ON_SURFACE_VARIANT, mod.size.c_str());
        }
        
        // 基地址
        if (font_small) {
            ImVec2 addr_text_size = font_small->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, mod.baseAddress.c_str());
            float addr_x = list_pos.x + checkbox_col_width + name_col_width + size_col_width + 80.0f - addr_text_size.x;
            list_draw->AddText(font_small, 12.0f, ImVec2(addr_x, name_y), MD_COLOR_ON_SURFACE_VARIANT, mod.baseAddress.c_str());
        }
    }
    
    ImGui::EndChild();
    
    current_y += list_height + 24.0f;
    
    // 底部按钮区域背景
    float footer_height = 80.0f;
    draw_list->AddRectFilled(ImVec2(dialog_x, dialog_y + dialog_height - footer_height), 
                             ImVec2(dialog_x + dialog_width, dialog_y + dialog_height), 
                             MD_COLOR_SURFACE_LOW, rounding);
    
    // 按钮
    float button_y = dialog_y + dialog_height - footer_height + 20.0f;
    float cancel_x = dialog_x + dialog_width - 220.0f;
    float dump_x = dialog_x + dialog_width - 160.0f;
    
    // CANCEL 按钮
    ImRect cancel_rect(ImVec2(cancel_x - 20.0f, button_y), ImVec2(cancel_x + 80.0f, button_y + 40.0f));
    bool cancel_hovered = ImGui::IsMouseHoveringRect(cancel_rect.Min, cancel_rect.Max);
    draw_list->AddRectFilled(cancel_rect.Min, cancel_rect.Max, 
        cancel_hovered ? MD_COLOR_SURFACE_HIGHEST : MD_COLOR_SURFACE_LOW, 8.0f);
    
    const char* cancel_text = "CANCEL";
    if (font_body) {
        ImVec2 text_size = font_body->CalcTextSizeA(14.0f, FLT_MAX, 0.0f, cancel_text);
        draw_list->AddText(font_body, 14.0f, 
            ImVec2(cancel_x + (80.0f - text_size.x) / 2.0f - 10.0f, button_y + (40.0f - text_size.y) / 2.0f), 
            MD_COLOR_ON_SURFACE, cancel_text);
    } else {
        draw_list->AddText(ImVec2(cancel_x + 15.0f, button_y + 12.0f), MD_COLOR_ON_SURFACE, cancel_text);
    }
    
    if (ImGui::IsMouseClicked(0) && cancel_hovered) {
        state.Close();
    }
    
    // DUMP SELECTED 按钮
    ImRect dump_rect(ImVec2(dump_x, button_y), ImVec2(dump_x + 140.0f, button_y + 40.0f));
    bool dump_hovered = ImGui::IsMouseHoveringRect(dump_rect.Min, dump_rect.Max);
    
    // 渐变背景
    ImU32 dump_color_start = MD_COLOR_PRIMARY;
    ImU32 dump_color_end = MD_COLOR_PRIMARY_CONTAINER;
    draw_list->AddRectFilledMultiColor(dump_rect.Min, dump_rect.Max, 
        dump_color_start, dump_color_end, dump_color_end, dump_color_start);
    
    // 按钮文字
    char dump_text[64];
    int selected_count = state.GetSelectedCount();
    if (selected_count > 0) {
        snprintf(dump_text, sizeof(dump_text), "DUMP SELECTED (%d)", selected_count);
    } else {
        snprintf(dump_text, sizeof(dump_text), "DUMP SELECTED");
    }
    
    if (font_body) {
        ImVec2 text_size = font_body->CalcTextSizeA(14.0f, FLT_MAX, 0.0f, dump_text);
        draw_list->AddText(font_body, 14.0f, 
            ImVec2(dump_x + (140.0f - text_size.x) / 2.0f, button_y + (40.0f - text_size.y) / 2.0f), 
            MD_COLOR_SURFACE, dump_text);
    } else {
        draw_list->AddText(ImVec2(dump_x + 20.0f, button_y + 12.0f), MD_COLOR_SURFACE, dump_text);
    }
    
    if (ImGui::IsMouseClicked(0) && dump_hovered && selected_count > 0) {
        result = true;
        state.Close();
    }
    
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    
    return result;
}
