#include "Fluent/FluentLineEdit.h"
#include "Fluent/FluentMotion.h"
#include "Fluent/FluentQtCompat.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"
#include "FluentInputVisuals_p.h"

#include <QAbstractAnimation>
#include <QEvent>
#include <QPainter>
#include <QVariantAnimation>

namespace Fluent {

FluentLineEdit::FluentLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setFrame(false);
    const auto m = Style::metrics();
    setTextMargins(m.paddingX, 0, m.paddingX, 0);
    setMinimumHeight(m.height);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);

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

    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentLineEdit::applyTheme);
}

FluentLineEdit::FluentLineEdit(const QString &text, QWidget *parent)
    : QLineEdit(text, parent)
{
    setFrame(false);
    const auto m = Style::metrics();
    setTextMargins(m.paddingX, 0, m.paddingX, 0);
    setMinimumHeight(m.height);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);

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

    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentLineEdit::applyTheme);
}

qreal FluentLineEdit::hoverLevel() const
{
    return m_hoverLevel;
}

void FluentLineEdit::setHoverLevel(qreal value)
{
    m_hoverLevel = qBound(0.0, value, 1.0);
    update();
}

qreal FluentLineEdit::focusLevel() const
{
    return m_focusLevel;
}

void FluentLineEdit::setFocusLevel(qreal value)
{
    m_focusLevel = qBound(0.0, value, 1.0);
    update();
}

void FluentLineEdit::changeEvent(QEvent *event)
{
    QLineEdit::changeEvent(event);

    if (event->type() == QEvent::EnabledChange) {
        applyTheme();
    }
}

void FluentLineEdit::applyTheme()
{
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

    const auto &colors = ThemeManager::instance().colors();
    const auto &tokens = ThemeManager::instance().tokens();
    const QColor textColor = isEnabled() ? colors.text : colors.disabledText;

    const bool dark = ThemeManager::instance().themeMode() == ThemeManager::ThemeMode::Dark;
    QColor selectionBg = tokens.accent.base;
    selectionBg.setAlphaF(dark ? 0.35 : 0.22);

    const QString next = QStringLiteral(
        "QLineEdit { background: transparent; color: %1; border: none; "
        "selection-background-color: %2; selection-color: %3; }"
        "QLineEdit:disabled { color: %4; }")
        .arg(textColor.name(QColor::HexArgb),
             selectionBg.name(QColor::HexArgb),
             colors.text.name(QColor::HexArgb),
             colors.disabledText.name(QColor::HexArgb));
    if (styleSheet() != next) {
        setStyleSheet(next);
    }

    // QSS owns text color; QPalette::Text is left for the native caret color.
    QPalette pal = palette();
    pal.setColor(QPalette::WindowText, textColor);
    pal.setColor(QPalette::Text, isEnabled() ? tokens.accent.base : colors.disabledText);
    pal.setColor(QPalette::Disabled, QPalette::WindowText, colors.disabledText);
    pal.setColor(QPalette::Disabled, QPalette::Text, colors.disabledText);
    pal.setColor(QPalette::Highlight, selectionBg);
    pal.setColor(QPalette::HighlightedText, colors.text);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    // WaterMark/placeholder should stay readable; do NOT follow accent.
    QColor placeholder = colors.subText;
    placeholder.setAlphaF(dark ? 0.55 : 0.60);
    pal.setColor(QPalette::PlaceholderText, placeholder);
#endif
    setPalette(pal);
    update();
}

void FluentLineEdit::paintEvent(QPaintEvent *event)
{
    const auto &colors = ThemeManager::instance().colors();

    QPainter painter(this);
    if (!painter.isActive()) {
        QLineEdit::paintEvent(event);
        return;
    }
    Style::paintControlSurface(
        painter,
        QRectF(this->rect()),
        colors,
        m_hoverLevel,
        0.0,
        isEnabled(),
        false);
    InputVisuals::paintFocusRing(painter, QRectF(this->rect()).adjusted(0.5, 0.5, -0.5, -0.5), colors, isEnabled() ? m_focusLevel : 0.0);

    QLineEdit::paintEvent(event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void FluentLineEdit::enterEvent(QEnterEvent *event)
#else
void FluentLineEdit::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event);
    startHoverAnimation(1.0);
}

void FluentLineEdit::leaveEvent(QEvent *event)
{
    QLineEdit::leaveEvent(event);
    startHoverAnimation(0.0);
}

void FluentLineEdit::focusInEvent(QFocusEvent *event)
{
    QLineEdit::focusInEvent(event);
    startFocusAnimation(1.0);
}

void FluentLineEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);
    startFocusAnimation(0.0);
}

void FluentLineEdit::startHoverAnimation(qreal endValue)
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

void FluentLineEdit::startFocusAnimation(qreal endValue)
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
