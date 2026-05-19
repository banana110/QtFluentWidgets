# Lottie 动效

本模块记录 `FluentLottieWidget`、`FluentAnimatedIcon`、`FluentAnimatedButton` 以及接入了 Lottie 图标的 `FluentCard`、`FluentNavigationView` 的使用方式。它们都基于 `third_party/rlottie` 子模块实现，用于在 Qt Widgets 中播放 Lottie JSON，并支持 WinUI 风格的 marker 状态动画。

## 控件清单（对应公开头文件）

- `FluentLottieWidget`（include: `Fluent/FluentLottieWidget.h`）
- `FluentAnimatedIcon`（include: `Fluent/FluentAnimatedIcon.h`）
- `FluentAnimatedButton`（include: `Fluent/FluentAnimatedButton.h`）
- `FluentCard` 的可折叠指示器（include: `Fluent/FluentCard.h`）
- `FluentNavigationItem` 的可选动画图标字段（include: `Fluent/FluentNavigationView.h`）

Demo 页面：

- Motion / 动态：`demo/pages/PageMotion.cpp`
- Buttons / 按钮：`demo/pages/PageButtons.cpp`
- Overview / 总览：`demo/pages/PageOverview.cpp`

---

## 依赖与构建

项目通过 Git 子模块源码级集成 `rlottie`，公开头文件只暴露 Qt 类型，不暴露 `rlottie` 类型。新 clone 仓库后需要初始化子模块：

```bash
git submodule update --init --recursive
```

根 `CMakeLists.txt` 会使用 `add_subdirectory(third_party/rlottie EXCLUDE_FROM_ALL)` 引入 `rlottie::rlottie`，并链接到 `QtFluentWidgets`。如果子模块缺失，配置阶段会给出明确错误。

当前构建策略：

- `LOTTIE_MODULE=OFF`：把 rlottie 图片 loader 编进库，避免额外运行时插件。
- `LOTTIE_TEST=OFF`：不构建 rlottie 自带测试。
- `EXCLUDE_FROM_ALL`：不构建 rlottie 示例工具，只拉入库目标。
- Windows 下会把 rlottie 运行时库复制到 Demo / Designer 插件输出目录旁边。

---

## 组件边界

### FluentLottieWidget

通用 Lottie 播放控件，适合成功反馈、加载动效、通知入口、文件传输、状态插画等普通动画。它负责加载 JSON、播放/暂停/停止、定位帧、播放 marker 区间、fallback 绘制、单色 tint，以及暴露基础动画元数据。

### FluentAnimatedIcon

基于 `FluentLottieWidget` 的状态动画图标。它把控件状态映射到 Lottie marker，适合 WinUI AnimatedIcon 类似的 hover / press / disabled / selected 状态过渡。

### FluentAnimatedButton

基于 `QPushButton` 的按钮封装，内部持有一个 `FluentAnimatedIcon`。它保留 `clicked` / `toggled` 等按钮语义，同时自动把 hover、press、checked、disabled 与动画状态联动。内部动画图标会自动 tint 到当前前景色，所以 Primary 按钮上会使用白色图标，禁用态会使用禁用文本色。

### FluentCard / FluentNavigationView

`FluentCard` 可以为可折叠标题行加载 Lottie 指示器。带 marker 的资源会按状态 marker 切换；普通完整周期型 Lottie JSON 会按中点拆分为展开 / 收起两段，并自动把播放速度同步到 Card 自身的展开收起动画时长。

`FluentNavigationView` 支持在 `FluentNavigationItem` 上设置动画图标。动画图标会覆盖原 glyph / `QIcon`，并按主题文本色 tint；如果加载失败，会自动回退到原来的静态图标。

---

## FluentLottieWidget API

继承与构造：

- `class FluentLottieWidget : public QWidget`
- 构造：`FluentLottieWidget(QWidget *parent = nullptr)`

加载与状态：

