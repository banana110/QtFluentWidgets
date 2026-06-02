#include "Fluent/FluentRadioButton.h"
#include "Fluent/FluentMotion.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"

#include <QAbstractAnimation>
#include <QEvent>
#include <QPainter>
#include <QVariantAnimation>

namespace Fluent {

FluentRadioButton::FluentRadioButton(QWidget *parent)
    : QRadioButton(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    setFocusPolicy(Qt::StrongFocus);

    m_hoverAnim = new QVariantAnimation(this);
    connect(m_hoverAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_hoverLevel = value.toReal();
        update();
    });

    m_focusAnim = new QVariantAnimation(this);
    connect(m_focusAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_focusLevel = value.toReal();
        update();
    });

    m_checkAnim = new QVariantAnimation(this);
    connect(m_checkAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_checkLevel = value.toReal();
        update();
    });
    connect(this, &QRadioButton::toggled, this, [this](bool checked) {
        FluentMotion::configure(m_checkAnim, FluentMotionRole::Selection);
        m_checkAnim->stop();
        const qreal target = checked ? 1.0 : 0.0;
        if (m_checkAnim->duration() <= 0) {
            m_checkLevel = target;
            update();
        } else {
            m_checkAnim->setStartValue(m_checkLevel);
            m_checkAnim->setEndValue(target);
            m_checkAnim->start();
        }
    });
    m_checkLevel = isChecked() ? 1.0 : 0.0;

    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentRadioButton::applyTheme);
}

FluentRadioButton::FluentRadioButton(const QString &text, QWidget *parent)
    : QRadioButton(text, parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    setFocusPolicy(Qt::StrongFocus);

    m_hoverAnim = new QVariantAnimation(this);
    connect(m_hoverAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_hoverLevel = value.toReal();
        update();
    });

    m_focusAnim = new QVariantAnimation(this);
    connect(m_focusAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_focusLevel = value.toReal();
        update();
    });

    m_checkAnim = new QVariantAnimation(this);
    connect(m_checkAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_checkLevel = value.toReal();
        update();
    });
    connect(this, &QRadioButton::toggled, this, [this](bool checked) {
        FluentMotion::configure(m_checkAnim, FluentMotionRole::Selection);
        m_checkAnim->stop();
        const qreal target = checked ? 1.0 : 0.0;
        if (m_checkAnim->duration() <= 0) {
            m_checkLevel = target;
            update();
        } else {
            m_checkAnim->setStartValue(m_checkLevel);
            m_checkAnim->setEndValue(target);
            m_checkAnim->start();
        }
    });
    m_checkLevel = isChecked() ? 1.0 : 0.0;

    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentRadioButton::applyTheme);
}

QSize FluentRadioButton::sizeHint() const
{
    const QFontMetrics fm(font());
    const int circle = 18;
    const int leftPad = 2;
    const int gap = 8;
    const int rightPad = 6;
    const int w = leftPad + circle + gap + fm.horizontalAdvance(text()) + rightPad;
    const int h = qMax(circle + 4, fm.height() + 8);
    return {w, h};
}

QSize FluentRadioButton::minimumSizeHint() const
{
    return sizeHint();
}

qreal FluentRadioButton::hoverLevel() const
{
    return m_hoverLevel;
}

void FluentRadioButton::setHoverLevel(qreal value)
{
    m_hoverLevel = qBound(0.0, value, 1.0);
    update();
}

qreal FluentRadioButton::focusLevel() const
{
    return m_focusLevel;
}

void FluentRadioButton::setFocusLevel(qreal value)
{
    m_focusLevel = qBound(0.0, value, 1.0);
    update();
}

void FluentRadioButton::changeEvent(QEvent *event)
{
    QRadioButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        applyTheme();
    }
}

void FluentRadioButton::applyTheme()
{
    const bool snapHover = m_hoverAnim &&
        m_hoverAnim->state() == QAbstractAnimation::Running &&
        FluentMotion::duration(FluentMotionRole::Hover) <= 0;
    const bool snapFocus = m_focusAnim &&
        m_focusAnim->state() == QAbstractAnimation::Running &&
        FluentMotion::duration(FluentMotionRole::Focus) <= 0;
    const bool snapCheck = m_checkAnim &&
        m_checkAnim->state() == QAbstractAnimation::Running &&
        FluentMotion::duration(FluentMotionRole::Selection) <= 0;
    const qreal hoverTarget = m_hoverAnim ? m_hoverAnim->endValue().toReal() : m_hoverLevel;
    const qreal focusTarget = m_focusAnim ? m_focusAnim->endValue().toReal() : m_focusLevel;
    const qreal checkTarget = m_checkAnim ? m_checkAnim->endValue().toReal() : m_checkLevel;

    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    FluentMotion::configure(m_focusAnim, FluentMotionRole::Focus);
    FluentMotion::configure(m_checkAnim, FluentMotionRole::Selection);
    if (snapHover) {
        m_hoverAnim->stop();
        setHoverLevel(hoverTarget);
    }
    if (snapFocus) {
        m_focusAnim->stop();
        setFocusLevel(focusTarget);
    }
    if (snapCheck) {
        m_checkAnim->stop();
        m_checkLevel = qBound<qreal>(0.0, checkTarget, 1.0);
    }
    update();
}

void FluentRadioButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    const auto &colors = ThemeManager::instance().colors();
    const auto tokens = ThemeManager::instance().tokens();

    QPainter painter(this);
    if (!painter.isActive()) {
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Hover background (full row highlight)
    if (m_hoverLevel > 0.01 && isEnabled()) {
        painter.setPen(Qt::NoPen);
        QColor hoverBg = Style::withAlpha(tokens.neutral.fillSecondary, static_cast<int>(90 * m_hoverLevel));
        painter.setBrush(hoverBg);
        const QRectF hoverRect = QRectF(this->rect()).adjusted(1.0, 2.0, -1.0, -2.0);
        painter.drawRoundedRect(hoverRect, 6.0, 6.0);
    }

    const qreal size = 18.0;
    const qreal leftPad = 2.0;
    QRectF circleRect(leftPad, (height() - size) / 2.0, size, size);
    QRectF textRect = QRectF(this->rect()).adjusted(static_cast<int>(leftPad + size + 8), 0, -4, 0);
    
    // Adjust for border sharpness
    QRectF drawRect = circleRect.adjusted(0.5, 0.5, -0.5, -0.5);

    QColor border = tokens.neutral.strokeStrong;
    QColor fill = tokens.neutral.card;
    if (!isEnabled()) {
        border = tokens.neutral.strokeSubtle;
        fill = Style::mix(tokens.neutral.card, tokens.neutral.background, tokens.dark ? 0.48 : 0.34);
        drawRect = circleRect;
    }

    painter.setPen(QPen(border, 1));
    painter.setBrush(fill);
    painter.drawEllipse(drawRect);

    // Focus ring (keyboard focus)
    if (isEnabled() && m_focusLevel > 0.01) {
        QColor focus = tokens.accent.base;
        focus.setAlphaF(0.9 * m_focusLevel);
        painter.setPen(QPen(focus, 2.0));
        painter.setBrush(Qt::NoBrush);
        const QRectF ring = circleRect.adjusted(1.0, 1.0, -1.0, -1.0);
        painter.drawEllipse(ring);
    }

    // Indicator hover is handled by the full-row highlight above.

    if (m_checkLevel > 0.01) {
        const QColor selectionColor = isEnabled() ? tokens.accent.base : colors.disabledText;
        if (isEnabled()) {
            QColor accentBorder = selectionColor;
            accentBorder.setAlphaF(m_checkLevel);
            painter.setPen(QPen(accentBorder, 1));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(drawRect);
        }
        
        // Dot scaling
        qreal dotSize = 10.0 * m_checkLevel; // Max dot size approx 10
        if (dotSize > 0.5) {
            QColor dotColor = selectionColor;
            dotColor.setAlphaF(m_checkLevel);
            painter.setPen(Qt::NoPen);
            painter.setBrush(dotColor);
            
            QRectF dotRect(
                circleRect.center().x() - dotSize / 2.0,
                circleRect.center().y() - dotSize / 2.0,
                dotSize, dotSize
            );
            painter.drawEllipse(dotRect);
        }
    }

    painter.setPen(isEnabled() ? colors.text : colors.disabledText);
    const QString elided = fontMetrics().elidedText(text(), Qt::ElideRight, static_cast<int>(textRect.width()));
    painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elided);
}

void FluentRadioButton::enterEvent(FluentEnterEvent *event)
{
    QRadioButton::enterEvent(event);
    startHoverAnimation(1.0);
}

void FluentRadioButton::leaveEvent(QEvent *event)
{
    QRadioButton::leaveEvent(event);
    startHoverAnimation(0.0);
}

void FluentRadioButton::focusInEvent(QFocusEvent *event)
{
    QRadioButton::focusInEvent(event);
    startFocusAnimation(1.0);
}

void FluentRadioButton::focusOutEvent(QFocusEvent *event)
{
    QRadioButton::focusOutEvent(event);
    startFocusAnimation(0.0);
}

void FluentRadioButton::startHoverAnimation(qreal endValue)
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

void FluentRadioButton::startFocusAnimation(qreal endValue)
{
    FluentMotion::configure(m_focusAnim, FluentMotionRole::Focus);
    m_focusAnim->stop();
    if (m_focusAnim->duration() <= 0) {
        setFocusLevel(endValue);
        return;
    }
    m_focusAnim->setStartValue(m_focusLevel);
    m_focusAnim->setEndValue(endValue);
    m_focusAnim->start();
}

} // namespace Fluent
