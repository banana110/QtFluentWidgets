#include "Fluent/FluentGroupBox.h"
#include "Fluent/FluentFramePainter.h"
#include "Fluent/FluentTheme.h"

#include <QEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>

namespace Fluent {

namespace {
constexpr int kHorizontalPadding = 14;
constexpr int kTopPadding = 38;
constexpr int kBottomPadding = 14;
constexpr int kTitleTop = 10;
constexpr int kTitleGap = 8;
constexpr int kTitleHeight = 22;
constexpr int kIndicatorSize = 16;

void initializeGroupBox(FluentGroupBox *box)
{
    box->setAttribute(Qt::WA_StyledBackground, false);
    box->setAutoFillBackground(false);
    box->setContentsMargins(kHorizontalPadding, kTopPadding, kHorizontalPadding, kBottomPadding);
}

void paintCheckIndicator(QPainter &p,
                         const QRectF &rect,
                         const ThemeColors &colors,
                         const FluentThemeTokens &tokens,
                         bool checked,
                         bool enabled)
{
    QColor fill = enabled
        ? tokens.neutral.card
        : Style::mix(tokens.neutral.card, tokens.neutral.background, tokens.dark ? 0.48 : 0.35);
    QColor border = enabled
        ? tokens.neutral.strokeSubtle
        : Style::mix(tokens.neutral.strokeSubtle, colors.disabledText, tokens.dark ? 0.28 : 0.18);

    if (checked && enabled) {
        fill = tokens.accent.base;
        border = fill;
    }

    p.save();
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(QPen(border, 1.0));
    p.setBrush(fill);
    p.drawRoundedRect(rect.adjusted(0.5, 0.5, -0.5, -0.5), 3.5, 3.5);

    if (checked) {
        QColor mark = enabled ? tokens.onAccent : colors.disabledText;
        QPainterPath path;
        path.moveTo(rect.left() + 4.0, rect.top() + 8.2);
        path.lineTo(rect.left() + 7.0, rect.top() + 11.0);
        path.lineTo(rect.left() + 12.4, rect.top() + 5.2);
        QPen pen(mark, 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);
    }

    p.restore();
}
} // namespace

FluentGroupBox::FluentGroupBox(QWidget *parent)
    : QGroupBox(parent)
{
    initializeGroupBox(this);
    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentGroupBox::applyTheme);
}

FluentGroupBox::FluentGroupBox(const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    initializeGroupBox(this);
    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentGroupBox::applyTheme);
}

void FluentGroupBox::changeEvent(QEvent *event)
{
    QGroupBox::changeEvent(event);
    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::FontChange) {
        applyTheme();
    }
}

void FluentGroupBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    const auto &manager = ThemeManager::instance();
    const auto &colors = manager.colors();
    const auto &tokens = manager.tokens();

    QPainter p(this);
    if (!p.isActive()) {
        return;
    }

    FluentSurfaceSpec surface;
    surface.level = FluentSurfaceLevel::Card;
    surface.radius = tokens.radius.card;
    surface.enabled = isEnabled();
    surface.borderWidth = 1.0;
    surface.borderInset = 0.5;
    paintFluentSurface(p, QRectF(rect()), colors, surface);

    const QString groupTitle = title();
    if (groupTitle.isEmpty() && !isCheckable()) {
        return;
    }

    p.setRenderHint(QPainter::Antialiasing, true);

    int textX = kHorizontalPadding;
    if (isCheckable()) {
        const QRect checkRect = checkBoxRect();
        paintCheckIndicator(p, QRectF(checkRect), colors, tokens, isChecked(), isEnabled());
        textX = checkRect.right() + 1 + kTitleGap;
    }

    if (!groupTitle.isEmpty()) {
        QFont titleFont = font();
        titleFont.setWeight(QFont::DemiBold);
        p.setFont(titleFont);
        QColor textColor = isEnabled() ? colors.text : colors.disabledText;
        p.setPen(textColor);
        QRect textRect = titleRect();
        textRect.setLeft(textX);
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, groupTitle);
    }
}

void FluentGroupBox::applyTheme()
{
    update();
}

QRect FluentGroupBox::titleRect() const
{
    return QRect(kHorizontalPadding, kTitleTop, width() - (kHorizontalPadding * 2), kTitleHeight);
}

QRect FluentGroupBox::checkBoxRect() const
{
    const int indicatorWidth = kIndicatorSize;
    const int indicatorHeight = kIndicatorSize;
    const int y = kTitleTop + ((kTitleHeight - indicatorHeight) / 2);
    return QRect(kHorizontalPadding, y, indicatorWidth, indicatorHeight);
}

} // namespace Fluent
