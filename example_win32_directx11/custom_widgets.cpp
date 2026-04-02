#define IMGUI_DEFINE_MATH_OPERATORS

#include <pimgui.h>
using namespace ImGui;

bool PImGui::InputTextPopup(const char* str_id, char* buffer, PImGuiTextPopupFlags popup_flags) {
    bool is_open = ImGui::BeginPopup(str_id);
    if (!is_open) return false;

    //��������
    if (ImGui::IsWindowAppearing())
        ImGui::SetKeyboardFocusHere();

    ImGui::BeginTable("##Buttons", 2);
    if (ImGui::TableNextColumn()) {
        if (ImGui::Button("Copy")) {
            ImGui::SetClipboardText(buffer);
        }
        if (!(popup_flags & PImGuiTextPopupFlags_NoPaste)) {
            bool paste = ImGui::Button("Paste");
            const char* clipboard = ImGui::GetClipboardText();
            if (!(popup_flags & PImGuiTextPopupFlags_NoPaste)) {
                bool paste = ImGui::Button("Paste");
                const char* clipboard = ImGui::GetClipboardText();
                if (clipboard && paste) {
                    size_t clipboard_len = strlen(clipboard);
                    size_t buffer_size = sizeof(buffer);
                    strncpy(buffer, clipboard, buffer_size - 1);
                    buffer[buffer_size - 1] = '\0';

                }

            }
        }
        if (ImGui::TableNextColumn()) {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 255));
            if (ImGui::Button("Exit")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleColor();
        }
        ImGui::EndTable();

        ImGui::EndPopup();
        return true;
    }
};

#include <algorithm>

bool PImGui::FunctionPlot(const char* label,
    float (*func)(float x),
    float x_min,
    float x_max,
    const ImVec2& size,
    const FunctionPlotConfig* config,
    PImGuiFunctinPlotFlags flags)
{
    // ��ȡ��ǰ����������
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems || !func)
        return false;

    // ��ʼ�����ò���
    const FunctionPlotConfig cfg = config ? *config : FunctionPlotConfig();

    // ��ȡΨһ��ʶ��
    const ImGuiID id = window->GetID(label);
    const ImVec2 frame_size = CalcItemSize(size, 256.0f, 256.0f); // ����Ĭ����С�ߴ�
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);


    ItemSize(frame_size);
    if (!ItemAdd(frame_bb, id))
        return false;

   
    const bool is_hovered = ItemHoverable(frame_bb, id, ImGuiItemFlags_None);
    bool is_clicked = false;

    //����任����
    static ImVec2 offset(0.0f, 0.0f);
    static float scale = 1.0f;
    scale = std::clamp(scale, 0.5f, 10.0f); //�������ŷ�Χ

  
    auto ToScreen = [&](float x, float y) -> ImVec2 {
        // ����ֵԼ��
        x = std::clamp(x, x_min, x_max);
        y = std::clamp(y, -1.0f, 1.0f);

        return ImVec2(
            ((x - x_min) / (x_max - x_min)) * frame_size.x * scale + frame_bb.Min.x + offset.x,
            frame_bb.Max.y - ((y + 1.0f) / 2.0f) * frame_size.y * scale + offset.y
        );
        };

    // ��ȡ�����б������òü�����
    ImDrawList* draw_list = GetWindowDrawList();
    draw_list->PushClipRect(frame_bb.Min, frame_bb.Max, true); 

  
    draw_list->AddRectFilled(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg));


    const float base_samples = frame_size.x * scale;
    const int samples = static_cast<int>(std::clamp(base_samples, 100.0f, 2000.0f)); 


    if (!(flags& PImGuiFunctinPlotFlags_NoGrid)) {
        const float grid_step_x = (x_max - x_min) / 10.0f;
        const float grid_step_y = 0.2f;

        //����������
        for (float x = x_min; x <= x_max; x += grid_step_x) {
            ImVec2 p0 = ToScreen(x, -1.0f);
            ImVec2 p1 = ToScreen(x, 1.0f);
            draw_list->AddLine(p0, p1, GetColorU32(ImGuiCol_Border), cfg.grid_thickness);
        }

        //����������
        for (float y = -1.0f; y <= 1.0f; y += grid_step_y) {
            ImVec2 p0 = ToScreen(x_min, y);
            ImVec2 p1 = ToScreen(x_max, y);
            draw_list->AddLine(p0, p1, GetColorU32(ImGuiCol_Border), cfg.grid_thickness);
        }
    }


    ImVec2 prev_point;
    bool first_point = true;
    for (int i = 0; i < samples; ++i) {
        const float t = static_cast<float>(i) / (samples - 1);
        const float x = x_min + t * (x_max - x_min);
        const float y = func(x);

        const ImVec2 p = ToScreen(x, y);

        if (!first_point) {
            //����߶��Ƿ��ڿ���������
            const bool line_visible =
                (prev_point.x >= frame_bb.Min.x && prev_point.x <= frame_bb.Max.x) ||
                (p.x >= frame_bb.Min.x && p.x <= frame_bb.Max.x);

            if (line_visible) {
                draw_list->AddLine(
                    prev_point, p,
                    ColorConvertFloat4ToU32(cfg.line_color),
                    cfg.line_thickness
                );
            }
        }

        prev_point = p;
        first_point = false;
    }

    //������
    draw_list->AddLine(
        ToScreen(x_min, 0), ToScreen(x_max, 0),
        ColorConvertFloat4ToU32(cfg.axis_color), cfg.axis_thickness
    );
    draw_list->AddLine(
        ToScreen(0, -1), ToScreen(0, 1),
        ColorConvertFloat4ToU32(cfg.axis_color), cfg.axis_thickness
    );

    //����
    if (is_hovered) {
        if (!(flags&PImGuiFunctinPlotFlags_NoZooom)) {

            const float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f) {
                const float zoom_factor = 1.1f;
                scale *= (wheel > 0) ? zoom_factor : 1.0f / zoom_factor;
                scale = std::clamp(scale, 0.5f, 10.0f); //�������ŷ�Χ
            }
        }


        is_clicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
    }


    draw_list->PopClipRect();

    return is_clicked;
}