- `load(const QString &path)`：从 `.json` 文件加载 Lottie。相对图片资源会以 JSON 文件所在目录作为 `resourcePath`。
- `loadData(const QByteArray &json, const QString &cacheKey = {}, const QString &resourcePath = {})`：从内存加载 Lottie；如果 JSON 引用了外部图片，传入图片所在目录。
- `setSource(const QString &path)` / `source()`：属性式加载入口。
- `isLoaded()` / `errorString()`：查询加载状态。
- `loaded()` / `loadFailed(const QString&)` / `sourceChanged(const QString&)`：加载相关信号。

播放控制：

- `play()` / `pause()` / `stop()`：播放、暂停、停止并回到第 0 帧。
- `setPlaying(bool)` / `isPlaying()`：属性式播放控制。
- `setLooping(bool)` / `isLooping()`：是否循环播放，默认循环。
- `setSpeed(qreal)` / `speed()`：播放速度，`1.0` 为原速，`2.0` 为两倍速，内部限制在 `0.05` 到 `8.0`。
- `setCurrentFrame(int)` / `currentFrame()`：定位帧。
- `setProgress(qreal)` / `progress()`：用 `0.0` 到 `1.0` 定位进度。
- `playSegment(int startFrame, int endFrame)`：播放帧区间，到结尾后停止。
- `playMarker(const QString &name)`：播放 marker 区间；如果 marker 没有区间长度，则跳转到 marker 起始帧。
- `playingChanged(bool)` / `currentFrameChanged(int)` / `progressChanged(qreal)` / `finished()`：播放相关信号。

动画元数据：

- `animationSize()`：Lottie 原始尺寸。
- `totalFrames()`：总帧数。
- `frameRate()`：帧率。
- `duration()`：时长，单位秒。
- `markerNames()` / `hasMarker()` / `markerFrame()` / `markerEndFrame()`：查询顶层 markers。

fallback 与 tint：

- `setFallbackIcon(const QIcon&)` / `fallbackIcon()`：加载失败或未加载时绘制的图标。
- `setFallbackIconSize(const QSize&)` / `fallbackIconSize()`：fallback 图标尺寸。
- `setTintColor(const QColor&)` / `resetTintColor()` / `tintColor()`：把渲染帧按 alpha 重染为单色。
- `tintColorChanged(const QColor&)`：tint 变化信号。

---

## 基础示例

从文件加载并循环播放：

```cpp
#include "Fluent/FluentLottieWidget.h"

auto *animation = new Fluent::FluentLottieWidget(parent);
animation->setFixedSize(96, 96);
animation->load(QStringLiteral(":/lottie/done.json"));
animation->play();
```

绑定进度条或滑块：

```cpp
auto *animation = new Fluent::FluentLottieWidget(parent);
animation->load(QStringLiteral(":/lottie/file_transfer.json"));
animation->setLooping(true);

auto *slider = new QSlider(Qt::Horizontal, parent);
slider->setRange(0, 1000);

QObject::connect(animation, &Fluent::FluentLottieWidget::progressChanged, slider, [slider](qreal value) {
    const QSignalBlocker blocker(slider);
    slider->setValue(qRound(value * 1000.0));
});

QObject::connect(slider, &QSlider::sliderPressed, animation, &Fluent::FluentLottieWidget::pause);
QObject::connect(slider, &QSlider::valueChanged, animation, [animation, slider](int value) {
    if (slider->isSliderDown()) {
        animation->setProgress(static_cast<qreal>(value) / 1000.0);
    }
});

animation->play();
```

一次性完成反馈：

```cpp
auto *done = new Fluent::FluentLottieWidget(parent);
done->load(QStringLiteral(":/lottie/done.json"));
done->setLooping(false);
done->play();
```

调整播放速度：

