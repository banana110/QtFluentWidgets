#include "Fluent/FluentDial.h"
#include "Fluent/FluentMotion.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"

#include <QAbstractAnimation>
#include <QFocusEvent>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVariantAnimation>
#include <QWheelEvent>
#include <QEasingCurve>
#include <QtMath>
#include <cmath>

namespace Fluent {

namespace {
static QPointF localPosF(QMouseEvent *e)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return e->position();
#else
    return e->localPos();
#endif
}

static int normalizedDegrees(int angleDegrees)
{
    return ((angleDegrees % 360) + 360) % 360;
}

static QRectF squareDialRect(const QRect &widgetRect, qreal margin = 4.0)
{
    QRectF bounds(widgetRect);
    bounds.adjust(margin, margin, -margin, -margin);
    const qreal side = qMax<qreal>(1.0, qMin(bounds.width(), bounds.height()));
    return QRectF(bounds.center().x() - side / 2.0,
                  bounds.center().y() - side / 2.0,
                  side,
                  side);
}
} // namespace

FluentDial::FluentDial(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(38, 38);
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);
    setToolTip(tr("拖拽旋转调整角度"));

    m_hoverAnim = new QVariantAnimation(this);
    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    connect(m_hoverAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_hoverLevel = value.toReal();
        update();
    });

    m_focusAnim = new QVariantAnimation(this);
    FluentMotion::configure(m_focusAnim, FluentMotionRole::Focus);
    connect(m_focusAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_focusLevel = value.toReal();
        update();
    });

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        const bool hoverRunning = m_hoverAnim && m_hoverAnim->state() == QAbstractAnimation::Running;
        const bool focusRunning = m_focusAnim && m_focusAnim->state() == QAbstractAnimation::Running;
        const QVariant hoverEnd = m_hoverAnim ? m_hoverAnim->endValue() : QVariant();
        const QVariant focusEnd = m_focusAnim ? m_focusAnim->endValue() : QVariant();

        FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
        FluentMotion::configure(m_focusAnim, FluentMotionRole::Focus);
        if (hoverRunning && m_hoverAnim->duration() <= 0) {
            m_hoverAnim->stop();
            m_hoverLevel = qBound<qreal>(0.0, hoverEnd.toReal(), 1.0);
        }
        if (focusRunning && m_focusAnim->duration() <= 0) {
            m_focusAnim->stop();
            m_focusLevel = qBound<qreal>(0.0, focusEnd.toReal(), 1.0);
        }
        update();
    });
    syncEnabledState();
}

QSize FluentDial::sizeHint() const
{
    return {38, 38};
}

QSize FluentDial::minimumSizeHint() const
{
    return sizeHint();
}

void FluentDial::setValue(int angleDegrees)
{
    setValueInternal(angleDegrees, true);
}

void FluentDial::setTicksVisible(bool visible)
{
    if (m_ticksVisible == visible)
        return;
    m_ticksVisible = visible;
    update();
}

void FluentDial::setTickStep(int stepDegrees)
{
    stepDegrees = qMax(1, stepDegrees);
    if (m_tickStep == stepDegrees)
        return;
    m_tickStep = stepDegrees;
    update();
}

void FluentDial::setMajorTickStep(int stepDegrees)
{
    stepDegrees = qMax(1, stepDegrees);
    if (m_majorTickStep == stepDegrees)
        return;
    m_majorTickStep = stepDegrees;
    update();
}

void FluentDial::setPointerVisible(bool visible)
{
    if (m_pointerVisible == visible)
        return;
    m_pointerVisible = visible;
    update();
}

void FluentDial::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event && event->type() == QEvent::EnabledChange) {
        syncEnabledState();
    }
}

void FluentDial::enterEvent(FluentEnterEvent *event)
{
    QWidget::enterEvent(event);
    if (isEnabled()) {
        startHoverAnimation(1.0);
    }
}

void FluentDial::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    startHoverAnimation(0.0);
}

void FluentDial::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    if (isEnabled()) {
        startFocusAnimation(1.0);
    }
}

void FluentDial::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    startFocusAnimation(0.0);
}

void FluentDial::setValueInternal(int angleDegrees, bool emitSignal)
{
    const int v = normalizedDegrees(angleDegrees);
    if (m_value == v) {
        return;
    }
    m_value = v;
    update();
    if (emitSignal) {
        emit valueChanged(m_value);
    }
}