// License Key输入回调 - 自动格式化为 XXXX-XXXX-XXXX-XXXX
static int InputLicenseKeyCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        // 允许输入字母和数字
        char c = data->EventChar;
        if ((c >= '0' && c <= '9') || 
            (c >= 'a' && c <= 'z') || 
            (c >= 'A' && c <= 'Z')) {
            // 转换为大写
            if (c >= 'a' && c <= 'z') {
                data->EventChar = c - 'a' + 'A';
            }
            return 0; // 允许输入
        }
        return 1; // 拒绝输入
    }
    
    if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) {
        // 获取当前文本
        char buf[32] = {0};
        int len = data->BufTextLen;
        if (len > 19) len = 19; // XXXX-XXXX-XXXX-XXXX 最大长度
        
        // 提取有效字符（去掉现有的分隔符）
        int j = 0;
        for (int i = 0; i < len && j < 16; i++) {
            char c = data->Buf[i];
            if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) {
                buf[j++] = c;
            }
        }
        
        // 格式化为 XXXX-XXXX-XXXX-XXXX
        char formatted[20] = {0};
        int pos = 0;
        for (int i = 0; i < j && pos < 19; i++) {
            if (i > 0 && i % 4 == 0 && pos < 19) {
                formatted[pos++] = '-';
            }
            formatted[pos++] = buf[i];
        }
        
        // 更新文本
        if (pos > 0) {
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, formatted);
        }
        
        // 调整光标位置
        int cursor_pos = data->CursorPos;
        // 计算格式化后的光标位置
        int formatted_cursor = cursor_pos;
        if (cursor_pos > 4) formatted_cursor++;
        if (cursor_pos > 8) formatted_cursor++;
        if (cursor_pos > 12) formatted_cursor++;
        if (formatted_cursor > pos) formatted_cursor = pos;
        data->CursorPos = formatted_cursor;
    }
    
    return 0;
}