```cpp
auto *animation = new Fluent::FluentLottieWidget(parent);
animation->load(QStringLiteral(":/lottie/menu.json"));
animation->setLooping(true);
animation->setSpeed(1.5); // 1.5x
animation->play();

auto *speed = new QSlider(Qt::Horizontal, parent);
speed->setRange(25, 400); // 0.25x ~ 4.00x
speed->setValue(100);

QObject::connect(speed, &QSlider::valueChanged, animation, [animation](int value) {
    animation->setSpeed(value / 100.0);
});
```

`setSpeed()` 会改变 timer 的推进频率，不会修改当前帧、循环设置或 marker。对于 `playSegment()` / `playMarker()`，速度同样生效；如果希望某个片段在固定时长内播放完成，可以按 `speed = 片段帧数 / fps / 目标秒数` 计算。

---

## 主题联动与 tint

`FluentLottieWidget` 会在主题变化时触发重绘，但不会自动替你决定 tint 语义。对于图标型 Lottie，推荐把 `tintColor` 映射到 `ThemeManager::colors()` 中的语义色：

```cpp
#include "Fluent/FluentLottieWidget.h"
#include "Fluent/FluentTheme.h"

auto *iconMotion = new Fluent::FluentLottieWidget(parent);
iconMotion->load(QStringLiteral(":/lottie/bell.json"));

auto applyThemeTint = [iconMotion]() {
    iconMotion->setTintColor(Fluent::ThemeManager::instance().colors().accent);
};

applyThemeTint();
QObject::connect(&Fluent::ThemeManager::instance(),
                 &Fluent::ThemeManager::themeChanged,
                 iconMotion,
                 applyThemeTint);
```

使用建议：

- `setTintColor()` 使用 `QPainter::CompositionMode_SourceIn` 按 alpha 重染整帧，适合线性图标、单色按钮图标、状态符号。
- 不建议对复杂插画、渐变丰富的庆祝动效、品牌插图做 tint；这会丢失原始色彩层次。
- `FluentAnimatedButton` 已经自动把内部动画图标 tint 到按钮前景色，通常不需要手动设置。

---

## FluentAnimatedIcon

继承与构造：

- `class FluentAnimatedIcon : public FluentLottieWidget`
- 构造：`FluentAnimatedIcon(QWidget *parent = nullptr)`

关键 API：

- `state()`：当前语义状态，默认 `Normal`。
- `setState(const QString &state)`：切换状态，默认使用动画过渡。
- `setState(const QString &state, bool animated)`：可选择是否播放过渡。
- `setInteractive(bool)` / `isInteractive()`：控件自身 hover / press 时自动切换 `Normal`、`PointerOver`、`Pressed`。
- `stateChanged(const QString&)`：状态变化信号。

常用状态：

- `Normal`
- `PointerOver`
- `Pressed`
- `Disabled`
- `Selected`

marker 查找规则：

- `[PreviousState]To[NewState]_Start` + `[PreviousState]To[NewState]_End`：播放该帧区间。
- `[PreviousState]To[NewState]`：播放或跳转该 transition marker。
- `[NewState]`：跳转到状态 marker。
- 任意 `To[NewState]_End`：作为目标状态兜底帧。
- 数字 state：作为目标帧号。
- 找不到 marker 时回到第 0 帧。

示例：

```cpp
#include "Fluent/FluentAnimatedIcon.h"

auto *icon = new Fluent::FluentAnimatedIcon(parent);
icon->setFixedSize(48, 48);
icon->load(QStringLiteral(":/lottie/search.json"));
icon->setInteractive(true);
icon->setState(QStringLiteral("Normal"), false);
```

手动控制状态：

```cpp
auto *normal = new Fluent::FluentButton(QStringLiteral("Normal"), parent);
auto *pressed = new Fluent::FluentButton(QStringLiteral("Pressed"), parent);

QObject::connect(normal, &QPushButton::clicked, icon, [icon]() {
    icon->setState(QStringLiteral("Normal"));
});
QObject::connect(pressed, &QPushButton::clicked, icon, [icon]() {
    icon->setState(QStringLiteral("Pressed"));
});
```

