# 图标

## 控件清单

- `FluentIcon`（include: `Fluent/FluentIcon.h`）
- `FluentIconType`：内置图标枚举
- `FluentIconOptions`：颜色、透明度、自动主题色与反色选项

Demo 页面：Motion（`demo/pages/PageMotion.cpp`，合并了 `demo/pages/PageIcons.cpp` 的图标画廊与动效示例）与 Overview。图标部分顶部提供 Icon State Matrix，可横向检查 normal、accent、onAccent 与 disabled 图标色。

## 用途

`FluentIcon` 是质感升级中的静态图标层，为导航、按钮、菜单、状态提示、命令栏和小型 chrome 控件提供统一的 24px outline 风格。

图标优先使用 `resources/icons/` 中的内置 SVG 资源；`FluentIcon` 首次使用时会注册图标 qrc，因此库作为静态 target 被消费时也能读取 SVG 资源。如果资源缺失或不可用，会回退到内置 `QPainter` 绘制路径，避免控件完全失去图标。

## 基础用法

```cpp
#include "Fluent/FluentIcon.h"

auto *action = new QAction(QStringLiteral("Search"), parent);
action->setIcon(Fluent::FluentIcon::icon(Fluent::FluentIconType::Search));

Fluent::FluentIconOptions options;
options.reversed = true; // 在 primary/accent 表面上使用主题 onAccent 颜色
auto primaryIcon = Fluent::FluentIcon::icon(Fluent::FluentIconType::Save, options);
```

自绘控件中可以直接绘制：

```cpp
Fluent::FluentIconOptions options;
options.color = palette().color(QPalette::WindowText);
options.autoTheme = false;

Fluent::FluentIcon::paintIcon(&painter,
                              Fluent::FluentIconType::Settings,
                              iconRect,
                              options);
```

## 主题与状态语义

- 默认 `autoTheme=true` 会解析为当前主题文本色。
- `QIcon::Disabled` 会解析为 `ThemeManager::colors().disabledText`。
- `reversed=true` 会解析为主题 `onAccent` token，用于 primary/accent 表面。
- 显式设置 `options.color` 时会保留该色；禁用态只降低 alpha，不改变色相。
- `autoTheme=false` 但没有显式 `options.color` 时，会回退到当前主题文本色 / 禁用文本色，而不是固定黑色。
- `options.opacity` 会继续乘到最终颜色透明度上，适合弱化 chrome 图标。

## 内置图标集

当前内置 SVG 集覆盖导航、命令、状态、数据、布局、选择器和窗口场景，例如 `Home`、`Menu`、`Back`、`Search`、`Settings`、`Add`、`Delete`、`Save`、`Info`、`Success`、`Warning`、`Calendar`、`Data`、`Layout`、`Window`、`More` 等。

可以在 Icons demo 页面先查看状态色矩阵，再查看完整图标矩阵；点击图标可复制对应枚举名。

## 质量门禁

`QtFluentWidgetsVisualSmokeTest::fluentIconsResolveThemeAndStateColors` 会验证每个内置图标资源存在、能渲染出可见像素、随 light/dark 主题文本色变化、在 reversed/primary 场景使用 `onAccent`，未显式给色的手动模式不会回退到固定黑色，并且显式颜色图标在 disabled 状态下降低 alpha。
