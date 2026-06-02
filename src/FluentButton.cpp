#include "Fluent/FluentButton.h"
#include "Fluent/FluentMotion.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"
#include "FluentButtonVisuals_p.h"

#include <QAbstractAnimation>
#include <QEvent>
#include <QPainter>
#include <QStyleOptionButton>
#include <QVariantAnimation>

namespace Fluent {

namespace {

QSize logicalPixmapSize(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        return {};
    }

    const qreal dpr = pixmap.devicePixelRatio();
    return QSize(qRound(pixmap.width() / dpr), qRound(pixmap.height() / dpr));
}

} // namespace

FluentButton::FluentButton(QWidget *parent)
    : QPushButton(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    setAutoDefault(false);
    setMinimumHeight(Style::metrics().height);

    m_hoverAnim = new QVariantAnimation(this);
    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    connect(m_hoverAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_hoverLevel = value.toReal();
        update();
    });

    m_pressAnim = new QVariantAnimation(this);
    FluentMotion::configure(m_pressAnim, FluentMotionRole::Press);
    connect(m_pressAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_pressLevel = value.toReal();
        update();
    });

    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentButton::applyTheme);
    connect(this, &QAbstractButton::toggled, this, QOverload<>::of(&FluentButton::update));
}

FluentButton::FluentButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    setAutoDefault(false);
    setMinimumHeight(Style::metrics().height);

    m_hoverAnim = new QVariantAnimation(this);
    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    connect(m_hoverAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_hoverLevel = value.toReal();
        update();
    });

    m_pressAnim = new QVariantAnimation(this);
    FluentMotion::configure(m_pressAnim, FluentMotionRole::Press);
    connect(m_pressAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_pressLevel = value.toReal();
        update();
    });

    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentButton::applyTheme);
    connect(this, &QAbstractButton::toggled, this, QOverload<>::of(&FluentButton::update));
}

bool FluentButton::isPrimary() const
{
    return m_primary;
}

void FluentButton::setPrimary(bool primary)
{
    if (m_primary == primary) {
        return;
    }

    m_primary = primary;
    applyTheme();
}

qreal FluentButton::hoverLevel() const
{
    return m_hoverLevel;
}

void FluentButton::setHoverLevel(qreal value)
{
    m_hoverLevel = qBound(0.0, value, 1.0);
    update();
}

qreal FluentButton::pressLevel() const
{
    return m_pressLevel;
}

void FluentButton::setPressLevel(qreal value)
{
    m_pressLevel = qBound(0.0, value, 1.0);
    update();
}

void FluentButton::changeEvent(QEvent *event)
{
    QPushButton::changeEvent(event);

    if (event->type() == QEvent::EnabledChange) {
        applyTheme();
    }
}

void FluentButton::applyTheme()
{
    const bool hoverRunning = m_hoverAnim && m_hoverAnim->state() == QAbstractAnimation::Running;
    const bool pressRunning = m_pressAnim && m_pressAnim->state() == QAbstractAnimation::Running;
    const QVariant hoverEnd = m_hoverAnim ? m_hoverAnim->endValue() : QVariant();
    const QVariant pressEnd = m_pressAnim ? m_pressAnim->endValue() : QVariant();

    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    FluentMotion::configure(m_pressAnim, FluentMotionRole::Press);
    if (hoverRunning && m_hoverAnim->duration() <= 0) {
        m_hoverAnim->stop();
        m_hoverLevel = qBound<qreal>(0.0, hoverEnd.toReal(), 1.0);
    }
    if (pressRunning && m_pressAnim->duration() <= 0) {
        m_pressAnim->stop();
        m_pressLevel = qBound<qreal>(0.0, pressEnd.toReal(), 1.0);
    }
    update();
}

void FluentButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    const auto &colors = ThemeManager::instance().colors();
    const auto &tokens = ThemeManager::instance().tokens();

    const bool checked = isCheckable() && isChecked();
    const ButtonVisuals::StateColors state =
        ButtonVisuals::resolve(colors, tokens, m_primary, checked, isEnabled());
    const QColor fill = ButtonVisuals::fillForState(state, m_hoverLevel, m_pressLevel);
    const QColor textColor = ButtonVisuals::textForState(state.text, m_pressLevel, isEnabled());

    QPainter painter(this);
    if (!painter.isActive()) {
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    const auto m = Style::metrics();
    QRectF rect = QRectF(this->rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    const qreal radius = m.radius;

    ButtonVisuals::paintRoundedControl(painter, rect, radius, fill, state.border, state.bottomBorder);

    // Fluent-like checked detail (without indicator bar): add a subtle inner highlight so
    // the selected state is still obvious on accent-filled primary buttons.
    if (checked && m_primary && isEnabled()) {
        QColor inner = tokens.onAccent;
        inner.setAlpha(115);
        painter.setPen(QPen(inner, 1.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect.adjusted(1.0, 1.0, -1.0, -1.0), radius - 1, radius - 1);
    }

    if (hasFocus() && isEnabled()) {
        QColor focus = tokens.accent.base;
        focus.setAlpha(230);
        painter.setPen(QPen(focus, 2.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(rect.adjusted(1, 1, -1, -1), radius - 1, radius - 1);
    }

    // Draw text
    painter.setPen(textColor);
    painter.setFont(this->font());
    
    // Calculate text and icon layout
    QRect contentRect = rect.toRect();
    
    if (!icon().isNull()) {
        const QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
        const QIcon::State state = (isCheckable() && isChecked()) ? QIcon::On : QIcon::Off;
        const QSize requestedSize = iconSize().isValid()
                                        ? iconSize()
                                        : QSize(m.height - 12, m.height - 12);
        const QSize actualSize = icon().actualSize(requestedSize, mode, state);
        const QPixmap pixmap = icon().pixmap(actualSize, mode, state);
        const QSize drawSize = logicalPixmapSize(pixmap);

        if (pixmap.isNull() || !drawSize.isValid()) {
            painter.drawText(contentRect, Qt::AlignCenter | Qt::AlignVCenter | Qt::TextShowMnemonic, text());
            return;
        }

        if (text().isEmpty()) {
            const QRect iconRect(contentRect.center().x() - drawSize.width() / 2,
                                 contentRect.center().y() - drawSize.height() / 2,
                                 drawSize.width(),
                                 drawSize.height());
            painter.drawPixmap(iconRect, pixmap);
            return;
        }

        const int gap = 8;
        const int textWidth = fontMetrics().horizontalAdvance(text());
        const int totalWidth = drawSize.width() + gap + textWidth;
        const int startX = contentRect.center().x() - totalWidth / 2;

        const QRect iconRect(startX,
                             contentRect.center().y() - drawSize.height() / 2,
                             drawSize.width(),
                             drawSize.height());
        painter.drawPixmap(iconRect, pixmap);

        const int textLeft = startX + drawSize.width() + gap;
        const QRect textRect(textLeft,
                             contentRect.top(),
                             contentRect.right() - textLeft + 1,
                             contentRect.height());
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, text());
    } else {
        // No icon - center text
        painter.drawText(contentRect, Qt::AlignCenter | Qt::AlignVCenter | Qt::TextShowMnemonic, text());
    }
}

void FluentButton::enterEvent(FluentEnterEvent *event)
{
    QPushButton::enterEvent(event);
    startHoverAnimation(1.0);
}

void FluentButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    startHoverAnimation(0.0);
}

void FluentButton::mousePressEvent(QMouseEvent *event)
{
    startPressAnimation(1.0);
    QPushButton::mousePressEvent(event);
}

void FluentButton::mouseReleaseEvent(QMouseEvent *event)
{
    startPressAnimation(0.0);
    QPushButton::mouseReleaseEvent(event);
}

void FluentButton::startHoverAnimation(qreal endValue)
{
    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    m_hoverAnim->stop();
    if (m_hoverAnim->duration() <= 0) {
        setHoverLevel(endValue);
        return;
    }
    m_hoverAnim->setStartValue(m_hoverLevel);
    m_hoverAnim->setEndValue(endValue);
    m_hoverAnim->start();
}

void FluentButton::startPressAnimation(qreal endValue)
{
    FluentMotion::configure(m_pressAnim, FluentMotionRole::Press);
    m_pressAnim->stop();
    if (m_pressAnim->duration() <= 0) {
        setPressLevel(endValue);
        return;
    }
    m_pressAnim->setStartValue(m_pressLevel);
    m_pressAnim->setEndValue(endValue);
    m_pressAnim->start();
}

} // namespace Fluent
