# Qt Designer 兼容性测试

本文记录 `QtFluentWidgetsDesignerCompatTest` 的目的、问题成因和覆盖范围。它不是普通的“菜单栏是否显示”测试，而是专门防止 `FluentMainWindow` 与 Qt Designer / uic 生成代码配合时再次出现崩溃、菜单丢失或 `ui->menubar` 指针失效。

## 为什么要有这个测试

issue #9 暴露了一个典型场景：用户在 Qt Designer 中把主窗口、菜单栏和菜单提升为 Fluent 控件，然后在业务代码中调用 `ui->setupUi(this)`，有时还会再调用一次 `setFluentMenuBar(ui->menubar)`。

这个场景必须用真实 `.ui` 文件测试，因为问题发生在 uic 生成代码的静态类型、调用顺序和 Qt 内部 `QMainWindowLayout` 管理逻辑之间。手写 C++ 模拟很容易漏掉关键细节。

## 问题成因

uic 对 `QMainWindow` 的生成顺序大致是：

```cpp
centralwidget = new QWidget(MainWindow);
MainWindow->setCentralWidget(centralwidget);
menubar = new QMenuBar(MainWindow);
menuFile = new QMenu(menubar);
MainWindow->setMenuBar(menubar);
statusbar = new QStatusBar(MainWindow);
MainWindow->setStatusBar(statusbar);
menubar->addAction(menuFile->menuAction());
```

这里有几个容易踩中的点：

- `QMainWindow::setMenuBar()` 不是虚函数。如果 `.ui` 根节点没有提升，uic 会生成 `setupUi(QMainWindow*)`，`MainWindow->setMenuBar(menubar)` 会静态绑定到 Qt 原生实现，而不是 `FluentMainWindow::setMenuBar()`。因此 Designer 推荐用法必须先提升根窗口。
- uic 会先调用 `setMenuBar(menubar)`，再把 `menuFile->menuAction()` 加到 menubar。只在 `setMenuBar()` 时拷贝一次 action 会丢掉后追加的菜单。
- 同样地，如果 `.ui` 根节点没有提升，`setStatusBar(statusbar)` 也会静态走 `QMainWindow::setStatusBar()`，把库不支持的原生 statusbar 放回 Qt 的 mainwindow layout。
- `FluentMainWindow` 使用 Qt 的 native menu slot 承载自定义标题栏。此前为了兼容未提升根窗口的路径，把标题栏宿主做成了 `QMenuBar` 兼容对象；这会在 promoted `.ui` 中造成两个菜单栏对象：一个原生 `QMenuBar` 宿主和一个真正的 `FluentMenuBar`。
- 如果普通 `QMenuBar` 的 action 被同时挂在隐藏 source menubar 和可见 `FluentMenuBar` 上，Qt 的菜单栏布局/样式状态也可能在显示窗口时进入不稳定状态。

## 修复策略

当前实现做了几件事来让这条路径稳定：

- 标题栏宿主保持为普通 `QWidget`，并延迟到真正需要时才创建，避免 promoted `.ui` 中出现“原生菜单栏宿主 + Fluent 菜单栏”的双菜单。
- promoted 场景下，`ui->menubar` 本身就是 `FluentMenuBar`，`setupUi()` 已经完成安装；再次调用 `setFluentMenuBar(ui->menubar)` 是安全的 no-op。
- `FluentMainWindow::setMenuBar(QMenuBar*)` 对普通 `QMenuBar` 采用兼容领养策略：创建可见的 `FluentMenuBar`，保留原 `ui->menubar`，隐藏它，并把 uic 后续追加的 action 迁移到 `FluentMenuBar`，避免同一个 action 同时属于两个 menubar。
- 对通过 `QMainWindow::setStatusBar()` 静态绕过进来的原生 statusbar，会在事件循环下一拍移出 native layout 并删除，保持 `FluentMainWindow` “不承载原生 statusbar/toolbars” 的约束。
- 根窗口未提升时，uic 会绕过太多 `FluentMainWindow` 的隐藏方法；这条路径只作为防御性兼容，不作为文档推荐用法。

## 测试覆盖

测试目标：`QtFluentWidgetsDesignerCompatTest`，默认不构建，需要显式启用 `FLUENT_BUILD_TESTS`。

测试 fixture：

- `tests/ui/DesignerPromotedMainWindow.ui`
  - 根窗口：`Fluent::FluentMainWindow`
  - menubar：`Fluent::FluentMenuBar`
  - menu：`Fluent::FluentMenu`
  - 覆盖用户在 Designer 中提升控件后的推荐路径，并额外模拟重复调用 `setFluentMenuBar(ui->menubar)`。
- `tests/ui/DesignerCompatMainWindow.ui`
  - 根窗口：`Fluent::FluentMainWindow`
  - menubar 仍是 `QMenuBar`
  - 覆盖“根窗口已提升，但 menubar/menu 仍是原生 Qt 类型”时的兼容领养路径。

断言内容：

- `setupUi()` 不崩溃。
- `FluentMainWindow::fluentMenuBar()` 最终可用。
- uic 后追加的 `menuFile` / `menuView` action 会出现在可见的 `FluentMenuBar`。
- promoted fixture 中只有一个可见 menubar，`QMainWindow::menuWidget()` 不再是 `QMenuBar`。
- 点击 promoted fixture 的 `File` 后只会出现一个非原生 popup，不会同时出现 Qt 原生 `QMenu` 和 Fluent popup。
- 普通 `QMenuBar` fixture 中，隐藏 source menubar 不保留 action；action 会迁移到可见的 `FluentMenuBar`。
- 删除窗口后 central widget、source menubar、fluent menubar 不残留悬空对象。

## 如何运行

自动测试：

```bash
cmake -S . -B build -DFLUENT_BUILD_TESTS=ON
cmake --build build --target QtFluentWidgetsDesignerCompatTest --config Debug
ctest --test-dir build -C Debug --output-on-failure -R QtFluentWidgetsDesignerCompatTest
```

可视化运行：

```bash
cmake --build build --target run-QtFluentWidgetsDesignerCompatTest --config Debug
```

可视化模式会让 promoted fixture 真实 `show()` 出来，这是 issue #9 对应的推荐/实际场景。compat fixture 只保留非可视断言，用来证明原生 `QMenuBar` 的 action 迁移逻辑仍然有效。默认会一直停留，直到你手动关闭窗口。也可以直接运行 exe 并控制停留时间：

```bash
QTFLUENT_DESIGNER_COMPAT_TEST_VISIBLE=1 QTFLUENT_DESIGNER_COMPAT_TEST_VISIBLE_MS=0 ./QtFluentWidgetsDesignerCompatTest
```

`QTFLUENT_DESIGNER_COMPAT_TEST_VISIBLE_MS=0` 表示窗口会一直停留，直到你手动关闭。
