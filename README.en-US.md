# QtFluentWidgets

A Fluent Design styled widget library built on **Qt Widgets** (with a full demo app and an optional Qt Designer plugin). It provides a unified theme/color system and a set of commonly used controls.

- Languages:
  - English: README.en-US.md (this file)
  - 中文: [README.zh-CN.md](README.zh-CN.md)

## Highlights

- Qt5 / Qt6 compatible (CMake auto-detects Qt6, otherwise falls back to Qt5).
- Unified theming: `ThemeManager` + `ThemeColors`.
- Fluent-like input surfaces via `Style::paintControlSurface()` (radius/border/focus ring).
- Includes `FluentLottieWidget` / `FluentAnimatedIcon` / `FluentAnimatedButton`, backed by the `third_party/rlottie` submodule for Lottie JSON rendering and marker-based state animation.
- Adds WinUI Gallery-style controls such as `FluentProgressRing`, `FluentInfoBar`, `FluentAutoSuggestBox` / `FluentSearchBox`, `FluentFlyout` / `FluentTeachingTip`, and `FluentCommandBar` / `FluentDropDownButton` / `FluentSplitButton`.
- Includes `FluentDateRangePicker` with a dual-panel range popup and configurable prefix / suffix / separator text.
- Window-layer support includes `FluentMainWindow` title-bar slots, Windows 11-style accent border / trace animation, and menu-bar embedding into the custom title bar.
- Demo covers all widgets and includes a sidebar panel to configure the CodeEditor (including clang-format path).
- Optional Qt Designer plugin (enabled by default).

## Quick Start

### Requirements

- CMake >= 3.16
- Qt Widgets (Qt5 or Qt6)
- Qt Svg, Qt UiPlugin (Demo / Designer plugin)
- rlottie submodule (see "Get the code" below)

### Get the code (including submodules)

If you have not cloned the repository yet, use `--recursive` so Git downloads `third_party/rlottie` as well:

```bash
git clone --recursive https://github.com/Type3limit/QtFluentWidgets.git
cd QtFluentWidgets
```

If you already cloned the repository without `--recursive`, no problem. Run this once from the repository root:

```bash
git submodule update --init --recursive
```

After this, `third_party/rlottie` should contain source files instead of being empty. This step is usually needed only the first time you set up the project; run the same command again later if submodules are updated.

### Build (out-of-source recommended)

```bash
cmake -S . -B build
cmake --build build --config Release
```

If Qt is not in your default search path, set `CMAKE_PREFIX_PATH` (Windows example):

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.6.0/msvc2019_64"
cmake --build build --config Release
```

### Run the demo

- CMake target: `QtFluentDemo`
- Convenience target: `run-QtFluentDemo`

## Use in Your Project

### Option A: add_subdirectory (simplest)

```cmake
add_subdirectory(path/to/QtFluent)

target_link_libraries(your_app PRIVATE QtFluentWidgets)
```

Include headers from `include/Fluent/`:

```cpp
#include "Fluent/FluentButton.h"
#include "Fluent/FluentDateRangePicker.h"
#include "Fluent/FluentTheme.h"

using namespace Fluent;

auto *rangePicker = new FluentDateRangePicker();
rangePicker->setDateRange(QDate::currentDate(), QDate::currentDate().addDays(7));
rangePicker->setStartPrefix(QStringLiteral("From: "));
rangePicker->setSeparator(QStringLiteral("  to  "));
rangePicker->setEndPrefix(QStringLiteral("To: "));
```

## Docs (by module)

Controls are documented by module under `docs/`:

- Theme / Style: [docs/en-us/theme-style.md](docs/en-us/theme-style.md)
- Buttons & toggles: [docs/en-us/buttons.md](docs/en-us/buttons.md)
- Lottie motion: [docs/en-us/lottie.md](docs/en-us/lottie.md)
- Inputs: [docs/en-us/inputs.md](docs/en-us/inputs.md)
- Code editor: [docs/en-us/code-editor.md](docs/en-us/code-editor.md)
- Pickers: [docs/en-us/pickers.md](docs/en-us/pickers.md)
- Data views: [docs/en-us/data-views.md](docs/en-us/data-views.md)
- Containers / layout: [docs/en-us/containers-layout.md](docs/en-us/containers-layout.md)
- NavigationView: [docs/en-us/navigation-view.md](docs/en-us/navigation-view.md)
- Windows / menus / dialogs: [docs/en-us/windows-dialogs.md](docs/en-us/windows-dialogs.md)
- Qt Designer compatibility test: [docs/en-us/designer-compat-test.md](docs/en-us/designer-compat-test.md)
- Utilities: [docs/en-us/utilities.md](docs/en-us/utilities.md)

## Qt Designer Plugin

The Designer plugin is built by default and placed under `designer/` inside your build directory. Copy the built plugin into Qt Designer's plugin directory.

Disable the plugin build:

```bash
cmake -S . -B build -DFLUENT_BUILD_DESIGNER_PLUGIN=OFF
cmake --build build
```
