#pragma once

class QVBoxLayout;
class QWidget;

namespace Fluent { class FluentMainWindow; }

namespace Demo::Pages {

// fill 仅填充内容到给定布局（不含 makePage 滚动外壳），供合并页复用。
void fillAngleControls(QVBoxLayout *page, Fluent::FluentMainWindow *window);

QWidget *createAngleControlsPage(Fluent::FluentMainWindow *window);

} // namespace Demo::Pages