void FluentDial::syncEnabledState()
{
    setCursor(isEnabled() ? Qt::PointingHandCursor : Qt::ArrowCursor);
    if (!isEnabled()) {
        m_dragging = false;
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

void FluentDial::startHoverAnimation(qreal endValue)
{
    if (!m_hoverAnim || !isEnabled()) {
        m_hoverLevel = isEnabled() ? qBound<qreal>(0.0, endValue, 1.0) : 0.0;
        update();
        return;
    }
    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    m_hoverAnim->stop();
    if (m_hoverAnim->duration() <= 0) {
        m_hoverLevel = qBound<qreal>(0.0, endValue, 1.0);
        update();
        return;
    }
    m_hoverAnim->setStartValue(m_hoverLevel);
    m_hoverAnim->setEndValue(qBound<qreal>(0.0, endValue, 1.0));
    m_hoverAnim->start();
}

void FluentDial::startFocusAnimation(qreal endValue)
{
    if (!m_focusAnim || !isEnabled()) {
        m_focusLevel = isEnabled() ? qBound<qreal>(0.0, endValue, 1.0) : 0.0;
        update();
        return;
    }
    FluentMotion::configure(m_focusAnim, FluentMotionRole::Focus);
    m_focusAnim->stop();
    if (m_focusAnim->duration() <= 0) {
        m_focusLevel = qBound<qreal>(0.0, endValue, 1.0);
        update();
        return;
    }
    m_focusAnim->setStartValue(m_focusLevel);
    m_focusAnim->setEndValue(qBound<qreal>(0.0, endValue, 1.0));
    m_focusAnim->start();
}

void FluentDial::updateFromPos(const QPointF &pos, bool emitSignal)
{
    const QPointF c = squareDialRect(rect()).center();
    const QPointF d = pos - c;
    if (std::abs(d.x()) < 1.0 && std::abs(d.y()) < 1.0) return; // ignore centre clicks

    qreal angle = qRadiansToDegrees(std::atan2(d.y(), d.x()));
    if (angle < 0.0) angle += 360.0;
    const int newVal = qBound(0, qRound(angle) % 360, 359);
    setValueInternal(newVal, emitSignal);
}

void FluentDial::mousePressEvent(QMouseEvent *event)
{
    if (!isEnabled()) {
        QWidget::mousePressEvent(event);
        return;
    }
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        updateFromPos(localPosF(event));
    }
    QWidget::mousePressEvent(event);
}

void FluentDial::mouseMoveEvent(QMouseEvent *event)
{
    if (!isEnabled()) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    if (m_dragging && (event->buttons() & Qt::LeftButton))
        updateFromPos(localPosF(event));
    QWidget::mouseMoveEvent(event);
}

void FluentDial::mouseReleaseEvent(QMouseEvent *event)
{
    if (!isEnabled()) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    if (event->button() == Qt::LeftButton && m_dragging) {
        updateFromPos(localPosF(event));
        m_dragging = false;
    }
    QWidget::mouseReleaseEvent(event);
}

void FluentDial::wheelEvent(QWheelEvent *event)
{
    if (!isEnabled()) {
        QWidget::wheelEvent(event);
        return;
    }
    // One wheel notch = ±1°
    const int delta = (event->angleDelta().y() > 0) ? 1 : -1;
    setValueInternal(m_value + delta, true);
    event->accept();
}

void FluentDial::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const auto &tokens = ThemeManager::instance().tokens();
    const QColor disabledText = tokens.legacyColors.disabledText;
    const QColor mutedAccent = Style::mix(tokens.neutral.strokeStrong,
                                          tokens.accent.base,
                                          tokens.dark ? 0.22 : 0.28);
    const bool enabled = isEnabled();
    const qreal hoverLevel = enabled ? m_hoverLevel : 0.0;
    const qreal focusLevel = enabled ? m_focusLevel : 0.0;

    // Geometry
    const QRectF outer = squareDialRect(rect());
    const QPointF center = outer.center();
    const qreal radius   = outer.width() / 2.0;

    if (enabled && (hoverLevel > 0.0 || focusLevel > 0.0)) {
        QColor halo = tokens.accent.base;
        halo.setAlphaF(qBound<qreal>(0.0, 0.08 + 0.12 * hoverLevel + 0.18 * focusLevel, 0.34));
        p.setPen(QPen(halo, 1.8));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(outer.adjusted(-1.5, -1.5, 1.5, 1.5));
    }

    // ── Track ring ────────────────────────────────────────────────────────
    QColor fill = enabled
        ? tokens.neutral.card
        : Style::mix(tokens.neutral.card, tokens.neutral.background, tokens.dark ? 0.48 : 0.35);
    if (enabled && hoverLevel > 0.0)
        fill = Style::mix(tokens.neutral.card, tokens.neutral.cardHover, 0.30 * hoverLevel);
    QColor stroke = enabled
        ? Style::mix(tokens.neutral.strokeSubtle, tokens.accent.base, 0.18 * hoverLevel + 0.15 * focusLevel)
        : Style::mix(tokens.neutral.strokeSubtle, disabledText, tokens.dark ? 0.28 : 0.18);
    p.setPen(QPen(stroke, 2.5, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(fill);
    p.drawEllipse(outer);

    if (m_ticksVisible) {
        const int tickStep = qMax(1, m_tickStep);
        const int majorStep = qMax(tickStep, m_majorTickStep);
        for (int angle = 0; angle < 360; angle += tickStep) {
            const bool major = (angle % majorStep) == 0;
            QColor tickColor = enabled
                ? Style::mix(tokens.neutral.strokeSubtle, tokens.accent.base, major ? 0.38 : 0.18)
                : disabledText;
            tickColor.setAlpha(enabled ? (major ? 190 : 130) : 95);
            const qreal angleRad = qDegreesToRadians(static_cast<qreal>(angle));
            const qreal outerR = radius - 1.8;
            const qreal innerR = radius - (major ? 6.1 : 4.2);
            const QPointF from(center.x() + innerR * std::cos(angleRad),
                               center.y() + innerR * std::sin(angleRad));
            const QPointF to(center.x() + outerR * std::cos(angleRad),
                             center.y() + outerR * std::sin(angleRad));
            p.setPen(QPen(tickColor, major ? 1.35 : 1.0, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(from, to);
        }
    }

    // ── Filled arc from 0° to current angle ──────────────────────────────
    // Qt arcs start at 3-o'clock (0°) and go counter-clockwise in angle units.
    // Our convention: 0° = east, clockwise positive → negate for Qt.
    if (m_value > 0) {
        QColor arcColor = enabled
            ? (m_dragging ? tokens.accent.light1 : Style::mix(tokens.accent.base, tokens.accent.light2, 0.22 * focusLevel))
            : mutedAccent;
        if (!enabled) {
            arcColor.setAlpha(135);
        }
        QPen arcPen(arcColor, 2.7, Qt::SolidLine, Qt::RoundCap);
        p.setPen(arcPen);
        p.setBrush(Qt::NoBrush);
        // Qt drawArc: startAngle in 1/16ths of a degree, counter-clockwise
        const int startAngle = 0;
        const int spanAngle  = -qRound(m_value * 16.0); // negative = clockwise
        p.drawArc(outer.adjusted(1.25, 1.25, -1.25, -1.25), startAngle, spanAngle);
    }

    // ── Pointer + indicator dot ───────────────────────────────────────────
    const qreal angleRad = qDegreesToRadians((qreal)m_value);
    const qreal dotCx = center.x() + (radius - 1.5) * std::cos(angleRad);
    const qreal dotCy = center.y() + (radius - 1.5) * std::sin(angleRad);

    if (m_pointerVisible) {
        const qreal innerLen = 5.0;
        const qreal pointerLen = radius - 6.5;
        const QPointF from(center.x() + innerLen * std::cos(angleRad),
                           center.y() + innerLen * std::sin(angleRad));
        const QPointF to(center.x() + pointerLen * std::cos(angleRad),
                         center.y() + pointerLen * std::sin(angleRad));
        QColor pointerColor = enabled
            ? (m_dragging ? tokens.accent.light1 : Style::mix(tokens.accent.base, tokens.accent.light2, 0.18 * focusLevel))
            : mutedAccent;
        if (!enabled) {
            pointerColor.setAlpha(125);
        }
        p.setPen(QPen(pointerColor, 2.2, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(from, to);
    }

    // Shadow
    if (enabled) {
        QColor dotShadow = tokens.elevation.shadow;
        dotShadow.setAlpha(tokens.dark ? 70 : 42);
        p.setPen(Qt::NoPen);
        p.setBrush(dotShadow);
        p.drawEllipse(QPointF(dotCx, dotCy + 1.0), 5.0, 5.0);
    }

    // Dot
    QColor dotColor = enabled
        ? (m_dragging ? tokens.accent.light1 : Style::mix(tokens.accent.base, tokens.accent.light2, 0.20 * focusLevel))
        : mutedAccent;
    if (!enabled) {
        dotColor.setAlpha(150);
    }
    p.setBrush(dotColor);
    p.setPen(QPen(fill, 1.5));
    p.drawEllipse(QPointF(dotCx, dotCy), 5.0, 5.0);

    // ── Centre dot ───────────────────────────────────────────────────────
    QColor centerDot = enabled
        ? Style::mix(tokens.neutral.strokeSubtle, tokens.accent.base, 0.30 * focusLevel)
        : mutedAccent;
    if (!enabled) {
        centerDot.setAlpha(115);
    }
    p.setBrush(centerDot);
    p.setPen(Qt::NoPen);
    p.drawEllipse(center, 2.5, 2.5);
}

} // namespace Fluent

