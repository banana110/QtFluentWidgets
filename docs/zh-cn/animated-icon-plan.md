# AnimatedIcon 实施计划

目标：为 QtFluentWidgets 增加 WinUI 风格的 `AnimatedIcon` 能力，让按钮、导航项、搜索入口和状态反馈可以使用 Lottie 动画，同时保持 Qt Widgets / Qt5 / Qt6 的轻量兼容。

当前使用说明见 [lottie.md](lottie.md)；本文保留实现拆分、构建策略与后续计划。

## 背景

WinUI Gallery 中的 `AnimatedIcon` 不只是播放一段动画。它的核心语义是把控件状态映射到 Lottie markers，例如 `NormalToPointerOver_Start` / `NormalToPointerOver_End`，从而让按钮 hover、press、selected 等状态拥有一致的过渡动画。

Qt 自带 Qt Lottie Animation 更偏 Qt Quick/QML，且许可约束与本项目当前 Qt Widgets 目标不完全贴合。`rlottie` 是跨平台 C++ Lottie renderer，支持从文件或原始 JSON 加载、查询帧率/总帧数、同步/异步渲染到 ARGB32 buffer，适合作为本项目的第一版 QWidget 后端。

## 设计原则

- `rlottie` 作为 `third_party/rlottie` Git 子模块集成，用户不需要单独安装系统包。
- 公开头文件不暴露 `rlottie` 类型，避免把第三方 ABI 传播给使用方。
- 先做 QWidget 级别的基础播放和状态切换，再逐步接入 `FluentButton`、`FluentIconButton`、`FluentNavigationView`。
- 支持 Lottie markers 的 WinUI 命名约定，但不要求第一版覆盖 LottieGen 的全部能力。
- 保留 fallback icon，便于加载失败或用户关闭动画时显示静态图标。

## 控件拆分

### FluentLottieWidget

通用 Lottie 播放控件，负责：

- `setSource()` / `load()`：从文件加载 JSON。
- `loadData()`：从内存 JSON 加载。
- `play()` / `pause()` / `stop()`。
- `setCurrentFrame()` / `setProgress()`。
- `playSegment(startFrame, endFrame)`。
- 解析 Lottie 顶层 `markers`，暴露 `markerNames()` / `hasMarker()` / `markerFrame()`。
- 没有后端或加载失败时绘制 fallback icon。

### FluentAnimatedIcon

WinUI 风格状态控件，基于 `FluentLottieWidget`，负责：

- `setState(QString state, bool animated = true)`。
- 默认状态：`Normal`。
- 可选 `interactive` 模式，控件自身 hover/press 时自动切换 `PointerOver` / `Pressed` / `Normal`。
- marker 查找规则：
  - `[PreviousState]To[NewState]_Start` + `_End`：播放区间。
  - 只有 `_End` 或 `_Start`：跳转到该帧。
  - `[PreviousState]To[NewState]`：跳转到 transition marker。
  - `[NewState]`：跳转到状态 marker。
  - 任意 `To[NewState]_End`：作为兜底状态帧。
  - 数字 state：作为目标帧。
  - 否则回到 0 帧。

### FluentAnimatedButton

按钮语义封装，基于 `QPushButton` + 内部 `FluentAnimatedIcon`，负责：

- 保留 `clicked` / `toggled` / `setCheckable()` 等 Qt 按钮语义。
- 按钮自身绘制 Fluent 背景、Primary/Secondary、hover/press/focus。
- hover 时把内部图标切到 `PointerOver`，press 时切到 `Pressed`，离开时回到 `Normal`。
- checkable 且 Lottie 提供 `Selected` marker 时，checked 状态回到 `Selected`。
- 提供 `loadAnimation()` / `loadAnimationData()` / `animatedIcon()` / `setAnimatedIconSize()`。

## CMake 策略

`rlottie` 通过 Git 子模块进入源码树：

```bash
git submodule update --init --recursive
```

主工程通过 `add_subdirectory(third_party/rlottie EXCLUDE_FROM_ALL)` 引入 `rlottie::rlottie`，并链接到 `QtFluentWidgets`。配置时如果子模块没有初始化，直接给出明确错误提示。

集成约定：

- `LOTTIE_MODULE=OFF`：把图片 loader 编进 rlottie，避免额外动态插件。
- `LOTTIE_TEST=OFF`：不构建 rlottie 自带测试。
- 使用 `EXCLUDE_FROM_ALL`，避免构建 rlottie 自带示例工具；只有被 `QtFluentWidgets` 依赖的库目标会参与构建。
- Windows 下把 rlottie 运行时库复制到 Demo / Designer 插件输出目录旁边，方便直接运行。

## 分阶段落地

1. 新增文档、rlottie 子模块、CMake 源码级集成、`FluentLottieWidget`、`FluentAnimatedIcon`、`FluentAnimatedButton`。
2. Demo 加入 `AnimatedIcon` 示例，展示 fallback 和 marker state API。
3. Designer 插件注册新控件。
4. 后续把 `FluentAnimatedIcon` 接入：
   - `FluentIconButton`：图标槽支持 animated icon。
   - `FluentNavigationView`：item hover/pressed/selected 自动设置 state。
   - `AutoSuggestBox` / `SearchBox`：搜索图标动画。

## 后续增强

- 支持系统“减少动画”设置：禁用动画时直接跳到目标帧。
- 支持 Foreground 动态颜色映射，让 Lottie 图标跟随主题文本色/accent 色。
- 支持异步预渲染或小尺寸帧缓存，降低频繁 hover 的 CPU 峰值。
- 评估 ThorVG / dotLottie 后端，尤其是 `.lottie` 包、主题、状态机等能力。
