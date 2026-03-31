#pragma once
#include "pandora_types.h"
#include <imgui.h>
#include <unordered_map>
#include <mutex>

namespace Pandora {

class WindowManager {
public:
    static WindowManager& GetInstance();

    void SetWindowData(ImGuiID windowId, const PandoraWindowData& data);
    PandoraWindowData* GetWindowData(ImGuiID windowId);
    void RemoveWindowData(ImGuiID windowId);

    void BeginWindow(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void EndWindow();

    void SetDropFilesEnabled(ImGuiID windowId, bool enabled);
    bool IsDropFilesEnabled(ImGuiID windowId);

    void SetBlurBackground(ImGuiID windowId, bool enabled);
    bool IsBlurBackground(ImGuiID windowId);

    void SetShadowWindow(ImGuiID windowId, bool enabled);
    bool IsShadowWindow(ImGuiID windowId);

private:
    WindowManager() = default;
    ~WindowManager() = default;
    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;

    std::unordered_map<ImGuiID, PandoraWindowData> m_windowData;
    std::mutex m_mutex;
};

}
