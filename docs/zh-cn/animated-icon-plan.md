# AnimatedIcon 接入计划

本文记录质感优化阶段里 Lottie / AnimatedIcon 的接入边界、已完成能力与后续打磨点。API 细节见 [lottie.md](lottie.md) 和 [buttons.md](buttons.md)。

## 已完成

- `FluentLottieWidget` 提供基础 Lottie JSON 加载、播放控制、marker 读取、fallback icon 和整帧 `tintColor` 重染。
- `FluentAnimatedIcon` 在 Lottie 之上增加 WinUI 风格状态语义：`Normal`、`PointerOver`、`Pressed`、`Disabled`、`Selected`，以及 `_Start` / `_End` transition marker。
- `FluentAnimatedButton` 会把内部动画图标同步到按钮前景色：Primary 使用 `onAccent`，普通按钮使用当前文本色，禁用态使用 `disabledText`。
- `FluentCard` 的折叠指示器可加载动画，`clearCollapseIndicatorAnimation()` 会回到静态 chevron。
- `FluentNavigationView` 支持 pane toggle、back button、导航项动画图标，也支持清除后回到静态 glyph。
- 公开头文件只暴露 Qt 类型，不暴露 rlottie 类型，后续替换渲染后端不会影响使用方 API。

## 设计契约

- 动效必须有静态 fallback：Lottie 加载失败、清除动画或关闭动效时，控件仍要保持可读图标。
- 动效颜色跟随控件语义前景色，不保留素材里的固定源色。
- 所有播放路径遵守全局 motion / reduced-motion 设置；关闭动效时应跳到目标状态帧，而不是继续播放。
- marker 是语义契约。设计资源推荐提供状态 marker 和成对 transition marker；只有单点 marker 时，控件跳转到对应状态帧。
- 组件接入要优先选择语义明确的动效资源。找不到准确匹配的动画时，保留静态 glyph 比使用泛化动效更好。

## 后续打磨

- 扩充一组项目内置、语义明确的小尺寸 Lottie 资源，并保持静态 glyph fallback 一致。
- 继续把动画入口限制在能提升可读性的控件上，避免把普通表单控件全部动画化。
- 继续扩展高 DPI、快速 resize 和主题切换下的缓存压力验证；当前已用 `lottieTintCacheSurvivesResizeAndThemeChanges` 覆盖 tint 帧不复用旧尺寸或旧主题颜色，并在 CTest 中以 `QT_SCALE_FACTOR=1.5` 单独重跑该用例。
- 视资源质量补充 NavigationView / Card 以外的 opt-in 接入点，但默认行为仍以静态、稳定、可读为先。

## 回归验证

当前 smoke test 使用 `QT_QPA_PLATFORM=offscreen` 和 `QWidget::render()` 做离屏视觉验证，覆盖：

- `lottieMetadataAndPlaybackApiFollowDocumentation`
- `lottieFallbackIconUsesTintColor`
- `lottieTintCacheSurvivesResizeAndThemeChanges`
- `animatedIconMarkerResolutionFollowsDocumentation`
- `buttonAnimatedIconsFollowForegroundTokens`
- `cardCollapseIndicatorUsesStateMarkersAndClearFallback`
- `navigationChromeAnimationsClearToStaticIcons`
