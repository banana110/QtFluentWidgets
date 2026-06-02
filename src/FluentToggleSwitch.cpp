#include "Fluent/FluentToggleSwitch.h"
#include "Fluent/FluentMotion.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"

#include <QAbstractAnimation>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QKeyEvent>

namespace Fluent {

FluentToggleSwitch::FluentToggleSwitch(QWidget *parent)
    : QWidget(parent)
{
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    setFocusPolicy(Qt::StrongFocus);

    m_progressAnim = new QPropertyAnimation(this, "progress", this);

    m_hoverAnim = new QPropertyAnimation(this, "hoverLevel", this);

    m_focusAnim = new QPropertyAnimation(this, "focusLevel", this);

    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentToggleSwitch::applyTheme);
    syncEnabledState();
}

FluentToggleSwitch::FluentToggleSwitch(const QString &text, QWidget *parent)
    : QWidget(parent)
    , m_text(text)
{
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    setFocusPolicy(Qt::StrongFocus);

    m_progressAnim = new QPropertyAnimation(this, "progress", this);

    m_hoverAnim = new QPropertyAnimation(this, "hoverLevel", this);

    m_focusAnim = new QPropertyAnimation(this, "focusLevel", this);

    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentToggleSwitch::applyTheme);
    syncEnabledState();
}

bool FluentToggleSwitch::isChecked() const
{
    return m_checked;
}

void FluentToggleSwitch::setChecked(bool checked)
{
    if (m_checked == checked) {
        return;
    }

    m_checked = checked;
    FluentMotion::configure(m_progressAnim, FluentMotionRole::Selection);
    m_progressAnim->stop();
    const qreal target = m_checked ? 1.0 : 0.0;
    if (m_progressAnim->duration() <= 0) {
        setProgress(target);
    } else {
        m_progressAnim->setStartValue(m_progress);
        m_progressAnim->setEndValue(target);
        m_progressAnim->start();
    }
    emit toggled(m_checked);
}

void FluentToggleSwitch::setText(const QString &text)
{
    m_text = text;
    update();
}

QSize FluentToggleSwitch::sizeHint() const
{
    const auto m = Style::metrics();
    const QFontMetrics fm(font());
    const int trackWidth = 40;
    const int gap = 10;
    const int rightPad = 6;
    const int textW = m_text.isEmpty() ? 0 : fm.horizontalAdvance(m_text);
    const int w = trackWidth + (m_text.isEmpty() ? 0 : (gap + textW)) + rightPad;
    return QSize(qMax(120, w), m.height);
}

QSize FluentToggleSwitch::minimumSizeHint() const
{
    return sizeHint();
}

qreal FluentToggleSwitch::progress() const
{
    return m_progress;
}

void FluentToggleSwitch::setProgress(qreal value)
{
    m_progress = qBound(0.0, value, 1.0);
    update();
}

qreal FluentToggleSwitch::hoverLevel() const
{
    return m_hoverLevel;
}

void FluentToggleSwitch::setHoverLevel(qreal value)
{
    m_hoverLevel = qBound(0.0, value, 1.0);
    update();
}

qreal FluentToggleSwitch::focusLevel() const
{
    return m_focusLevel;
}

void FluentToggleSwitch::setFocusLevel(qreal value)
{
    m_focusLevel = qBound(0.0, value, 1.0);
    update();
}

void FluentToggleSwitch::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        syncEnabledState();
        applyTheme();
    }
}

void FluentToggleSwitch::applyTheme()
{
    const bool snapProgress = m_progressAnim &&
        m_progressAnim->state() == QAbstractAnimation::Running &&
        FluentMotion::duration(FluentMotionRole::Selection) <= 0;
    const bool snapHover = m_hoverAnim &&
        m_hoverAnim->state() == QAbstractAnimation::Running &&
        FluentMotion::duration(FluentMotionRole::Hover) <= 0;
    const bool snapFocus = m_focusAnim &&
        m_focusAnim->state() == QAbstractAnimation::Running &&
        FluentMotion::duration(FluentMotionRole::Focus) <= 0;
    const qreal progressTarget = m_progressAnim ? m_progressAnim->endValue().toReal() : m_progress;
    const qreal hoverTarget = m_hoverAnim ? m_hoverAnim->endValue().toReal() : m_hoverLevel;
    const qreal focusTarget = m_focusAnim ? m_focusAnim->endValue().toReal() : m_focusLevel;

    FluentMotion::configure(m_progressAnim, FluentMotionRole::Selection);
    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    FluentMotion::configure(m_focusAnim, FluentMotionRole::Focus);
    if (snapProgress) {
        m_progressAnim->stop();
        setProgress(progressTarget);
    }
    if (snapHover) {
        m_hoverAnim->stop();
        setHoverLevel(hoverTarget);
    }
    if (snapFocus) {
        m_focusAnim->stop();
        setFocusLevel(focusTarget);
    }
    update();
}

void FluentToggleSwitch::syncEnabledState()
{
    setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
    if (!isEnabled()) {
        if (m_hoverAnim) {
            m_hoverAnim->stop();
        }
        if (m_focusAnim) {
            m_focusAnim->stop();
        }
        m_hoverLevel = 0.0;
        m_focusLevel = 0.0;
        update();
    }
}

