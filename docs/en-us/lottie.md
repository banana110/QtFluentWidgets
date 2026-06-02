# Lottie Motion

This module documents `FluentLottieWidget`, `FluentAnimatedIcon`, `FluentAnimatedButton`, plus the Lottie-enabled pieces of `FluentCard` and `FluentNavigationView`. They are backed by the `third_party/rlottie` submodule, enabling Qt Widgets applications to render Lottie JSON and, when markers are present, use WinUI-style state transitions.

## Widget List (Public Headers)

- `FluentLottieWidget` (include: `Fluent/FluentLottieWidget.h`)
- `FluentAnimatedIcon` (include: `Fluent/FluentAnimatedIcon.h`)
- `FluentAnimatedButton` (include: `Fluent/FluentAnimatedButton.h`)
- `FluentCard` collapsible indicator (include: `Fluent/FluentCard.h`)
- Optional animated icon fields on `FluentNavigationItem` (include: `Fluent/FluentNavigationView.h`)

Demo pages:

- Motion: `demo/pages/PageMotion.cpp`
- Buttons: `demo/pages/PageButtons.cpp`
- Overview: `demo/pages/PageOverview.cpp`

The Motion page starts with a Motion Role Matrix for configured/effective duration and reduced-motion checks across Hover, Focus, Popup, Selection, Navigation, Page, Toast, and WheelSnap.

---

## Dependency and Build

The project integrates `rlottie` as a Git submodule and builds it from source. Public headers expose Qt types only; `rlottie` types do not leak into the public API. After a fresh clone, initialize the submodule:

```bash
git submodule update --init --recursive
```

The root `CMakeLists.txt` uses `add_subdirectory(third_party/rlottie EXCLUDE_FROM_ALL)`, links `rlottie::rlottie` into `QtFluentWidgets`, and fails configure with a clear message if the submodule is missing.

Current build choices:

- `LOTTIE_MODULE=OFF`: build the rlottie image loader into the library and avoid an extra runtime plugin.
- `LOTTIE_TEST=OFF`: skip rlottie tests.
- `EXCLUDE_FROM_ALL`: avoid building rlottie examples; only the library target is pulled in.
- On Windows, the rlottie runtime library is copied next to the demo and Designer plugin outputs.

---

## Component Boundaries

### FluentLottieWidget

General-purpose Lottie playback widget for success feedback, loading motion, notification affordances, file transfer, status illustrations, and other regular animation. It handles JSON loading, play/pause/stop, frame seeking, marker range playback, fallback painting, single-color tinting, and basic animation metadata.

### FluentAnimatedIcon

A stateful animated icon built on `FluentLottieWidget`. It maps control states to Lottie markers, which fits WinUI AnimatedIcon-like hover / press / disabled / selected transitions.

### FluentAnimatedButton

A `QPushButton` wrapper with an internal `FluentAnimatedIcon`. It keeps normal button semantics such as `clicked` and `toggled`, while automatically coordinating hover, press, checked, and disabled states with the animation. The internal animated icon is tinted to the current foreground color, so primary buttons use white icons and disabled buttons use the disabled text color.

### FluentCard / FluentNavigationView

`FluentCard` can load a Lottie indicator for collapsible headers. Marker-aware assets use their state markers; ordinary full-cycle Lottie JSON is split at the midpoint into expand / collapse segments, and playback speed is synchronized with the card's own expand/collapse animation duration.

`FluentNavigationView` supports animated icons on `FluentNavigationItem`. The animated icon overlays the original glyph / `QIcon` and is tinted to the theme text color; if loading fails, the original static icon remains as fallback.

---

## FluentLottieWidget API

Inheritance and construction:

- `class FluentLottieWidget : public QWidget`
- Constructor: `FluentLottieWidget(QWidget *parent = nullptr)`

Loading and state:

- `load(const QString &path)`: load a `.json` Lottie file. Relative image assets use the JSON file's directory as `resourcePath`.
- `loadData(const QByteArray &json, const QString &cacheKey = {}, const QString &resourcePath = {})`: load from memory; pass the image directory when the JSON references external images.
- `setSource(const QString &path)` / `source()`: property-style loading.
- `isLoaded()` / `errorString()`: inspect loading state.
- `loaded()` / `loadFailed(const QString&)` / `sourceChanged(const QString&)`: loading signals.

