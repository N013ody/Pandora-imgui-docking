# Pandora ImGui Extensions - 重构后的架构

## 📁 新目录结构

```
Pandora-imgui-docking/
├── imgui/                  # 纯净的imgui本体（未修改！）
│   ├── imgui.h
│   ├── imgui.cpp
│   ├── imgui_internal.h
│   └── ...                 # 其他imgui文件
├── backends/               # imgui后端（保持原样）
│   ├── imgui_impl_dx11.cpp
│   ├── imgui_impl_win32.cpp
│   └── ...
├── pandora/                # Pandora扩展（独立！）
│   ├── include/
│   │   └── pandora/
│   │       ├── pandora.h           # 主入口
│   │       ├── pandora_types.h     # 类型定义
│   │       ├── pandora_animation.h # 动画系统
│   │       ├── pandora_widgets.h   # 扩展控件
│   │       ├── pandora_platform.h  # 平台相关
│   │       └── pandora_render.h    # 渲染相关
│   └── src/
│       ├── core/
│       ├── platform/
│       └── render/
├── examples/               # 示例项目
│   └── win32_dx11/
│       ├── main.cpp
│       └── ...
└── thirdparty/             # 第三方库
    ├── nanosvg/
    └── rlottie/
```

## ✨ 核心改进

### 1. **Imgui本体保持纯净**
- 不再修改 `imgui.h`, `imgui.cpp` 等核心文件
- 可以随时更新imgui到最新版本，只需替换imgui目录即可
- 自定义Window Flags移至 `Pandora::PandoraWindowFlags`

### 2. **完全解耦**
- Pandora扩展完全独立于imgui本体
- 通过接口而非修改内部实现
- 支持条件编译，按需启用功能

### 3. **清晰的依赖关系**
```
pandora_*.h → imgui.h (仅公共接口)
            → 平台/渲染API (条件编译)
```

## 🚀 使用方式

### 基本使用
```cpp
#include <imgui.h>
#include <pandora/pandora.h>

// 初始化Pandora
Pandora::Platform::CallbackManager::GetInstance(&ImGui::GetPlatformIO());
Pandora::Platform::DropManager::Initialize();

// 使用动画
auto anim = std::make_shared<Pandora::Animation::Animation<float>>();
anim->Start(0.0f, 1.0f, { .duration = 1.0f });

// 使用Pandora窗口标志
Pandora::PandoraWindowData windowData;
windowData.flags |= Pandora::PandoraWindowFlags_DropFiles;
```

### 更新imgui
1. 删除旧的imgui目录
2. 复制新的imgui版本
3. 重新编译 - 无需修改任何Pandora代码！

## 📋 迁移指南

### 从旧版本迁移
1. 恢复imgui核心文件到原始版本
2. 将自定义窗口标志改为 `Pandora::PandoraWindowFlags`
3. 使用 `Pandora::` 命名空间访问功能

## 🔧 Windows特定功能
- 拖放支持
- DWM效果（模糊、阴影、圆角）
- SVG渲染
- Lottie动画
