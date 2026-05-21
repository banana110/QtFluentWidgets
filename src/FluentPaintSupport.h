#pragma once

#include <QWidget>
#include <QWindow>

namespace Fluent::Detail {

inline bool canBeginWidgetPainter(const QWidget *widget)
{
    if (!widget) {
        return false;
    }
    if (!widget->testAttribute(Qt::WA_WState_InPaintEvent)) {
        return false;
    }
    if (const QWidget *windowWidget = widget->window()) {
        if (QWindow *window = windowWidget->windowHandle(); window && !window->isExposed()) {
            return false;
        }
    }
    return true;
}

} // namespace Fluent::Detail