void FluentToggleSwitch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    const auto &colors = ThemeManager::instance().colors();
    const auto tokens = ThemeManager::instance().tokens();

    QPainter painter(this);
    if (!painter.isActive()) {
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const auto m = Style::metrics();

    // Fluent Design: 40x20 track
    const qreal trackWidth = 40.0;
    const qreal trackHeight = 20.0;
    const qreal radius = trackHeight / 2.0;
    const qreal x = 2.0;
    const qreal y = (height() - trackHeight) / 2.0;

    // Hover background (full row highlight)
    if (m_hoverLevel > 0.01 && isEnabled()) {
        painter.setPen(Qt::NoPen);
        QColor hoverBg = tokens.neutral.cardHover;
        hoverBg.setAlphaF(qBound<qreal>(0.0, (tokens.dark ? 0.56 : 0.46) * m_hoverLevel, 1.0));
        painter.setBrush(hoverBg);
        const QRectF hoverRect = QRectF(this->rect()).adjusted(1.0, 2.0, -1.0, -2.0);
        painter.drawRoundedRect(hoverRect, 6.0, 6.0);
    }

    // Track colors
    QColor trackColor;
    if (!isEnabled()) {
        trackColor = Style::mix(tokens.neutral.card, tokens.neutral.fillSecondary, tokens.dark ? 0.58 : 0.46);
    } else if (m_checked) {
        trackColor = tokens.accent.base;
    } else {
        trackColor = Style::mix(tokens.neutral.stroke, tokens.neutral.fillSecondary, tokens.dark ? 0.38 : 0.25);
    }

    // Focus ring
    if (isEnabled() && m_focusLevel > 0.01) {
        QColor focus = tokens.accent.base;
        focus.setAlphaF(0.9 * m_focusLevel);
        painter.setPen(QPen(focus, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);
        const QRectF ring(x + 1.5, y + 1.5, trackWidth - 3.0, trackHeight - 3.0);
        painter.drawRoundedRect(ring, (trackHeight - 2.0) / 2.0, (trackHeight - 2.0) / 2.0);
    }

    // Draw track
    painter.setPen(Qt::NoPen);
    painter.setBrush(trackColor);
    painter.drawRoundedRect(QRectF(x, y, trackWidth, trackHeight), radius, radius);

    // Knob parameters - Islands style
    const qreal knobSize = 14.0;
    const qreal knobPadding = 3.0;
    
    // Calculate knob position
    const qreal knobMinX = x + knobPadding;
    const qreal knobMaxX = x + trackWidth - knobPadding - knobSize;
    const qreal knobX = knobMinX + (knobMaxX - knobMinX) * m_progress;
    const qreal knobY = y + (trackHeight - knobSize) / 2.0;

    // Draw hover ring
    if (m_hoverLevel > 0.01 && isEnabled()) {
        QColor hoverRing = tokens.accent.base;
        hoverRing.setAlphaF(m_hoverLevel * 0.3);
        painter.setPen(Qt::NoPen);
        painter.setBrush(hoverRing);
        const qreal expandSize = 2.0 * m_hoverLevel;
        painter.drawEllipse(QRectF(knobX - expandSize, knobY - expandSize, 
                                   knobSize + expandSize * 2, knobSize + expandSize * 2));
    }

    QColor knobFill;
    QColor knobBorder;
    if (!isEnabled()) {
        knobFill = Style::mix(tokens.neutral.card, tokens.neutral.background, tokens.dark ? 0.48 : 0.34);
        knobBorder = tokens.neutral.strokeSubtle;
    } else if (m_checked) {
        knobFill = tokens.onAccent;
        knobBorder = Style::withAlpha(tokens.onAccent, 180);
    } else {
        knobFill = tokens.dark
            ? Style::mix(tokens.neutral.cardHover, colors.text, 0.10)
            : tokens.neutral.layer;
        knobBorder = tokens.neutral.strokeStrong;
    }

    // Draw knob with token-derived surface and border.
    painter.setBrush(knobFill);
    painter.setPen(QPen(knobBorder, 1.0));
    painter.drawEllipse(QRectF(knobX, knobY, knobSize, knobSize));

    // Draw text label
    if (!m_text.isEmpty()) {
        painter.setPen(isEnabled() ? colors.text : colors.disabledText);
        painter.setFont(this->font());
        const QRect textRect(static_cast<int>(x + trackWidth + 10), 0, width() - static_cast<int>(x + trackWidth + 10), height());
        const QString elided = fontMetrics().elidedText(m_text, Qt::ElideRight, textRect.width());
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elided);
    }
}

void FluentToggleSwitch::mousePressEvent(QMouseEvent *event)
{
    if (!isEnabled()) {
        QWidget::mousePressEvent(event);
        return;
    }
    if (event->button() == Qt::LeftButton) {
        setChecked(!m_checked);
    }

    QWidget::mousePressEvent(event);
}

void FluentToggleSwitch::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    startFocusAnimation(1.0);
}

void FluentToggleSwitch::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    startFocusAnimation(0.0);
}

void FluentToggleSwitch::startFocusAnimation(qreal endValue)
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

void FluentToggleSwitch::enterEvent(FluentEnterEvent *event)
{
    QWidget::enterEvent(event);
    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    m_hoverAnim->stop();
    if (m_hoverAnim->duration() <= 0) {
        setHoverLevel(1.0);
        return;
    }
    m_hoverAnim->setStartValue(m_hoverLevel);
    m_hoverAnim->setEndValue(1.0);
    m_hoverAnim->start();
}

void FluentToggleSwitch::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    m_hoverAnim->stop();
    if (m_hoverAnim->duration() <= 0) {
        setHoverLevel(0.0);
        return;
    }
    m_hoverAnim->setStartValue(m_hoverLevel);
    m_hoverAnim->setEndValue(0.0);
    m_hoverAnim->start();
}

} // namespace Fluent
