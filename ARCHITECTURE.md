# Pandora ImGui - 架构设计文档

## 🎯 设计目标

1. **与Imgui本体完全解耦** - 不修改imgui核心文件
2. **易于移植和更新** - 可以随时更新imgui版本
3. **模块化设计** - 功能模块独立，按需使用
4. **清晰的接口** - 简单易用的API

---

## 📂 完整目录结构

```
Pandora-imgui-docking/
│
├── imgui/                           # [纯净] Imgui本体（未修改！）
│   ├── imgui.h
│   ├── imgui.cpp
│   ├── imgui_internal.h
│   ├── imgui_widgets.cpp
│   ├── imgui_tables.cpp
│   ├── imgui_draw.cpp
│   ├── imgui_demo.cpp
│   └── ...
│
├── backends/                        # Imgui后端（保持原样）
│   ├── imgui_impl_win32.h/cpp
│   ├── imgui_impl_dx11.h/cpp
│   ├── imgui_impl_opengl3.h/cpp
│   └── ...
│
├── pandora/                         # [独立] Pandora扩展库 ✨
│   ├── include/
│   │   └── pandora/
│   │       ├── pandora.h            # 主入口，包含所有
│   │       │
│   │       ├── pandora_types.h      # 类型定义
│   │       │   └── PandoraWindowFlags, PandoraWindowData
│   │       │
│   │       ├── pandora_window.h     # 窗口管理
│   │       │   └── WindowManager
│   │       │
│   │       ├── pandora_animation.h  # 动画系统
│   │       │   ├── Animation<T>
│   │       │   ├── AnimationController
│   │       │   └── Ease::*
│   │       │
│   │       ├── pandora_widgets.h    # 扩展控件
│   │       │   ├── FunctionPlot
│   │       │   └── InputTextPopup
│   │       │
│   │       ├── pandora_platform.h   # 平台相关 [Windows only]
│   │       │   ├── DropManager
│   │       │   ├── WindowUtil
│   │       │   └── CallbackManager
│   │       │
│   │       └── pandora_render.h     # 渲染相关 [DX11 only]
│   │           ├── SVGCacheManager
│   │           └── LottieAnimator
│   │
│   └── src/
│       ├── core/
│       │   └── pandora_window.cpp
│       │
│       ├── platform/                 # Windows平台实现
│       │   ├── pandora_time.hpp
│       │   ├── pandora_dropfile.hpp
│       │   └── ...
│       │
│       └── render/                   # DX11渲染实现
│           └── ...
│
├── examples/                        # 示例项目
│   └── win32_dx11/
│       ├── main.cpp
│       ├── GuiMain.h/cpp
│       └── ...
│
├── thirdparty/                      # 第三方依赖
│   ├── nanosvg/
│   └── rlottie/
│
├── misc/                            # Imgui附带文件
│   ├── fonts/
│   ├── freetype/
│   └── ...
│
├── PANDORA_README.md                # Pandora扩展说明
├── ARCHITECTURE.md                  # 本文档
├── MIGRATION_GUIDE.md               # 迁移指南
└── README.md                        # 原项目说明
```

---

## 🔧 核心设计原则

### 1. 分层架构

```
┌─────────────────────────────────────┐
│   应用层 (Application)              │
│   example_win32_directx11/         │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│   Pandora扩展层                      │
│   pandora/                           │
│   ├── WindowManager                  │
│   ├── Animation System               │
│   ├── Widgets                        │
│   ├── Platform (Windows)             │
│   └── Render (DX11)                 │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│   Imgui核心层 (未修改!)              │
│   imgui.h, imgui.cpp, ...           │
└─────────────────────────────────────┘
```

### 2. 依赖关系

```
应用 → pandora_*.h → imgui.h (公共API)
                 ↳ Windows API (条件编译)
                 ↳ DirectX API (条件编译)
```

**关键：pandora只依赖imgui的公共接口，不依赖imgui_internal.h！**

### 3. 条件编译

平台和渲染后端通过条件编译隔离：

```cpp
// pandora_platform.h
#ifdef _WIN32
    // Windows特定代码
#endif

// pandora_render.h
#ifdef _WIN32
#ifdef DIRECT3D11_VERSION
    // DX11特定代码
#endif
#endif
```

---

## 🧩 模块说明

### WindowManager (`pandora_window.h`)
管理Pandora特定的窗口属性，替代修改imgui.h的方式。

```cpp
// 不再修改imgui.h添加Window Flags！
// 而是用外部管理器：

WindowManager& mgr = WindowManager::GetInstance();
ImGuiID id = ImGui::GetID("My Window");

mgr.SetDropFilesEnabled(id, true);
mgr.SetBlurBackground(id, true);
mgr.SetShadowWindow(id, true);
```

### Animation System (`pandora_animation.h`)
完整的动画系统，支持缓动函数、循环、回调等。

```cpp
using namespace Pandora::Animation;

auto anim = std::make_shared<Animation<float>>();
anim->Start(0.0f, 1.0f, {
    .duration = 1.0f,
    .easing = Ease::InOutQuad,
    .loopCount = -1,  // 无限循环
    .pingPong = true
});
```

### Platform (`pandora_platform.h`)
Windows特定功能：
- 拖放支持 (DropManager)
- 窗口工具 (WindowUtil)
- 回调管理 (CallbackManager)

### Render (`pandora_render.h`)
DX11特定渲染功能：
- SVG渲染 (SVGCacheManager)
- Lottie动画 (LottieAnimator)

---

## 🚀 使用指南

### 最小化集成

```cpp
// 只包含你需要的模块
#include <imgui.h>
#include <pandora/pandora_animation.h>  // 只用动画

// 或者包含全部
#include <pandora/pandora.h>
```

### 初始化顺序

```cpp
// 1. 初始化Imgui
ImGui::CreateContext();
ImGui_ImplWin32_Init(hwnd);
ImGui_ImplDX11_Init(device, ctx);

// 2. 初始化Pandora（按需）
#ifdef _WIN32
Pandora::Platform::CallbackManager::GetInstance(&ImGui::GetPlatformIO());
Pandora::Platform::DropManager::Initialize();
#endif
```

---

## 🔄 更新Imgui流程

### 以前（有问题）
```
1. 修改了imgui.h, imgui.cpp
2. 更新imgui时需要手动合并修改 ❌
3. 容易出错，耗时
```

### 现在（完美）
```
1. imgui核心文件完全纯净 ✅
2. 删除旧imgui，复制新版本
3. 重新编译 - 无需修改任何代码！
4. Done! 🎉
```

---

## 📋 Checklist

### 在提交代码前检查

- [ ] 没有修改 `imgui.h`, `imgui.cpp`, `imgui_internal.h` 等核心文件
- [ ] 新功能放在 `pandora/` 目录
- [ ] 平台特定代码用 `#ifdef _WIN32` 包裹
- [ ] 渲染后端代码用条件编译包裹
- [ ] 更新了 `PANDORA_README.md` (如果需要)

---

## 💡 最佳实践

1. **保持imgui纯净** - 永远不要修改imgui核心文件
2. **使用命名空间** - 所有Pandora功能在 `Pandora::` 命名空间
3. **条件编译** - 平台/后端特定代码必须隔离
4. **模块化** - 只依赖你需要的头文件
5. **文档化** - 添加新模块时更新本文档
