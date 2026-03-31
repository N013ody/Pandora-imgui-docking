# Pandora ImGui - 现代化 ImGui 扩展框架

<p align="center">
  <img src="https://img.shields.io/badge/ImGui-Docking-blue?style=flat-square" alt="ImGui Docking">
  <img src="https://img.shields.io/badge/Platform-Windows-blue?style=flat-square" alt="Platform">
  <img src="https://img.shields.io/badge/Renderer-DirectX11-blue?style=flat-square" alt="DirectX11">
  <img src="https://img.shields.io/badge/License-MIT-green?style=flat-square" alt="License">
</p>

<p align="center">
  <b>让你的 ImGui 应用看起来像原生 Windows 程序</b><br>
  <i>Modern ImGui Extension Framework with Native Windows Look & Feel</i>
</p>

---

## ✨ 核心特性

### 🎨 视觉效果
- **背景模糊** - 基于 Windows DWM 的实时背景模糊效果
- **窗口阴影** - 自适应窗口阴影，支持自定义强度和颜色
- **透明度调节** - 窗口整体透明度控制
- **圆角窗口** - 现代风格的圆角窗口边框
- **纹理旋转** - 支持 360° 动态纹理旋转

### 🚀 动画系统
- **缓动函数** - 内置 20+ 种缓动函数（Linear, Quad, Cubic, Elastic, Bounce 等）
- **类型安全** - 模板化动画系统，支持 `float`, `ImVec2`, `ImVec4` 等类型
- **链式调用** - 流畅的动画配置 API

### 📐 扩展功能
- **SVG 渲染** - 基于 NanoSVG 的高质量 SVG 渲染，支持缩放缓存
- **Lottie 动画** - 基于 rlottie 的 Lottie 动画播放支持
- **文件拖放** - 原生 Windows 文件拖放支持
- **多视口** - ImGui 多视口支持，窗口可脱离主窗口
- **FreeType 字体** - 集成 FreeType，支持高质量字体渲染和彩色表情

---

## 📁 项目结构

```
Pandora-imgui-docking/
├── backends/                    # ImGui 后端实现
│   ├── imgui_impl_dx11.cpp/h   # DirectX11 后端
│   ├── imgui_impl_win32.cpp/h  # Win32 后端
│   └── pimgui.cpp/h            # Pandora 扩展后端
│
├── example_win32_directx11/     # 示例项目
│   ├── main.cpp                # 程序入口
│   ├── GuiMain.cpp/h           # 主界面渲染
│   ├── LottieAnimator.cpp/h    # Lottie 动画封装
│   ├── SVGCacheManager.hpp     # SVG 缓存管理
│   ├── EasingAnimationSystem.h # 动画系统
│   ├── Utils/                  # 工具类
│   │   ├── WindowUtil.cpp/h    # 窗口工具
│   │   └── MathUtil.cpp/h      # 数学工具
│   ├── Blur/                   # 模糊效果着色器
│   └── Profile/                # 性能分析
│
├── pandora/                     # Pandora 扩展库（重构中）
│   ├── include/pandora/        # 头文件
│   └── src/                    # 实现文件
│
├── vcpkg_ports/                 # 自定义 vcpkg 端口
│   ├── nanosvg/                # NanoSVG 端口
│   └── rlottie/                # rlottie 端口
│
├── vcpkg.json                   # vcpkg 依赖配置
└── imgui_*.cpp/h               # ImGui 核心文件
```

---

## 🛠️ 构建步骤

### 前置要求

- **Visual Studio 2022** 或更高版本
- **Windows SDK 10.0** 或更高版本
- **vcpkg** - C++ 包管理器
- **Git** - 用于克隆依赖

### 1. 安装 vcpkg

如果尚未安装 vcpkg：

```powershell
# 克隆 vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg

# 运行安装脚本
C:\vcpkg\bootstrap-vcpkg.bat

# 添加到环境变量（可选）
setx PATH "%PATH%;C:\vcpkg"
```

### 2. 克隆项目

```powershell
git clone <repository-url>
cd Pandora-imgui-docking
```

### 3. 自动安装依赖

项目使用 vcpkg manifest 模式，依赖会在构建时自动安装：

```powershell
# 依赖列表（定义在 vcpkg.json 中）：
# - directxtk    : DirectX Tool Kit
# - rlottie      : Lottie 动画库（静态链接）
# - nanosvg      : SVG 解析库（header-only）
```

### 4. 构建项目

#### 使用 Visual Studio

1. 打开 `imgui_examples.sln`
2. 选择配置：`Debug` 或 `Release`
3. 选择平台：`x64`
4. 菜单栏 → **生成** → **生成解决方案**

#### 使用命令行

```powershell
# 进入项目目录
cd example_win32_directx11

# 使用 MSBuild 构建（Debug）
msbuild example_win32_directx11.vcxproj /p:Configuration=Debug /p:Platform=x64

# 使用 MSBuild 构建（Release）
msbuild example_win32_directx11.vcxproj /p:Configuration=Release /p:Platform=x64
```

