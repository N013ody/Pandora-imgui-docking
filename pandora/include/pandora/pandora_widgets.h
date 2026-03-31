#pragma once
#include <vector>
#include <string>
#include <imgui.h>

namespace Pandora {
namespace Widgets {

struct FunctionPlotConfig {
    ImVec4      line_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4      grid_color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    ImVec4      axis_color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    float       axis_thickness = 2.0f;
    float       line_thickness = 2.0f;
    float       grid_spacing = 1.0f;
    float       grid_thickness = 1.0f;
    const char* x_label = "X";
    const char* y_label = "Y";
};

enum FunctionPlotFlags : int {
    FunctionPlotFlags_None = 0,
    FunctionPlotFlags_NoZoom = 1 << 0,
    FunctionPlotFlags_NoGrid = 1 << 1,
};

enum TextPopupFlags : int {
    TextPopupFlags_None = 0,
    TextPopupFlags_NoPaste = 1 << 0,
    TextPopupFlags_NoCopy = 1 << 1,
};

bool FunctionPlot(const char* label, float(*func)(float x), float x_min, float x_max,
    const ImVec2& size, const FunctionPlotConfig* config, FunctionPlotFlags flags = FunctionPlotFlags_None);

bool InputTextPopup(const char* str_id, char* buffer, TextPopupFlags popup_flags = TextPopupFlags_None);

}
}