Playback:

- `play()` / `pause()` / `stop()`: play, pause, or stop and return to frame 0.
- `setPlaying(bool)` / `isPlaying()`: property-style playback control.
- `setLooping(bool)` / `isLooping()`: looping playback, enabled by default.
- `setSpeed(qreal)` / `speed()`: playback speed. `1.0` is native speed and `2.0` is double speed. Values are clamped internally to `0.05` through `8.0`.
- `setCurrentFrame(int)` / `currentFrame()`: seek by frame.
- `setProgress(qreal)` / `progress()`: seek by normalized `0.0` to `1.0` progress.
- `playSegment(int startFrame, int endFrame)`: play a frame range and stop at the end.
- `playMarker(const QString &name)`: play a marker range; if the marker has no range length, jump to its first frame.
- `playingChanged(bool)` / `currentFrameChanged(int)` / `progressChanged(qreal)` / `finished()`: playback signals.

Animation metadata:

- `animationSize()`: original Lottie size.
- `totalFrames()`: total frame count.
- `frameRate()`: animation frame rate.
- `duration()`: duration in seconds.
- `markerNames()` / `hasMarker()` / `markerFrame()` / `markerEndFrame()`: top-level marker lookup.

Fallback and tint:

- `setFallbackIcon(const QIcon&)` / `fallbackIcon()`: icon painted when loading fails or no Lottie is loaded; when `tintColor` is valid, fallback icons are recolored by alpha as well so AnimatedButton / NavigationView failure fallbacks keep following the themed foreground.
- `setFallbackIconSize(const QSize&)` / `fallbackIconSize()`: fallback icon size.
- `setTintColor(const QColor&)` / `resetTintColor()` / `tintColor()`: recolor rendered frames and fallback icons by alpha.
- `tintColorChanged(const QColor&)`: tint change signal.

---

## Basic Examples

Load from a file and loop:

```cpp
#include "Fluent/FluentLottieWidget.h"

auto *animation = new Fluent::FluentLottieWidget(parent);
animation->setFixedSize(96, 96);
animation->load(QStringLiteral(":/lottie/done.json"));
animation->play();
```

Bind progress to a slider:

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

One-shot completion feedback:

```cpp
auto *done = new Fluent::FluentLottieWidget(parent);
done->load(QStringLiteral(":/lottie/done.json"));
done->setLooping(false);
done->play();
```

Adjust playback speed:

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

`setSpeed()` changes the timer cadence. It does not modify the current frame, looping mode, or markers. The setting also applies to `playSegment()` / `playMarker()`. To fit a segment into a fixed duration, compute `speed = segmentFrameCount / fps / targetSeconds`.

---

## Reduced Motion

`FluentLottieWidget` follows `ThemeManager::setAnimationsEnabled(false)`:

- `play()` does not start the timer while global animations are disabled.
- `playSegment(start, end)` jumps to `end` immediately and emits `finished()`.
- If reduced motion is turned on while a segment is playing, the widget stops and lands on the segment end frame.
- Regular looping animations pause on the current frame when reduced motion is turned on.

This means `FluentAnimatedIcon` state transitions and `FluentAnimatedButton` icon transitions resolve to their target marker frame instead of animating through intermediate frames.

---

## Theme Linkage and Tint

`FluentLottieWidget` repaints on theme changes, but it does not choose tint semantics for you. For icon-like Lottie files, map `tintColor` to a semantic color from `ThemeManager::colors()`:

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

Guidance:

- `setTintColor()` uses `QPainter::CompositionMode_SourceIn` to recolor the whole frame by alpha. It is best for line icons, single-color button icons, and status symbols.
- Avoid tinting complex illustrations, rich gradients, celebration animations, or brand artwork; it will flatten the original color design.
- `FluentAnimatedButton` already tints its internal animated icon to the button foreground color, so manual tinting is usually unnecessary.

---

## FluentAnimatedIcon

Inheritance and construction:

- `class FluentAnimatedIcon : public FluentLottieWidget`
- Constructor: `FluentAnimatedIcon(QWidget *parent = nullptr)`

Key APIs:

