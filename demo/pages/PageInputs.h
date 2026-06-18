#pragma once

class QVBoxLayout;
class QWidget;

namespace Fluent { class FluentMainWindow; }

namespace Demo::Pages {

// fill 仅填充内容到给定布局（不含 makePage 滚动外壳），供合并页复用。
void fillInputs(QVBoxLayout *page, Fluent::FluentMainWindow *window);

QWidget *createInputsPage(Fluent::FluentMainWindow *window);

} // namespace Demo::Pages
