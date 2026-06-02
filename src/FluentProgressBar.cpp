#include "Fluent/FluentProgressBar.h"
#include "Fluent/FluentMotion.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"

#include <QAbstractAnimation>
#include <QEvent>
#include <QPainter>
#include <QPropertyAnimation>

namespace Fluent {

FluentProgressBar::FluentProgressBar(QWidget *parent)
    : QProgressBar(parent)
{
    setTextVisible(true);
    setMinimumHeight(22);

    m_valueAnim = new QPropertyAnimation(this, "displayValue", this);

    m_displayValue = value();

    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentProgressBar::applyTheme);
    connect(this, &QProgressBar::valueChanged, this, [this](int newValue) {
        FluentMotion::configure(m_valueAnim, FluentMotionRole::Selection);
        m_valueAnim->stop();
        const qreal target = static_cast<qreal>(newValue);
        if (m_valueAnim->duration() <= 0) {
            setDisplayValue(target);
        } else {
            m_valueAnim->setStartValue(m_displayValue);
            m_valueAnim->setEndValue(target);
            m_valueAnim->start();
        }
    });
}

FluentProgressBar::TextPosition FluentProgressBar::textPosition() const
{
    return m_textPosition;
}

void FluentProgressBar::setTextPosition(TextPosition pos)
{
    if (m_textPosition == pos) {
        return;
    }
    m_textPosition = pos;
    setTextVisible(pos != TextPosition::None);
    update();
}

QColor FluentProgressBar::textColor() const
{
    return m_textColor;
}

void FluentProgressBar::setTextColor(const QColor &color)
{
    m_textColor = color;
    update();
}

qreal FluentProgressBar::displayValue() const
{
    return m_displayValue;
}

void FluentProgressBar::setDisplayValue(qreal value)
{
    m_displayValue = value;
    update();
}

void FluentProgressBar::changeEvent(QEvent *event)
{
    QProgressBar::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        applyTheme();
    }
}

void FluentProgressBar::applyTheme()
{
    const bool snapValue = m_valueAnim &&
        m_valueAnim->state() == QAbstractAnimation::Running &&
        FluentMotion::duration(FluentMotionRole::Selection) <= 0;
    const qreal target = m_valueAnim ? m_valueAnim->endValue().toReal() : m_displayValue;

    FluentMotion::configure(m_valueAnim, FluentMotionRole::Selection);
    if (snapValue) {
        m_valueAnim->stop();
        setDisplayValue(target);
    }
    update();
}

void FluentProgressBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    const auto &colors = ThemeManager::instance().colors();
    const auto tokens = ThemeManager::instance().tokens();

    QPainter painter(this);
    if (!painter.isActive()) {
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int radius = 2; // Fluent Design usually has smaller corner radius for thin bars
    QRectF rect = QRectF(this->rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    qreal barHeight = 4.0;
    const qreal textAreaH = (m_textPosition == TextPosition::None) ? 0.0 : qMax<qreal>(0.0, rect.height() - barHeight - 6.0);
    const qreal barY = rect.top() + textAreaH + 2.0;
    const qreal inset = 12.0;
    QRectF trackRect(rect.left() + inset, barY, qMax<qreal>(0.0, rect.width() - inset * 2.0), barHeight);
    
    // Draw full track background
    painter.setPen(Qt::NoPen);
    QColor track = isEnabled()
        ? Style::mix(tokens.neutral.strokeSubtle, tokens.neutral.fillSecondary, tokens.dark ? 0.36 : 0.28)
        : tokens.neutral.strokeSubtle;
    painter.setBrush(track);
    painter.drawRoundedRect(trackRect, radius, radius);

    const int minVal = minimum();
    const int maxVal = maximum();
    const qreal range = qMax(1, maxVal - minVal);
    const qreal progress = (m_displayValue - minVal) / range;
    const qreal fillWidth = trackRect.width() * qBound(0.0, progress, 1.0);

    if (fillWidth > 0) {
        QRectF fillRect = trackRect;
        fillRect.setWidth(fillWidth);
        QColor fill = tokens.accent.base;
        if (!isEnabled()) {
            fill = Style::mix(tokens.neutral.strokeStrong, tokens.accent.base, tokens.dark ? 0.22 : 0.28);
            fill.setAlpha(172);
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(fill);
        painter.drawRoundedRect(fillRect, radius, radius);
    }
    
    if (m_textPosition != TextPosition::None && isTextVisible() && textAreaH >= 12.0) {
        QRectF textRect(rect.left() + inset, rect.top(), rect.width() - inset * 2.0, textAreaH);

        Qt::Alignment align = Qt::AlignVCenter;
        switch (m_textPosition) {
        case TextPosition::Left:
            align |= Qt::AlignLeft;
            break;
        case TextPosition::Center:
            align |= Qt::AlignHCenter;
            break;
        case TextPosition::Right:
            align |= Qt::AlignRight;
            break;
        case TextPosition::None:
            break;
        }

        const QColor tc = m_textColor.isValid() ? m_textColor : (isEnabled() ? colors.text : colors.disabledText);
        painter.setPen(tc);
        painter.drawText(textRect.toRect(), align, QString("%1%").arg(static_cast<int>(progress * 100)));
    }
}

} // namespace Fluent