bool PImGui::InputLicenseKey(const char* label, char* buffer, size_t buffer_size, const char* hint, float width) {
    // 设置输入框宽度
    if (width > 0.0f) {
        ImGui::PushItemWidth(width);
    }
    
    // 输入框标志
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackCharFilter | 
                                ImGuiInputTextFlags_CallbackEdit |
                                ImGuiInputTextFlags_CharsUppercase |
                                ImGuiInputTextFlags_NoHorizontalScroll;
    
    // 创建输入框
    bool result;
    if (hint && hint[0]) {
        result = ImGui::InputTextWithHint(label, hint, buffer, buffer_size, flags, InputLicenseKeyCallback);
    } else {
        result = ImGui::InputText(label, buffer, buffer_size, flags, InputLicenseKeyCallback);
    }
    
    if (width > 0.0f) {
        ImGui::PopItemWidth();
    }
    
    return result;
}

// ========== 动画输入框组件 ==========
#include <EasingAnimationSystem.h>
using namespace ImEasing;

// 光标动画状态
struct CursorAnimState {
    Animation<float> blink_anim;
    float last_blink_time = 0.0f;
    bool visible = true;
};

// 字符输入动画状态
struct CharAnimState {
    Animation<float> scale_anim;
    Animation<float> alpha_anim;
    int char_index = -1;
    bool active = false;
};

static CursorAnimState g_cursor_state;
static CharAnimState g_char_state;

// 更新光标闪动动画（使用缓动）
static void UpdateCursorBlink(float delta_time) {
    const float blink_interval = 0.53f; // 光标闪动间隔
    float current_time = ImGui::GetTime();
    
    if (current_time - g_cursor_state.last_blink_time > blink_interval) {
        g_cursor_state.last_blink_time = current_time;
        g_cursor_state.visible = !g_cursor_state.visible;
        
        // 使用缓动动画切换透明度
        Params blink_params;
        blink_params.duration = 0.15f;
        blink_params.easing = Ease::InOutQuad;
        
        if (g_cursor_state.visible) {
            g_cursor_state.blink_anim.Start(0.0f, 1.0f, blink_params);
        } else {
            g_cursor_state.blink_anim.Start(1.0f, 0.0f, blink_params);
        }
    }
    
    g_cursor_state.blink_anim.Update();
}

// 触发字符输入动画
static void TriggerCharAnim(int char_index) {
    g_char_state.char_index = char_index;
    g_char_state.active = true;
    
    Params scale_params;
    scale_params.duration = 0.3f;
    scale_params.easing = Ease::OutElastic; // 弹性效果
    g_char_state.scale_anim.Start(1.5f, 1.0f, scale_params);
    
    Params alpha_params;
    alpha_params.duration = 0.2f;
    alpha_params.easing = Ease::OutQuad;
    g_char_state.alpha_anim.Start(0.0f, 1.0f, alpha_params);
}

// 更新字符动画
static void UpdateCharAnim() {
    if (g_char_state.active) {
        g_char_state.scale_anim.Update();
        g_char_state.alpha_anim.Update();
        
        // 动画完成后重置
        if (g_char_state.scale_anim.GetState() == Stopped && 
            g_char_state.alpha_anim.GetState() == Stopped) {
            g_char_state.active = false;
        }
    }
}

