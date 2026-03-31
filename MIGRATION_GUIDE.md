# Pandora ImGui - 迁移指南

## 🚨 问题说明

### 原始问题
- imgui核心文件（imgui.h, imgui.cpp, imgui_widgets.cpp, imgui_internal.h）被修改了
- 添加了自定义的Window Flags:
  - `ImGuiWindowFlags_DropFiles`
  - `ImGuiWindowFlags_BlurBackGround`
  - `ImGuiWindowFlags_AttachedWindow`
  - `ImGuiWindowFlags_ShadowWindow`
- 这导致无法轻松更新imgui到新版本

---

## ✅ 解决方案

### 1. 恢复Imgui核心文件（关键步骤！）

```bash
# 方法1: 使用git恢复
git checkout -- imgui.h imgui.cpp imgui_widgets.cpp imgui_internal.h

# 方法2: 从官方仓库重新下载
# 删除imgui相关文件，复制新版本
```

### 2. 使用新的Pandora命名空间

#### 旧代码:
```cpp
// ❌ 不好 - 修改了imgui.h
ImGui::Begin("My Window", nullptr, ImGuiWindowFlags_DropFiles);

auto files = PImGui::GetCurrentWindowFiles();
```

#### 新代码:
```cpp
// ✅ 好 - 使用独立的Pandora扩展
#include <pandora/pandora.h>

// 设置窗口标志
ImGuiID windowId = ImGui::GetID("My Window");
Pandora::WindowManager::GetInstance().SetDropFilesEnabled(windowId, true);

ImGui::Begin("My Window");

// 获取拖放文件
auto& dropManager = Pandora::Platform::DropManager::GetInstance();
auto files = dropManager.GetDroppedFiles(windowId);
```

### 3. Window Flags 对照表

| 旧标志 (imgui.h) | 新方式 (Pandora) |
|------------------|-----------------|
| `ImGuiWindowFlags_DropFiles` | `WindowManager::SetDropFilesEnabled(id, true)` |
| `ImGuiWindowFlags_BlurBackGround` | `WindowManager::SetBlurBackground(id, true)` |
| `ImGuiWindowFlags_ShadowWindow` | `WindowManager::SetShadowWindow(id, true)` |
| `ImGuiWindowFlags_AttachedWindow` | (待实现) |

### 4. 命名空间迁移

| 旧命名空间 | 新命名空间 |
|-----------|-----------|
| `PImGui::` | `Pandora::Widgets::` |
| `ImEasing::` | `Pandora::Animation::` |

---

## 📁 目录变化

### 旧结构 (问题)
```
.
├── imgui.h              (修改过!) ❌
├── imgui.cpp            (修改过!) ❌
├── backends/
├── example_win32_directx11/  (混杂了库代码) ❌
│   ├── EasingAnimationSystem.h
│   ├── DropManager.hpp
│   └── ...
```

### 新结构 (正确)
```
.
├── imgui.h              (纯净!) ✅
├── imgui.cpp            (纯净!) ✅
├── pandora/             (独立扩展!) ✅
│   ├── include/pandora/
│   │   ├── pandora.h
│   │   ├── pandora_types.h
│   │   ├── pandora_window.h
│   │   ├── pandora_animation.h
│   │   ├── pandora_widgets.h
│   │   ├── pandora_platform.h
│   │   └── pandora_render.h
│   └── src/
│       ├── core/
│       ├── platform/
│       └── render/
└── examples/            (独立示例) ✅
    └── win32_dx11/
```

---

## 🚀 快速开始

### 初始化
```cpp
#include <imgui.h>
#include <pandora/pandora.h>

// 1. 初始化ImGui (照常)
ImGui::CreateContext();
ImGui_ImplWin32_Init(hwnd);
ImGui_ImplDX11_Init(device, context);

// 2. 初始化Pandora
Pandora::Platform::CallbackManager::GetInstance(&ImGui::GetPlatformIO());
Pandora::Platform::DropManager::Initialize();
```

### 使用动画
```cpp
#include <pandora/pandora_animation.h>

using namespace Pandora::Animation;

auto anim = std::make_shared<Animation<float>>();
anim->Start(0.0f, 1.0f, {
    .duration = 1.0f,
    .easing = Ease::InOutQuad
});

// 每帧更新
anim->Update();
float value = anim->GetCurrentValue();
```

### 使用拖放
```cpp
ImGuiID windowId = ImGui::GetID("Drop Window");
auto& winMgr = Pandora::WindowManager::GetInstance();
winMgr.SetDropFilesEnabled(windowId, true);

if (ImGui::Begin("Drop Window")) {
    auto files = Pandora::Platform::DropManager::GetDroppedFiles(windowId);
    for (auto& file : files) {
        ImGui::Text("Dropped: %s", file.c_str());
    }
}
ImGui::End();
```

---

## 🔄 更新Imgui版本

现在你可以随时安全地更新imgui！

```bash
# 1. 删除旧imgui文件
rm imgui.h imgui.cpp imgui_widgets.cpp imgui_tables.cpp imgui_draw.cpp
rm imgui_internal.h imgui.h imgui_settings.h

# 2. 复制新版本imgui
cp -r /path/to/new/imgui/* ./

# 3. 重新编译
# 不需要修改任何Pandora代码!
```

---

## ⚠️ 注意事项

1. **不要修改imgui核心文件** - 所有扩展都在pandora目录
2. **使用条件编译** - 平台特定代码用 `#ifdef _WIN32` 包裹
3. **保持向后兼容** - 可以提供旧API的wrapper层
