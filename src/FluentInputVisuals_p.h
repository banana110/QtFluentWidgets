#pragma once

#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"

#include <QPainter>
#include <QPainterPath>
#include <QRectF>
#include <QtGlobal>

namespace Fluent {
namespace InputVisuals {

inline void paintFocusRing(QPainter &painter, const QRectF &rect, const ThemeColors &colors, qreal focusLevel)
{
    if (!painter.isActive()) {
        return;
    }

    focusLevel = qBound<qreal>(0.0, focusLevel, 1.0);
    if (focusLevel <= 0.0) {
        return;
    }

    const auto tokens = Theme::tokens(colors);
    QColor accent = tokens.accent.base;
    accent.setAlpha(qBound(0, int(230.0 * focusLevel), 230));

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(accent, 2.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(Style::roundedRectPath(rect.adjusted(1.0, 1.0, -1.0, -1.0),
                                            qMax<qreal>(0.0, tokens.radius.control - 1.0)));
    painter.restore();
}

inline QColor inputStroke(const ThemeColors &colors, bool enabled)
{
    const auto tokens = Theme::tokens(colors);
    return enabled ? tokens.neutral.strokeSubtle
                   : Style::mix(tokens.neutral.strokeSubtle, colors.disabledText, tokens.dark ? 0.28 : 0.18);
}

inline QColor inputDividerStroke(const ThemeColors &colors, bool enabled)
{
    return inputStroke(colors, enabled);
}

inline void paintTrailingDivider(QPainter &painter,
                                 const QRect &chromeRect,
                                 const ThemeColors &colors,
                                 bool enabled,
                                 int inset = 8)
{
    if (!painter.isActive() || chromeRect.isEmpty()) {
        return;
    }

    const int clampedInset = qBound(0, inset, qMax(0, chromeRect.height() / 2 - 1));
    painter.save();
    painter.setPen(QPen(inputDividerStroke(colors, enabled), 1.0));
    painter.drawLine(QPointF(chromeRect.left() + 0.5, chromeRect.top() + clampedInset),
                     QPointF(chromeRect.left() + 0.5, chromeRect.bottom() - clampedInset));
    painter.restore();
}

} // namespace InputVisuals
} // namespace Fluent