// 带动画的输入框回调
static int AnimatedInputCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        // 允许输入字母和数字
        char c = data->EventChar;
        if ((c >= '0' && c <= '9') || 
            (c >= 'a' && c <= 'z') || 
            (c >= 'A' && c <= 'Z')) {
            // 转换为大写
            if (c >= 'a' && c <= 'z') {
                data->EventChar = c - 'a' + 'A';
            }
            // 触发字符输入动画
            TriggerCharAnim(data->BufTextLen);
            return 0;
        }
        return 1;
    }
    
    if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit) {
        // 格式化逻辑与之前相同
        char buf[32] = {0};
        int len = data->BufTextLen;
        if (len > 19) len = 19;
        
        int j = 0;
        for (int i = 0; i < len && j < 16; i++) {
            char c = data->Buf[i];
            if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z')) {
                buf[j++] = c;
            }
        }
        
        char formatted[20] = {0};
        int pos = 0;
        for (int i = 0; i < j && pos < 19; i++) {
            if (i > 0 && i % 4 == 0 && pos < 19) {
                formatted[pos++] = '-';
            }
            formatted[pos++] = buf[i];
        }
        
        if (pos > 0) {
            data->DeleteChars(0, data->BufTextLen);
            data->InsertChars(0, formatted);
        }
        
        int cursor_pos = data->CursorPos;
        int formatted_cursor = cursor_pos;
        if (cursor_pos > 4) formatted_cursor++;
        if (cursor_pos > 8) formatted_cursor++;
        if (cursor_pos > 12) formatted_cursor++;
        if (formatted_cursor > pos) formatted_cursor = pos;
        data->CursorPos = formatted_cursor;
    }
    
    return 0;
}

// 绘制带动画的光标
void DrawAnimatedCursor(ImDrawList* draw_list, ImVec2 pos, ImVec2 size, ImU32 color) {
    UpdateCursorBlink(0.016f); // 假设60fps
    float alpha = g_cursor_state.blink_anim.GetCurrentValue();
    
    if (alpha > 0.01f) {
        ImU32 cursor_color = IM_COL32(
            (color >> 0) & 0xFF,
            (color >> 8) & 0xFF,
            (color >> 16) & 0xFF,
            static_cast<int>(alpha * 255)
        );
        draw_list->AddRectFilled(pos, pos + size, cursor_color, 1.0f);
    }
}

// 带输入动画的License Key输入框
bool PImGui::InputLicenseKeyAnimated(const char* label, char* buffer, size_t buffer_size, 
                                       const char* hint, float width, float fade_alpha) {
    // 设置输入框宽度
    if (width > 0.0f) {
        ImGui::PushItemWidth(width);
    }
    
    // 更新字符动画
    UpdateCharAnim();
    
    // 输入框标志
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_CallbackCharFilter | 
                                ImGuiInputTextFlags_CallbackEdit |
                                ImGuiInputTextFlags_CharsUppercase |
                                ImGuiInputTextFlags_NoHorizontalScroll;
    
    // 创建输入框
    bool result;
    if (hint && hint[0]) {
        result = ImGui::InputTextWithHint(label, hint, buffer, buffer_size, flags, AnimatedInputCallback);
    } else {
        result = ImGui::InputText(label, buffer, buffer_size, flags, AnimatedInputCallback);
    }
    
    // 获取输入框的位置和大小用于绘制动画效果
    ImVec2 input_min = ImGui::GetItemRectMin();
    ImVec2 input_max = ImGui::GetItemRectMax();
    
    // 如果有活跃的字符动画，绘制缩放效果
    if (g_char_state.active && strlen(buffer) > 0) {
        float scale = g_char_state.scale_anim.GetCurrentValue();
        float alpha = g_char_state.alpha_anim.GetCurrentValue() * fade_alpha;
        
        // 计算最后一个字符的位置
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 text_pos = input_min + ImVec2(48.0f, (input_max.y - input_min.y - ImGui::GetTextLineHeight()) / 2.0f);
        
        // 获取最后一个字符
        int len = strlen(buffer);
        if (len > 0 && buffer[len-1] != '-') {
            char last_char[2] = {buffer[len-1], '\0'};
            
            // 计算前面字符的宽度
            char prefix[32] = {0};
            strncpy(prefix, buffer, len - 1);
            prefix[len-1] = '\0';
            float prefix_width = ImGui::CalcTextSize(prefix).x;
            
            // 绘制缩放动画的字符
            ImVec2 char_pos = text_pos + ImVec2(prefix_width, 0);
            ImVec2 char_size = ImGui::CalcTextSize(last_char);
            
            // 计算缩放后的中心点
            ImVec2 center = char_pos + ImVec2(char_size.x * 0.5f, char_size.y * 0.5f);
            ImVec2 scaled_min = center - ImVec2(char_size.x * 0.5f * scale, char_size.y * 0.5f * scale);
            ImVec2 scaled_max = center + ImVec2(char_size.x * 0.5f * scale, char_size.y * 0.5f * scale);
            
            ImU32 anim_color = IM_COL32(0, 88, 188, static_cast<int>(alpha * 255));
            draw_list->AddText(scaled_min, anim_color, last_char);
        }
    }
    
    if (width > 0.0f) {
        ImGui::PopItemWidth();
    }
    
    return result;
}

