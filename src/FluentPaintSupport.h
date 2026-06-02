#pragma once

#include <QByteArray>
#include <QWidget>
#include <QWindow>
#include <QtGlobal>

namespace Fluent::Detail {

inline bool isOffscreenPlatform()
{
    const QByteArray platform = qgetenv("QT_QPA_PLATFORM").toLower();
    return platform.contains("offscreen");
}

inline bool canBeginWidgetPainter(const QWidget *widget)
{
    if (!widget) {
        return false;
    }
    if (!widget->testAttribute(Qt::WA_WState_InPaintEvent)) {
        return false;
    }
    if (const QWidget *windowWidget = widget->window()) {
        if (QWindow *window = windowWidget->windowHandle(); window && !window->isExposed() && !isOffscreenPlatform()) {
            return false;
        }
    }
    return true;
}

} // namespace Fluent::Detail
