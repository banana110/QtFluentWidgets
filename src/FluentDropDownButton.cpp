#include "Fluent/FluentDropDownButton.h"

#include "Fluent/FluentMenu.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"

#include <QMenu>
#include <QPainter>

namespace Fluent {

namespace {

constexpr int kHorizontalPadding = 12;
constexpr int kArrowSlotWidth = 30;
constexpr int kTextArrowGap = 6;

QSize logicalPixmapSize(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        return {};
    }

    const qreal dpr = pixmap.devicePixelRatio();
    return QSize(qRound(pixmap.width() / dpr), qRound(pixmap.height() / dpr));
}

} // namespace

FluentDropDownButton::FluentDropDownButton(QWidget *parent)
    : FluentButton(parent)
{
    setMinimumWidth(86);
    connect(this, &QPushButton::clicked, this, &FluentDropDownButton::showMenu);
}

FluentDropDownButton::FluentDropDownButton(const QString &text, QWidget *parent)
    : FluentButton(text, parent)
{
    setMinimumWidth(qMax(86, fontMetrics().horizontalAdvance(text) + kHorizontalPadding * 2 + kArrowSlotWidth + kTextArrowGap));
    connect(this, &QPushButton::clicked, this, &FluentDropDownButton::showMenu);
}

QMenu *FluentDropDownButton::menu() const
{
    return m_menu.data();
}

void FluentDropDownButton::setMenu(QMenu *menu)
{
    if (menu && !menu->parent()) {
        menu->setParent(this);
    }
    m_menu = menu;
}

QAction *FluentDropDownButton::addAction(const QString &text)
{
    if (!menu()) {
        setMenu(new FluentMenu(this));
    }
    return menu()->addAction(text);
}

QAction *FluentDropDownButton::addAction(const QIcon &icon, const QString &text)
{
    if (!menu()) {
        setMenu(new FluentMenu(this));
    }
    return menu()->addAction(icon, text);
}

QSize FluentDropDownButton::sizeHint() const
{
    QSize size = QPushButton::sizeHint();
    size.rwidth() += kArrowSlotWidth + kTextArrowGap;
    size.rheight() = qMax(size.height(), Style::metrics().height);
    return size;
}

void FluentDropDownButton::showMenu()
{
    if (!m_menu) {
        return;
    }

    const QPoint pos = mapToGlobal(QPoint(0, height() + 4));
    if (auto *fluentMenu = qobject_cast<FluentMenu *>(m_menu.data())) {
        fluentMenu->popup(pos);
    } else {
        m_menu->popup(pos);
    }
}

void FluentDropDownButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    const auto &colors = ThemeManager::instance().colors();
    const bool checked = isCheckable() && isChecked();

    QColor base;
    QColor hover;
    QColor pressed;
    QColor border;
    QColor textColor;

    if (isPrimary()) {
        base = checked ? colors.accent.darker(125) : colors.accent;
        hover = base.lighter(118);
        pressed = base.darker(118);
        border = base.darker(110);
        textColor = QColor(QStringLiteral("#FFFFFF"));
    } else {
        const QColor accentTint = Style::mix(colors.surface, colors.accent, 0.12);
        base = checked ? accentTint : colors.surface;
        hover = checked ? Style::mix(accentTint, colors.accent, 0.10)
                        : Style::mix(colors.surface, colors.hover, 0.88);
        pressed = checked ? Style::mix(accentTint, colors.accent, 0.18)
                          : Style::mix(colors.surface, colors.pressed, 0.92);
        border = checked ? Style::mix(colors.border, colors.accent, 0.85) : colors.border;
        textColor = checked ? Style::mix(colors.text, colors.accent, 0.82) : colors.text;
    }

    QColor fill = Style::mix(base, hover, hoverLevel());
    fill = Style::mix(fill, pressed, pressLevel());

    if (!isEnabled()) {
        fill = Style::mix(colors.surface, colors.hover, 0.45);
        border = Style::mix(colors.border, colors.disabledText, 0.25);
        textColor = colors.disabledText;
    }

    QPainter p(this);
    if (!p.isActive()) {
        return;
    }
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const auto metrics = Style::metrics();
    const QRectF buttonRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    const qreal radius = metrics.radius;

    p.setPen(QPen(border, 1.0));
    p.setBrush(fill);
    p.drawRoundedRect(buttonRect, radius, radius);

    if (checked && isPrimary() && isEnabled()) {
        p.setPen(QPen(QColor(255, 255, 255, 115), 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(buttonRect.adjusted(1.0, 1.0, -1.0, -1.0), radius - 1, radius - 1);
    }

    if (hasFocus() && isEnabled()) {
        QColor focus = colors.focus;
        focus.setAlpha(230);
        p.setPen(QPen(focus, 2.0));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(buttonRect.adjusted(1, 1, -1, -1), radius - 1, radius - 1);
    }

    const QRect contentRect = buttonRect.toRect().adjusted(kHorizontalPadding, 0, -(kArrowSlotWidth + kTextArrowGap), 0);
    p.setPen(textColor);
    p.setFont(font());

    bool contentDrawn = false;
    if (!icon().isNull()) {
        const QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
        const QIcon::State state = (isCheckable() && isChecked()) ? QIcon::On : QIcon::Off;
        const QSize requestedSize = iconSize().isValid()
                                        ? iconSize()
                                        : QSize(metrics.height - 12, metrics.height - 12);
        const QSize actualSize = icon().actualSize(requestedSize, mode, state);
        const QPixmap pixmap = icon().pixmap(actualSize, mode, state);
        const QSize drawSize = logicalPixmapSize(pixmap);

        if (!pixmap.isNull() && drawSize.isValid()) {
            if (text().isEmpty()) {
                const QRect iconRect(contentRect.center().x() - drawSize.width() / 2,
                                     contentRect.center().y() - drawSize.height() / 2,
                                     drawSize.width(),
                                     drawSize.height());
                p.drawPixmap(iconRect, pixmap);
                contentDrawn = true;
            } else {
                const int gap = 8;
                const int textWidth = fontMetrics().horizontalAdvance(text());
                const int totalWidth = drawSize.width() + gap + textWidth;
                const int startX = qMax(contentRect.left(), contentRect.center().x() - totalWidth / 2);
                const QRect iconRect(startX,
                                     contentRect.center().y() - drawSize.height() / 2,
                                     drawSize.width(),
                                     drawSize.height());
                p.drawPixmap(iconRect, pixmap);

                const QRect textRect(startX + drawSize.width() + gap,
                                     contentRect.top(),
                                     contentRect.right() - startX - drawSize.width() - gap + 1,
                                     contentRect.height());
                const QString label = fontMetrics().elidedText(text(), Qt::ElideRight, textRect.width());
                p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, label);
                contentDrawn = true;
            }
        }
    }

    if (!contentDrawn) {
        const QString label = fontMetrics().elidedText(text(), Qt::ElideRight, contentRect.width());
        p.drawText(contentRect, Qt::AlignCenter | Qt::AlignVCenter | Qt::TextShowMnemonic, label);
    }

    const QPointF center(width() - kArrowSlotWidth / 2.0, height() / 2.0 + 0.5);
    Style::drawChevronDown(p, center, textColor, 7.0, 1.6);
}

} // namespace Fluent