---

## FluentAnimatedButton

继承与构造：

- `class FluentAnimatedButton : public QPushButton`
- 构造：`FluentAnimatedButton(QWidget *parent = nullptr)`
- 构造：`FluentAnimatedButton(const QString &text, QWidget *parent = nullptr)`

关键 API：

- `setPrimary(bool)` / `isPrimary()`：Primary 按钮样式。
- `loadAnimation(const QString &path)`：加载按钮内部动画图标。
- `loadAnimationData(const QByteArray &json, const QString &cacheKey = {}, const QString &resourcePath = {})`：从内存加载内部动画图标。
- `animatedIcon()`：访问内部 `FluentAnimatedIcon`，可设置 fallback icon、查询 marker、设置 tint 等。
- `setAnimatedIconSize(const QSize&)` / `animatedIconSize()`：设置内部动画图标尺寸，默认 `24x24`。
- `animationState()` / `setAnimationState()`：读取或手动设置按钮动画状态。
- `animationStateChanged(const QString&)`：动画状态变化信号。

按钮状态映射：

- hover：`Normal` -> `PointerOver`
- press：`PointerOver` -> `Pressed`
- leave：回到 resting 状态
- disabled：如果 Lottie 提供 `Disabled` marker，使用 `Disabled`
- checked：如果按钮 `setCheckable(true)` 且 Lottie 提供 `Selected` marker，使用 `Selected`

示例：

```cpp
#include "Fluent/FluentAnimatedButton.h"

auto *button = new Fluent::FluentAnimatedButton(QStringLiteral("Search"), parent);
button->loadAnimation(QStringLiteral(":/lottie/search.json"));

QObject::connect(button, &QPushButton::clicked, this, [] {
    qDebug() << "Search clicked";
});
```

Primary 与图标尺寸：

```cpp
auto *primary = new Fluent::FluentAnimatedButton(QStringLiteral("Primary Search"), parent);
primary->setPrimary(true);
primary->setAnimatedIconSize(QSize(20, 20));
primary->loadAnimation(QStringLiteral(":/lottie/search.json"));
```

图标按钮：

```cpp
auto *iconOnly = new Fluent::FluentAnimatedButton(parent);
iconOnly->setFixedSize(40, 32);
iconOnly->setAnimatedIconSize(QSize(20, 20));
iconOnly->loadAnimation(QStringLiteral(":/lottie/search.json"));
```

---

## FluentCard 与 FluentNavigationView

可折叠 Card 的动画指示器：

```cpp
#include "Fluent/FluentCard.h"

auto *card = new Fluent::FluentCard(parent);
card->setCollapsible(true);
card->setTitle(QStringLiteral("Advanced"));
card->loadCollapseIndicatorAnimation(QStringLiteral(":/qtfluent/demo/lottie/dropdown.json"));
```

相关 API：

- `loadCollapseIndicatorAnimation(const QString &path)`：从路径或 qrc 资源加载标题行折叠指示器。
- `loadCollapseIndicatorAnimationData(const QByteArray &json, const QString &cacheKey = {}, const QString &resourcePath = {})`：从内存加载。
- `clearCollapseIndicatorAnimation()`：清除动画指示器，回到静态 chevron。

如果动画包含 `Expanded` / `Collapsed` marker，Card 会优先按 marker 切换状态。没有 marker 时，Card 会把动画按中点拆成两段：`0~50%` 用于展开，`50%~100%` 用于收起，适合“展开再收起”的完整周期型 dropdown JSON。播放时会根据当前片段帧数和 Lottie fps 自动调整 `speed`，让指示器动效时长尽量匹配 Card 本身的展开/收起动画。

NavigationView 自身 chrome 按钮（菜单 / 返回）的动画：

