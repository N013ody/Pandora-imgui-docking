#include "pandora/pandora_window.h"

namespace Pandora {

WindowManager& WindowManager::GetInstance() {
    static WindowManager instance;
    return instance;
}

void WindowManager::SetWindowData(ImGuiID windowId, const PandoraWindowData& data) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_windowData[windowId] = data;
}

PandoraWindowData* WindowManager::GetWindowData(ImGuiID windowId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_windowData.find(windowId);
    return (it != m_windowData.end()) ? &it->second : nullptr;
}

void WindowManager::RemoveWindowData(ImGuiID windowId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_windowData.erase(windowId);
}

void WindowManager::BeginWindow(const char* name, bool* p_open, ImGuiWindowFlags flags) {
    ImGui::Begin(name, p_open, flags);
}

void WindowManager::EndWindow() {
    ImGui::End();
}

void WindowManager::SetDropFilesEnabled(ImGuiID windowId, bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& data = m_windowData[windowId];
    if (enabled) {
        data.flags |= PandoraWindowFlags_DropFiles;
    } else {
        data.flags &= ~PandoraWindowFlags_DropFiles;
    }
}

bool WindowManager::IsDropFilesEnabled(ImGuiID windowId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_windowData.find(windowId);
    return (it != m_windowData.end()) ? (it->second.flags & PandoraWindowFlags_DropFiles) != 0 : false;
}

void WindowManager::SetBlurBackground(ImGuiID windowId, bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& data = m_windowData[windowId];
    if (enabled) {
        data.flags |= PandoraWindowFlags_BlurBackground;
    } else {
        data.flags &= ~PandoraWindowFlags_BlurBackground;
    }
}

bool WindowManager::IsBlurBackground(ImGuiID windowId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_windowData.find(windowId);
    return (it != m_windowData.end()) ? (it->second.flags & PandoraWindowFlags_BlurBackground) != 0 : false;
}

void WindowManager::SetShadowWindow(ImGuiID windowId, bool enabled) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto& data = m_windowData[windowId];
    if (enabled) {
        data.flags |= PandoraWindowFlags_ShadowWindow;
    } else {
        data.flags &= ~PandoraWindowFlags_ShadowWindow;
    }
}

bool WindowManager::IsShadowWindow(ImGuiID windowId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_windowData.find(windowId);
    return (it != m_windowData.end()) ? (it->second.flags & PandoraWindowFlags_ShadowWindow) != 0 : false;
}

}