- `state()`: current semantic state, defaulting to `Normal`.
- `setState(const QString &state)`: switch state using an animated transition when available.
- `setState(const QString &state, bool animated)`: choose whether to play a transition.
- `setInteractive(bool)` / `isInteractive()`: automatically switch between `Normal`, `PointerOver`, and `Pressed` on the widget's own hover / press events.
- `stateChanged(const QString&)`: state change signal.

Common states:

- `Normal`
- `PointerOver`
- `Pressed`
- `Disabled`
- `Selected`

Marker resolution:

- `[PreviousState]To[NewState]_Start` + `[PreviousState]To[NewState]_End`: play that frame range.
- `[PreviousState]To[NewState]`: play or jump to the transition marker.
- `[NewState]`: jump to a state marker.
- Any `To[NewState]_End`: fallback frame for the target state.
- Numeric state: target frame number.
- Missing marker: frame 0.

Example:

```cpp
#include "Fluent/FluentAnimatedIcon.h"

auto *icon = new Fluent::FluentAnimatedIcon(parent);
icon->setFixedSize(48, 48);
icon->load(QStringLiteral(":/lottie/search.json"));
icon->setInteractive(true);
icon->setState(QStringLiteral("Normal"), false);
```

Manual state controls:

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

Inheritance and construction:

- `class FluentAnimatedButton : public QPushButton`
- Constructor: `FluentAnimatedButton(QWidget *parent = nullptr)`
- Constructor: `FluentAnimatedButton(const QString &text, QWidget *parent = nullptr)`

Key APIs:

- `setPrimary(bool)` / `isPrimary()`: primary button style.
- `loadAnimation(const QString &path)`: load the internal animated icon.
- `loadAnimationData(const QByteArray &json, const QString &cacheKey = {}, const QString &resourcePath = {})`: load the internal animated icon from memory.
- `animatedIcon()`: access the internal `FluentAnimatedIcon` for fallback icons, marker queries, tint, and related setup.
- `setAnimatedIconSize(const QSize&)` / `animatedIconSize()`: internal animated icon size, defaulting to `24x24`.
- `animationState()` / `setAnimationState()`: inspect or manually set the button animation state.
- `animationStateChanged(const QString&)`: animation state change signal.

Button state mapping:

- hover: `Normal` -> `PointerOver`
- press: `PointerOver` -> `Pressed`
- leave: return to the resting state
- disabled: use `Disabled` if the Lottie provides that marker
- checked: when the button is `setCheckable(true)`, use `Selected` if the Lottie provides that marker

Example:

```cpp
#include "Fluent/FluentAnimatedButton.h"

auto *button = new Fluent::FluentAnimatedButton(QStringLiteral("Search"), parent);
button->loadAnimation(QStringLiteral(":/lottie/search.json"));

QObject::connect(button, &QPushButton::clicked, this, [] {
    qDebug() << "Search clicked";
});
```

Primary button and icon size:

```cpp
auto *primary = new Fluent::FluentAnimatedButton(QStringLiteral("Primary Search"), parent);
primary->setPrimary(true);
primary->setAnimatedIconSize(QSize(20, 20));
primary->loadAnimation(QStringLiteral(":/lottie/search.json"));
```

Icon-only button:

```cpp
auto *iconOnly = new Fluent::FluentAnimatedButton(parent);
iconOnly->setFixedSize(40, 32);
iconOnly->setAnimatedIconSize(QSize(20, 20));
iconOnly->loadAnimation(QStringLiteral(":/lottie/search.json"));
```

---

## FluentCard and FluentNavigationView

Animated indicator for a collapsible card:

```cpp
#include "Fluent/FluentCard.h"

auto *card = new Fluent::FluentCard(parent);
card->setCollapsible(true);
card->setTitle(QStringLiteral("Advanced"));
card->loadCollapseIndicatorAnimation(QStringLiteral(":/qtfluent/demo/lottie/dropdown.json"));
```

Related APIs:

- `loadCollapseIndicatorAnimation(const QString &path)`: load the header collapse indicator from a file or qrc resource.
- `loadCollapseIndicatorAnimationData(const QByteArray &json, const QString &cacheKey = {}, const QString &resourcePath = {})`: load from memory.
- `clearCollapseIndicatorAnimation()`: remove the animated indicator and return to the static chevron.

