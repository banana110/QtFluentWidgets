# AnimatedIcon Plan

Goal: add WinUI-style `AnimatedIcon` support to QtFluentWidgets so buttons, navigation items, search affordances, and status feedback can use Lottie animation while keeping the Qt Widgets / Qt5 / Qt6 surface lightweight.

Current usage documentation lives in [lottie.md](lottie.md); this document keeps the implementation split, build strategy, and follow-up plan.

## Background

WinUI `AnimatedIcon` is more than a Lottie player. Its key behavior maps control states to Lottie markers such as `NormalToPointerOver_Start` and `NormalToPointerOver_End`, which lets hover, press, and selected states use consistent transition segments.

Qt Lottie Animation is mainly a Qt Quick/QML module. `rlottie` is a cross-platform C++ Lottie renderer that can load files or raw JSON, query frame metadata, and render ARGB32 frames into caller-provided buffers, making it a practical first QWidget backend.

## Principles

- `rlottie` is integrated as the `third_party/rlottie` Git submodule, so users do not need a system package.
- Public headers expose only Qt types, not `rlottie` types.
- Build a QWidget playback primitive first, then layer WinUI state semantics on top.
- Support the common WinUI marker naming rules without trying to cover every LottieGen behavior in the first pass.
- Keep a fallback icon for failed loads or reduced-motion scenarios.

## Controls

### FluentLottieWidget

General Lottie playback widget:

- `setSource()` / `load()` for JSON files.
- `loadData()` for in-memory JSON.
- `play()` / `pause()` / `stop()`.
- `setCurrentFrame()` / `setProgress()`.
- `playSegment(startFrame, endFrame)`.
- Parse top-level Lottie `markers` and expose `markerNames()` / `hasMarker()` / `markerFrame()`.
- Paint a fallback icon when the renderer is unavailable or loading fails.

### FluentAnimatedIcon

WinUI-style state wrapper built on `FluentLottieWidget`:

- `setState(QString state, bool animated = true)`.
- Default state: `Normal`.
- Optional `interactive` mode that maps hover/press to `PointerOver` / `Pressed` / `Normal`.
- Marker resolution:
  - `[PreviousState]To[NewState]_Start` + `_End`: play the segment.
  - Only `_End` or `_Start`: jump to that frame.
  - `[PreviousState]To[NewState]`: jump to the transition marker.
  - `[NewState]`: jump to the state marker.
  - Any marker ending in `To[NewState]_End`: fallback state frame.
  - Numeric state: target frame.
  - Otherwise: frame 0.

### FluentAnimatedButton

Button wrapper built from `QPushButton` plus an internal `FluentAnimatedIcon`:

- Keeps Qt button semantics such as `clicked`, `toggled`, and `setCheckable()`.
- Paints the Fluent button surface, Primary/Secondary states, hover/press/focus.
- Switches the internal icon to `PointerOver` on hover, `Pressed` on press, and `Normal` when resting.
- For checkable buttons, uses a `Selected` marker for the checked resting state when available.
- Provides `loadAnimation()` / `loadAnimationData()` / `animatedIcon()` / `setAnimatedIconSize()`.

## CMake

`rlottie` enters the source tree through a Git submodule:

```bash
git submodule update --init --recursive
```

The root project uses `add_subdirectory(third_party/rlottie EXCLUDE_FROM_ALL)` and links `rlottie::rlottie` to `QtFluentWidgets`. Configure fails with an explicit message when the submodule is missing.

Integration rules:

- `LOTTIE_MODULE=OFF`: compile the image loader into rlottie and avoid an extra runtime plugin.
- `LOTTIE_TEST=OFF`: skip rlottie tests.
- `EXCLUDE_FROM_ALL`: avoid building rlottie examples; only the library target is pulled in by `QtFluentWidgets`.
- On Windows, copy the rlottie runtime library next to the demo and Designer plugin outputs.

## Phases

1. Add docs, rlottie submodule, source-level CMake integration, `FluentLottieWidget`, `FluentAnimatedIcon`, and `FluentAnimatedButton`.
2. Add a demo showing fallback and marker state APIs.
3. Register the controls in the Designer plugin.
4. Later integrate animated icons into `FluentIconButton`, `FluentNavigationView`, and a future `AutoSuggestBox` / `SearchBox`.

## Later Work

- Respect reduced-motion settings by jumping to target frames.
- Map a Lottie `Foreground` property to theme text/accent colors.
- Add async pre-rendering or a small frame cache for lower CPU spikes.
- Evaluate ThorVG / dotLottie for `.lottie`, theming, and state-machine support.