// ========== 渐变进度条组件 ==========
// 绘制带圆角的水平渐变进度条
// progress: 进度值 (0.0 - 1.0)
// width: 进度条总宽度
// height: 进度条高度
// color_start: 起始颜色 (左边)
// color_end: 结束颜色 (右边)
// bg_color: 背景颜色
void PImGui::GradientProgressBar(float progress, float width, float height, 
                                   ImU32 color_start, ImU32 color_end, ImU32 bg_color) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 pos = ImGui::GetCursorScreenPos();
    
    float radius = height / 2.0f;
    float filled_width = width * ImClamp(progress, 0.0f, 1.0f);
    
    // 绘制背景
    draw_list->AddRectFilled(
        pos,
        ImVec2(pos.x + width, pos.y + height),
        bg_color,
        radius
    );
    
    // 绘制前景渐变
    if (filled_width > 0) {
        int segments = 30;
        float segment_width = filled_width / segments;
        
        // 解析起始和结束颜色
        int start_r = (color_start >> 0) & 0xFF;
        int start_g = (color_start >> 8) & 0xFF;
        int start_b = (color_start >> 16) & 0xFF;
        int end_r = (color_end >> 0) & 0xFF;
        int end_g = (color_end >> 8) & 0xFF;
        int end_b = (color_end >> 16) & 0xFF;
        
        // 绘制主体渐变部分
        for (int i = 0; i < segments; i++) {
            float seg_x1 = pos.x + i * segment_width;
            float seg_x2 = pos.x + (i + 1) * segment_width;
            
            float t = (i + 0.5f) / segments;
            int r = (int)(start_r + (end_r - start_r) * t);
            int g = (int)(start_g + (end_g - start_g) * t);
            int b = (int)(start_b + (end_b - start_b) * t);
            ImU32 col = IM_COL32(r, g, b, 255);
            
            if (i == 0 && filled_width >= radius) {
                seg_x1 = pos.x + radius;
            }
            if (i == segments - 1 && filled_width >= radius) {
                seg_x2 = pos.x + filled_width - radius;
            }
            
            if (seg_x2 > seg_x1) {
                draw_list->AddRectFilled(
                    ImVec2(seg_x1, pos.y),
                    ImVec2(seg_x2, pos.y + height),
                    col,
                    0.0f
                );
            }
        }
        
        // 绘制左端半圆
        if (filled_width >= radius) {
            draw_list->AddCircleFilled(
                ImVec2(pos.x + radius, pos.y + radius),
                radius,
                color_start,
                12
            );
        }
        
        // 绘制右端半圆
        if (filled_width >= radius * 2) {
            draw_list->AddCircleFilled(
                ImVec2(pos.x + filled_width - radius, pos.y + radius),
                radius,
                color_end,
                12
            );
        } else if (filled_width > radius) {
            float t_end = filled_width / width;
            int r = (int)(start_r + (end_r - start_r) * t_end);
            int g = (int)(start_g + (end_g - start_g) * t_end);
            int b = (int)(start_b + (end_b - start_b) * t_end);
            ImU32 right_col = IM_COL32(r, g, b, 255);
            draw_list->AddCircleFilled(
                ImVec2(pos.x + filled_width - radius, pos.y + radius),
                radius,
                right_col,
                12
            );
        }
    }
    
    // 占用空间
    ImGui::Dummy(ImVec2(width, height));
}


