#include "Fluent/FluentToolButton.h"
#include "Fluent/FluentMotion.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"
#include "FluentButtonVisuals_p.h"

#include <QAbstractAnimation>
#include <QEvent>
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QVariantAnimation>

namespace Fluent {

FluentToolButton::FluentToolButton(QWidget *parent)
    : QToolButton(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    setAutoRaise(true);
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
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentToolButton::applyTheme);
    connect(this, &QAbstractButton::toggled, this, QOverload<>::of(&FluentToolButton::update));
}

FluentToolButton::FluentToolButton(const QString &text, QWidget *parent)
    : QToolButton(parent)
{
    setText(text);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    setAutoRaise(true);
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
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentToolButton::applyTheme);
    connect(this, &QAbstractButton::toggled, this, QOverload<>::of(&FluentToolButton::update));
}

qreal FluentToolButton::hoverLevel() const
{
    return m_hoverLevel;
}

void FluentToolButton::setHoverLevel(qreal value)
{
    m_hoverLevel = qBound(0.0, value, 1.0);
    update();
}

qreal FluentToolButton::pressLevel() const
{
    return m_pressLevel;
}

void FluentToolButton::setPressLevel(qreal value)
{
    m_pressLevel = qBound(0.0, value, 1.0);
    update();
}

void FluentToolButton::changeEvent(QEvent *event)
{
    QToolButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        applyTheme();
    }
}

void FluentToolButton::applyTheme()
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

void FluentToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    const auto &colors = ThemeManager::instance().colors();
    const auto &tokens = ThemeManager::instance().tokens();

    const auto m = Style::metrics();

    QPainter painter(this);
    if (!painter.isActive()) {
        return;
    }
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRect rect = this->rect().adjusted(0, 0, -1, -1);

    // Caption / window buttons (used by FluentMainWindow)
    const QVariant glyphVar = property("fluentWindowGlyph");
    if (glyphVar.isValid()) {
        const int glyph = glyphVar.toInt();
        const bool isClose = (glyph == 3);
        const bool neutralClose = property("fluentNeutralCloseGlyph").toBool();

        const qreal hover = isEnabled() ? m_hoverLevel : 0.0;
        const qreal press = isEnabled() ? m_pressLevel : 0.0;

        QColor fill = Qt::transparent;
        if (isClose && !neutralClose) {
            // Win11-like close hover: vivid red hover with a theme-derived pressed shade.
            QColor danger = tokens.semantic.error.isValid() ? tokens.semantic.error : QColor("#E81123");
            const QColor dangerPressedTarget = tokens.dark ? tokens.neutral.background : colors.text;
            QColor dangerPressed = Style::mix(danger, dangerPressedTarget, tokens.dark ? 0.16 : 0.20);
            danger.setAlphaF(0.92);
            dangerPressed.setAlphaF(0.97);
            fill = ButtonVisuals::transparentVersion(danger);
            fill = Style::mix(fill, danger, hover);
            fill = Style::mix(fill, dangerPressed, press);
        } else {
            // Subtle hover/press like caption buttons, derived from neutral tokens.
            QColor hoverFill = tokens.neutral.cardHover;
            hoverFill.setAlphaF(tokens.dark ? 0.62 : 0.56);
            QColor pressedFill = tokens.neutral.fillTertiary;
            pressedFill.setAlphaF(tokens.dark ? 0.78 : 0.72);
            fill = ButtonVisuals::transparentVersion(hoverFill);
            fill = Style::mix(fill, hoverFill, hover);
            fill = Style::mix(fill, pressedFill, press);
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(fill);
        painter.drawRoundedRect(QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5), 4.0, 4.0);

        QColor glyphColor = Style::withAlpha(colors.text, 235);
        if (isClose && !neutralClose && (hover > 0.01 || press > 0.01)) {
            glyphColor = Style::withAlpha(Theme::contrastColor(tokens.semantic.error), 245);
        }

        const qreal strokeW = 1.6;
        painter.setPen(QPen(glyphColor, strokeW, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.setBrush(Qt::NoBrush);

        // Use a fixed square glyph box so icons stay 16x16 and never look squashed
        // when the caption button is wide.
        const QPointF center = QRectF(rect).center();
        const QRectF glyphBox(center.x() - 8.0, center.y() - 8.0, 16.0, 16.0);
        const qreal pad = 3.0;
        const QRectF r = glyphBox.adjusted(pad, pad, -pad, -pad);

        switch (glyph) {
        case 0: // minimize
            painter.drawLine(QPointF(r.left(), r.bottom()), QPointF(r.right(), r.bottom()));
            break;
        case 1: // maximize
            painter.drawRoundedRect(r.adjusted(0.7, 0.7, -0.7, -0.7), 1.2, 1.2);
            break;
        case 2: // restore
            {
                // Draw two windows with a visible offset so the glyph doesn't look like
                // perfectly stacked squares.
                const qreal offset = 1.6;
                const qreal shrink = 0.9;
                const QRectF base = r.adjusted(shrink, shrink, -shrink, -shrink);
                const QRectF back = base.translated(offset, -offset);
                const QRectF front = base.translated(-offset, offset);
                painter.drawRoundedRect(back, 1.2, 1.2);
                painter.drawRoundedRect(front, 1.2, 1.2);
            }
            break;
        case 3: // close
            painter.drawLine(QPointF(r.left(), r.top()), QPointF(r.right(), r.bottom()));
            painter.drawLine(QPointF(r.right(), r.top()), QPointF(r.left(), r.bottom()));
            break;
        default:
            break;
        }
        return;
    }

    const bool subtleCommandButton = property("fluentCommandBarButton").toBool();

    const bool checked = isCheckable() && isChecked();
    const ButtonVisuals::StateColors state =
        ButtonVisuals::resolve(colors, tokens, false, checked, isEnabled(), subtleCommandButton, 0.16);
    const QColor fill = ButtonVisuals::fillForState(state, m_hoverLevel, m_pressLevel);

    ButtonVisuals::paintRoundedControl(
        painter, QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5), m.radius, fill, state.border, state.bottomBorder);

    if (hasFocus() && isEnabled()) {
        QColor focus = tokens.accent.base;
        focus.setAlpha(230);
        painter.setPen(QPen(focus, 2.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(QRectF(rect).adjusted(1.0, 1.0, -1.0, -1.0), m.radius - 1, m.radius - 1);
    }

    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    opt.state &= ~QStyle::State_MouseOver;
    opt.state &= ~QStyle::State_Sunken;
    opt.state &= ~QStyle::State_On;
    const QColor labelColor = isEnabled()
        ? ButtonVisuals::textForState(state.text, m_pressLevel, true)
        : colors.disabledText;
    opt.palette.setColor(QPalette::ButtonText, labelColor);
    opt.palette.setColor(QPalette::Text, labelColor);
    opt.palette.setColor(QPalette::WindowText, labelColor);
    opt.palette.setColor(QPalette::Disabled, QPalette::ButtonText, colors.disabledText);
    opt.palette.setColor(QPalette::Disabled, QPalette::Text, colors.disabledText);
    opt.palette.setColor(QPalette::Disabled, QPalette::WindowText, colors.disabledText);
    if (checked && isEnabled()) {
        // Make the checked state more obvious by tinting label color.
        opt.palette.setColor(QPalette::ButtonText, labelColor);
        opt.palette.setColor(QPalette::Text, labelColor);
    }
    style()->drawControl(QStyle::CE_ToolButtonLabel, &opt, &painter, this);
}

void FluentToolButton::enterEvent(FluentEnterEvent *event)
{
    QToolButton::enterEvent(event);
    startHoverAnimation(1.0);
}

void FluentToolButton::leaveEvent(QEvent *event)
{
    QToolButton::leaveEvent(event);
    startHoverAnimation(0.0);
}

void FluentToolButton::mousePressEvent(QMouseEvent *event)
{
    startPressAnimation(1.0);
    QToolButton::mousePressEvent(event);
}

void FluentToolButton::mouseReleaseEvent(QMouseEvent *event)
{
    startPressAnimation(0.0);
    QToolButton::mouseReleaseEvent(event);
}

void FluentToolButton::startHoverAnimation(qreal endValue)
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

void FluentToolButton::startPressAnimation(qreal endValue)
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
