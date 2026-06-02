#pragma once

#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"

#include <QPalette>
#include <QWidget>

namespace Fluent::Detail {

inline QPalette fluentViewPalette(const QPalette &base, const ThemeColors &colors)
{
    const FluentThemeTokens tokens = Theme::tokens(colors);
    const QColor disabledBase =
        Style::mix(tokens.neutral.card, tokens.neutral.fillSecondary, tokens.dark ? 0.30 : 0.46);
    const QColor disabledStroke =
        Style::mix(tokens.neutral.strokeSubtle, colors.disabledText, tokens.dark ? 0.28 : 0.18);

    QPalette palette = base;
    palette.setColor(QPalette::Window, tokens.neutral.card);
    palette.setColor(QPalette::Base, tokens.neutral.card);
    palette.setColor(QPalette::AlternateBase, tokens.neutral.cardHover);
    palette.setColor(QPalette::WindowText, colors.text);
    palette.setColor(QPalette::Text, colors.text);
    palette.setColor(QPalette::ButtonText, colors.text);
    palette.setColor(QPalette::Mid, tokens.neutral.strokeSubtle);
    palette.setColor(QPalette::Light, tokens.neutral.cardHover);
    palette.setColor(QPalette::Dark, tokens.neutral.fillTertiary);
    palette.setColor(QPalette::Highlight, tokens.accent.base);
    palette.setColor(QPalette::HighlightedText, tokens.onAccent);
    palette.setColor(QPalette::PlaceholderText, colors.disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Window, disabledBase);
    palette.setColor(QPalette::Disabled, QPalette::Base, disabledBase);
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase, tokens.neutral.cardHover);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, colors.disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Text, colors.disabledText);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, colors.disabledText);
    palette.setColor(QPalette::Disabled, QPalette::Mid, disabledStroke);
    palette.setColor(QPalette::Disabled, QPalette::Highlight, disabledBase);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, colors.disabledText);
    return palette;
}

inline void applyFluentViewPalette(QWidget *widget, QWidget *viewport, const ThemeColors &colors)
{
    if (!widget) {
        return;
    }

    const QPalette palette = fluentViewPalette(widget->palette(), colors);
    widget->setPalette(palette);
    if (viewport) {
        viewport->setPalette(palette);
    }
}

} // namespace Fluent::Detail
