#pragma once
#include <vector>
#include <string>
#include <functional>
#include <imgui.h>

#ifdef _WIN32
#include <wtypes.h>
#endif

namespace Pandora {
namespace Platform {

#ifdef _WIN32

class DropManager {
public:
    static void Initialize();
    static void Shutdown();
    static void RegisterWindowDropTarget(void* hwnd, ImGuiID id);
    static void UnregisterWindowDropTarget(void* hwnd, ImGuiID id);
    static const std::vector<std::string>& GetDroppedFiles(ImGuiID id);
    static void SetupViewportCallbacks();
};

class WindowUtil {
public:
    static void* GetWindowHWND(ImGuiWindow* window);
    static ImGuiWindow* GetWindowByHwnd(void* hwnd);
    static bool AreAllPlatformWindowsClosed();
    static int SetWindowStyles(void* hwnd, unsigned long style);
    static const WNDCLASSEXW GetWindowClass(const std::string& name);
};

#endif

class CallbackManager {
public:
    enum CallbackType {
        CALLBACK_CREATE_WINDOW,
        CALLBACK_DESTROY_WINDOW
    };

    enum CallbackTiming {
        PRE,
        POST
    };

    using WindowCallback = std::function<void(ImGuiViewport*)>;

    static CallbackManager& GetInstance();
    static CallbackManager& GetInstance(ImGuiPlatformIO* platformIO);

    void RegesiterCallBack(WindowCallback callback, CallbackType type, CallbackTiming timing);
    void SetUpCallBack();

private:
    CallbackManager() = default;
    ImGuiPlatformIO* m_platformIO = nullptr;
};

}
}