### 5. 运行程序

构建完成后，可执行文件位于：

```
example_win32_directx11/Debug/example_win32_directx11.exe
example_win32_directx11/Release/example_win32_directx11.exe
```

---

## 📦 依赖说明

| 依赖 | 版本 | 用途 | 链接方式 |
|------|------|------|----------|
| [Dear ImGui](https://github.com/ocornut/imgui) | docking | GUI 框架 | 源码集成 |
| [DirectX Tool Kit](https://github.com/microsoft/DirectXTK) | 最新 | 纹理加载 | 动态链接 |
| [rlottie](https://github.com/Samsung/rlottie) | 0.2 | Lottie 动画 | **静态链接** |
| [nanosvg](https://github.com/memononen/nanosvg) | 最新 | SVG 渲染 | header-only |
| FreeType | 最新 | 字体渲染 | 源码集成 |

> **注意**: rlottie 已配置为静态链接，运行时不需要 `rlottie.dll`。

---

## 🎮 使用示例

### 动画系统

```cpp
#include <EasingAnimationSystem.h>
using namespace ImEasing;

// 创建动画
auto anim = std::make_shared<Animation<float>>();

// 配置并启动
Params params;
params.duration = 1.0f;
params.easing = Ease::OutBounce;
anim->Start(0.0f, 100.0f, params);

// 在渲染循环中更新
anim->Update();
float value = anim->GetCurrentValue();
```

### SVG 渲染

```cpp
#include <SVGCacheManager.hpp>

SVGCacheManager cacheManager;
cacheManager.SetDevice(g_pd3dDevice);
cacheManager.LoadSVG("icon.svg");

// 获取纹理并渲染
SVGTexture tex = cacheManager.GetScaledTexture(1.0f);
ImGui::Image(tex.srv, ImVec2(100, 100));
```

### Lottie 动画

```cpp
#include <LottieAnimator.h>

LottieAnimator animator;
animator.Init("animation.json", g_pd3dDevice, 256, 256);

// 在渲染循环中更新和显示
animator.Update(deltaTime);
ImGui::Image(animator.GetTextureID(), animator.GetSize());
```

---

## 🔧 配置选项

### 静态链接 rlottie

rlottie 默认配置为静态链接。如需修改，编辑：

```cmake
# vcpkg_ports/rlottie/portfile.cmake
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DLOTTIE_MODULE=OFF
        -DLOTTIE_THREAD=OFF
        -DBUILD_SHARED_LIBS=OFF  # OFF=静态链接, ON=动态链接
)
```

### 修改窗口效果

```cpp
// WindowUtil.h
void EnableBlurBehind(HWND hwnd);        // 启用背景模糊
void EnableDarkMode(HWND hwnd);          // 启用深色模式
void SetWindowRounded(HWND hwnd, int r); // 设置圆角半径
```

---

## 🐛 常见问题

### Q: 构建时提示找不到 vcpkg 包？
**A**: 确保 vcpkg 已正确安装并集成到 Visual Studio：
```powershell
vcpkg integrate install
```

### Q: 运行时提示缺少 DLL？
**A**: 
- `DirectXTK.dll` - 需要随程序一起分发
- `rlottie.dll` - 已静态链接，不需要

### Q: 如何更新 ImGui 版本？
**A**: 直接替换 `imgui*.cpp/h` 文件即可，Pandora 扩展与 ImGui 本体解耦。

### Q: SVG 渲染模糊？
**A**: 使用 `SVGCacheManager::GetScaledTexture(scale)` 并传入适当的缩放比例。

---

## 🗺️ 开发路线图

| 功能 | 状态 | 描述 |
|------|------|------|
| 多视口着色器支持 | 🚧 计划中 | 为独立视口应用自定义效果 |
| 基于着色器的阴影 | 🚧 计划中 | GPU 加速的动态阴影 |
| 基于着色器的模糊 | 🚧 计划中 | 高性能 GPU 模糊效果 |
| 跨平台支持 | 📋 调研中 | Linux/macOS 后端 |

---

## 📄 许可证

本项目基于 [MIT License](LICENSE) 开源。

### 第三方组件
- **Dear ImGui**: MIT License
- **rlottie**: MIT / BSD-3-Clause
- **nanosvg**: zlib License
- **DirectX Tool Kit**: MIT License

---

## 🙏 致谢

- [ocornut/imgui](https://github.com/ocornut/imgui) - 优秀的即时模式 GUI 库
- [Samsung/rlottie](https://github.com/Samsung/rlottie) - Lottie 动画渲染
- [memononen/nanosvg](https://github.com/memononen/nanosvg) - SVG 解析
- [iconfont](https://www.iconfont.cn/) - 测试资源

---

<p align="center">
  <sub>Built with ❤️ for the ImGui community</sub>
</p>