If the animation provides `Expanded` / `Collapsed` markers, the card uses those markers first. Without markers, the card splits the timeline at the midpoint: `0~50%` plays when expanding, and `50%~100%` plays when collapsing. This fits full-cycle dropdown JSON files that animate from expanded to collapsed in one pass. During playback, the card derives `speed` from the segment frame count and Lottie fps so the indicator duration closely matches the card expand/collapse animation.

Animated NavigationView chrome buttons:

```cpp
#include "Fluent/FluentNavigationView.h"

auto *nav = new Fluent::FluentNavigationView(parent);
nav->loadPaneToggleAnimation(QStringLiteral(":/qtfluent/demo/lottie/menu.json"));
nav->loadBackButtonAnimation(QStringLiteral(":/qtfluent/demo/lottie/left-arrow.json"));
```

Related APIs:

- `loadPaneToggleAnimation(const QString &path)` / `loadPaneToggleAnimationData(...)`: replace the pane toggle hamburger button.
- `loadBackButtonAnimation(const QString &path)` / `loadBackButtonAnimationData(...)`: replace the back button icon.
- `clearPaneToggleAnimation()` / `clearBackButtonAnimation()`: return to static drawing.

Animated navigation item icon:

```cpp
#include "Fluent/FluentNavigationView.h"

Fluent::FluentNavigationItem settings;
settings.key = QStringLiteral("settings");
settings.text = QStringLiteral("Settings");
settings.iconGlyph = QString(QChar(0xE713)); // fallback if loading fails
settings.animatedIconSource = QStringLiteral(":/qtfluent/demo/lottie/setting.json");

nav->addFooterItem(settings);
```

New `FluentNavigationItem` fields:

- `animatedIconSource`: Lottie JSON file path or qrc resource.
- `animatedIconData` / `animatedIconCacheKey` / `animatedIconResourcePath`: in-memory loading path for generated or embedded assets.
- `animatedIconLooping`: whether the icon loops while hovered or selected.

Use animated item icons only when the asset clearly matches the item meaning, such as `setting.json` for a Settings entry. If no good semantic match is available, keep the static `iconGlyph` so the navigation remains readable.

---

## Asset Authoring Guidance

Regular `FluentLottieWidget` samples can use ordinary Lottie JSON and do not require markers. `FluentAnimatedIcon` and `FluentAnimatedButton` need top-level markers to produce natural state transitions.

Recommended marker names:

- Resting states: `Normal`, `PointerOver`, `Pressed`, `Disabled`, `Selected`
- Transition: `NormalToPointerOver_Start` / `NormalToPointerOver_End`
- Transition: `PointerOverToPressed_Start` / `PointerOverToPressed_End`
- Transition: `PressedToPointerOver_Start` / `PressedToPointerOver_End`
- Transition: `PointerOverToNormal_Start` / `PointerOverToNormal_End`
- Selected state: `NormalToSelected_Start` / `NormalToSelected_End`, plus `Selected`

If you only provide single-point markers such as `Normal`, `PointerOver`, and `Pressed`, the widget jumps to state frames. If you provide paired `_Start` / `_End` markers, it plays the frame range.

---

## Performance and Limits

- The current rendering path renders the current frame synchronously during widget painting and caches the current frame and tinted frame. Give animation widgets explicit sizes to avoid frequent resize invalidation.
- Small icon motion, button motion, and status feedback are the best fit. Keep the number and frame rate of large complex illustrations under control.
- For motion that does not need to run continuously, use `setLooping(false)` or call `pause()` when it is off-screen.
- `setTintColor()` is whole-frame single-color recoloring, not Lottie property-level recoloring.
- `markerNames()` reads top-level Lottie markers, not keypaths or layer names.
- `.json` Lottie files are supported; `.lottie` packages are not currently supported.
- `FluentNavigationView` and collapsible `FluentCard` support animated icons today. Other controls can still use `FluentLottieWidget` / `FluentAnimatedIcon` as standalone widgets or buttons in layouts.

---

## Related Docs

- Buttons & toggles: [buttons.md](buttons.md)
- AnimatedIcon plan: [animated-icon-plan.md](animated-icon-plan.md)
- Theme and style: [theme-style.md](theme-style.md)
