# Icons

## Components

- `FluentIcon` (include: `Fluent/FluentIcon.h`)
- `FluentIconType`: built-in icon identifiers
- `FluentIconOptions`: color, opacity, auto-theme, and reversed rendering options

Demo page: Icons (`demo/pages/PageIcons.cpp`) and Overview. The Icons page starts with an Icon State Matrix for normal, accent, onAccent, and disabled color checks.

## Purpose

`FluentIcon` is the static icon layer for the visual-quality work. It provides a consistent 24px outline icon set for navigation, buttons, menus, status messages, command bars, and small chrome controls.

Icons prefer bundled SVG resources from `resources/icons/`. `FluentIcon` registers the icon qrc on first use, which keeps SVG resources available when the library is consumed as a static target. If a resource is missing or invalid, the implementation falls back to the built-in `QPainter` drawing path so controls can still render.

## Basic Usage

```cpp
#include "Fluent/FluentIcon.h"

auto *action = new QAction(QStringLiteral("Search"), parent);
action->setIcon(Fluent::FluentIcon::icon(Fluent::FluentIconType::Search));

Fluent::FluentIconOptions options;
options.reversed = true; // use the theme onAccent color on primary/accent surfaces
auto primaryIcon = Fluent::FluentIcon::icon(Fluent::FluentIconType::Save, options);
```

For custom-painted widgets:

```cpp
Fluent::FluentIconOptions options;
options.color = palette().color(QPalette::WindowText);
options.autoTheme = false;

Fluent::FluentIcon::paintIcon(&painter,
                              Fluent::FluentIconType::Settings,
                              iconRect,
                              options);
```

## Theme And State Behavior

- Default `autoTheme=true` resolves to the current theme text color.
- `QIcon::Disabled` resolves to `ThemeManager::colors().disabledText`.
- `reversed=true` resolves to the theme `onAccent` token for primary/accent surfaces.
- An explicit `options.color` is respected; disabled mode lowers alpha instead of changing the hue.
- If `autoTheme=false` is used without an explicit `options.color`, the fallback is the current theme text / disabled text color instead of fixed black.
- `options.opacity` multiplies the resolved color opacity for subtle chrome.

## Built-In Set

The current built-in SVG set includes navigation, command, status, data, layout, picker, and window icons such as `Home`, `Menu`, `Back`, `Search`, `Settings`, `Add`, `Delete`, `Save`, `Info`, `Success`, `Warning`, `Calendar`, `Data`, `Layout`, `Window`, and `More`.

Use the Icons demo page to inspect the state-color matrix first, then the full icon matrix. Click any icon to copy its enum name.

## Quality Gate

`QtFluentWidgetsVisualSmokeTest::fluentIconsResolveThemeAndStateColors` verifies that every built-in icon resource exists, renders visible pixels, changes with light/dark theme text colors, uses `onAccent` for reversed/primary rendering, avoids fixed-black fallback when manual mode has no explicit color, and lowers alpha for disabled explicit-color icons.