```cpp
#include "Fluent/FluentNavigationView.h"

auto *nav = new Fluent::FluentNavigationView(parent);
nav->loadPaneToggleAnimation(QStringLiteral(":/qtfluent/demo/lottie/menu.json"));
nav->loadBackButtonAnimation(QStringLiteral(":/qtfluent/demo/lottie/left-arrow.json"));
```

相关 API：

- `loadPaneToggleAnimation(const QString &path)` / `loadPaneToggleAnimationData(...)`：替换 pane 顶部的汉堡菜单按钮图标。
- `loadBackButtonAnimation(const QString &path)` / `loadBackButtonAnimationData(...)`：替换返回按钮图标。
- `clearPaneToggleAnimation()` / `clearBackButtonAnimation()`：回到静态绘制。

导航项动画图标：

```cpp
#include "Fluent/FluentNavigationView.h"

Fluent::FluentNavigationItem settings;
settings.key = QStringLiteral("settings");
settings.text = QStringLiteral("Settings");
settings.iconGlyph = QString(QChar(0xE713)); // 加载失败时的 fallback
settings.animatedIconSource = QStringLiteral(":/qtfluent/demo/lottie/setting.json");

nav->addFooterItem(settings);
```

`FluentNavigationItem` 新增字段：

- `animatedIconSource`：Lottie JSON 路径或 qrc 资源。
- `animatedIconData` / `animatedIconCacheKey` / `animatedIconResourcePath`：内存加载入口，适合动态生成或嵌入式资源。
- `animatedIconLooping`：图标处于 hover / selected 时是否循环播放。

导航项动画应当只用于语义明确匹配的入口，例如设置项使用 `setting.json`。如果没有找到与文字含义足够一致的 Lottie，建议保留静态 `iconGlyph`，避免动效削弱导航可读性。

---

## 资源制作建议

普通 `FluentLottieWidget` 示例可以使用常规 Lottie JSON，不要求 marker。`FluentAnimatedIcon` 和 `FluentAnimatedButton` 想获得自然的状态过渡，需要设计师在 Lottie 顶层 markers 中写入状态与 transition 名称。

推荐 marker 命名：

- Resting 状态：`Normal`、`PointerOver`、`Pressed`、`Disabled`、`Selected`
- 状态过渡：`NormalToPointerOver_Start` / `NormalToPointerOver_End`
- 状态过渡：`PointerOverToPressed_Start` / `PointerOverToPressed_End`
- 状态过渡：`PressedToPointerOver_Start` / `PressedToPointerOver_End`
- 状态过渡：`PointerOverToNormal_Start` / `PointerOverToNormal_End`
- 选中态：`NormalToSelected_Start` / `NormalToSelected_End`，以及 `Selected`

如果只提供 `Normal`、`PointerOver`、`Pressed` 这类单点 marker，组件会跳转状态帧；如果提供 `_Start` / `_End` 成对 marker，组件会播放区间。

---

## 性能与限制

- 当前渲染路径在控件 paint 时同步渲染当前帧，并缓存当前帧与 tint 后帧。建议给动画控件设置明确尺寸，避免频繁 resize 造成缓存失效。
- 小尺寸图标、按钮动效、状态反馈最适合该控件；大面积复杂插画应控制数量和帧率。
- 不需要持续播放的动效建议 `setLooping(false)` 或在不可见时 `pause()`。
- `setTintColor()` 是整帧单色重染，不是 Lottie 属性级改色。
- `markerNames()` 读取的是 Lottie 顶层 markers，不是 keypath 或图层名。
- 目前支持 `.json` Lottie；暂不支持 `.lottie` 包格式。
- 目前 `FluentNavigationView` 和可折叠 `FluentCard` 已支持动画图标；其他控件仍可把 `FluentLottieWidget` / `FluentAnimatedIcon` 作为独立 QWidget 或按钮放进布局。

---

## 相关文档

- 按钮与开关：[buttons.md](buttons.md)
- AnimatedIcon 实施计划：[animated-icon-plan.md](animated-icon-plan.md)
- 主题与色板：[theme-style.md](theme-style.md)
