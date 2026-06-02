#include "Fluent/FluentAnimatedButton.h"

#include "Fluent/FluentMotion.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"
#include "FluentButtonVisuals_p.h"

#include <QAbstractAnimation>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QVariantAnimation>

namespace Fluent {

namespace {

constexpr int kIconTextGap = 8;

struct ButtonColors
{
    QColor base;
    QColor hover;
    QColor pressed;
    QColor border;
    QColor bottomBorder;
    QColor text;
};

QString normalizedState(QString state)
{
    state = state.trimmed();
    return state.isEmpty() ? QStringLiteral("Normal") : state;
}

QSize boundedIconSize(const QSize &size)
{
    if (size.isValid()) {
        return size;
    }
    return QSize(24, 24);
}

ButtonColors resolveButtonColors(const ThemeColors &colors, bool primary, bool checked, bool enabled)
{
    const auto tokens = Theme::tokens(colors);
    const ButtonVisuals::StateColors state =
        ButtonVisuals::resolve(colors, tokens, primary, checked, enabled);
    return {state.base, state.hover, state.pressed, state.border, state.bottomBorder, state.text};
}

} // namespace

FluentAnimatedButton::FluentAnimatedButton(QWidget *parent)
    : QPushButton(parent)
{
    initialize();
}

FluentAnimatedButton::FluentAnimatedButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    initialize();
}

bool FluentAnimatedButton::isPrimary() const
{
    return m_primary;
}

void FluentAnimatedButton::setPrimary(bool primary)
{
    if (m_primary == primary) {
        return;
    }

    m_primary = primary;
    applyTheme();
}

qreal FluentAnimatedButton::hoverLevel() const
{
    return m_hoverLevel;
}

void FluentAnimatedButton::setHoverLevel(qreal value)
{
    m_hoverLevel = qBound<qreal>(0.0, value, 1.0);
    update();
}

qreal FluentAnimatedButton::pressLevel() const
{
    return m_pressLevel;
}

void FluentAnimatedButton::setPressLevel(qreal value)
{
    m_pressLevel = qBound<qreal>(0.0, value, 1.0);
    update();
}

QSize FluentAnimatedButton::animatedIconSize() const
{
    return m_animatedIconSize;
}

void FluentAnimatedButton::setAnimatedIconSize(const QSize &size)
{
    const QSize next = boundedIconSize(size);
    if (m_animatedIconSize == next) {
        return;
    }

    m_animatedIconSize = next;
    m_animatedIcon->setFallbackIconSize(next);
    m_animatedIcon->setFixedSize(next);
    updateAnimatedIconGeometry();
    updateGeometry();
    update();
}

FluentAnimatedIcon *FluentAnimatedButton::animatedIcon() const
{
    return m_animatedIcon;
}

bool FluentAnimatedButton::loadAnimation(const QString &path)
{
    const bool ok = m_animatedIcon->load(path);
    syncRestingAnimationState(false);
    updateAnimatedIconGeometry();
    return ok;
}

bool FluentAnimatedButton::loadAnimationData(const QByteArray &json, const QString &cacheKey, const QString &resourcePath)
{
    const bool ok = m_animatedIcon->loadData(json, cacheKey, resourcePath);
    syncRestingAnimationState(false);
    updateAnimatedIconGeometry();
    return ok;
}

QString FluentAnimatedButton::animationState() const
{
    return m_animationState;
}

void FluentAnimatedButton::setAnimationState(const QString &state)
{
    setAnimationState(state, true);
}

void FluentAnimatedButton::setAnimationState(const QString &state, bool animated)
{
    const QString next = normalizedState(state);
    if (m_animationState == next) {
        return;
    }

    m_animationState = next;
    m_animatedIcon->setState(next, animated);
    emit animationStateChanged(next);
}

QSize FluentAnimatedButton::sizeHint() const
{
    const auto m = Style::metrics();
    const int textWidth = text().isEmpty() ? 0 : fontMetrics().horizontalAdvance(text());
    const int iconWidth = m_animatedIconSize.width();
    const int contentWidth = textWidth > 0 ? iconWidth + kIconTextGap + textWidth : iconWidth;
    const int width = contentWidth + m.paddingX * 2;
    const int height = qMax(m.height, m_animatedIconSize.height() + m.paddingY * 2);
    return QSize(width, height);
}

QSize FluentAnimatedButton::minimumSizeHint() const
{
    return QSize(qMax(32, m_animatedIconSize.width() + 12), qMax(32, m_animatedIconSize.height() + 8));
}

void FluentAnimatedButton::changeEvent(QEvent *event)
{
    QPushButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        applyTheme();
        syncRestingAnimationState(true);
    } else if (event->type() == QEvent::FontChange) {
        updateAnimatedIconGeometry();
        updateGeometry();
    }
}

void FluentAnimatedButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    updateAnimatedIconGeometry();

    const auto &colors = ThemeManager::instance().colors();
    const auto &tokens = ThemeManager::instance().tokens();
    const bool checked = isCheckable() && isChecked();
    const ButtonColors buttonColors = resolveButtonColors(colors, m_primary, checked, isEnabled());
    const ButtonVisuals::StateColors state = {buttonColors.base,
                                              buttonColors.hover,
                                              buttonColors.pressed,
                                              buttonColors.border,
                                              buttonColors.bottomBorder,
                                              buttonColors.text};
    const QColor fill = ButtonVisuals::fillForState(state, m_hoverLevel, m_pressLevel);
    const QColor textColor = ButtonVisuals::textForState(buttonColors.text, m_pressLevel, isEnabled());

    QPainter painter(this);
    if (!painter.isActive()) {
        return;
    }

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const auto m = Style::metrics();
    const QRectF rect = QRectF(this->rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    const qreal radius = m.radius;

    ButtonVisuals::paintRoundedControl(painter, rect, radius, fill, buttonColors.border, buttonColors.bottomBorder);

    if (checked && m_primary && isEnabled()) {
        QColor inner = textColor;
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

    if (text().isEmpty()) {
        return;
    }

    const QRect contentRect = rect.toRect();
    const int textWidth = fontMetrics().horizontalAdvance(text());
    const int totalWidth = m_animatedIconSize.width() + kIconTextGap + textWidth;
    const int startX = contentRect.center().x() - totalWidth / 2;
    const int textLeft = startX + m_animatedIconSize.width() + kIconTextGap;
    const QRect textRect(textLeft,
                         contentRect.top(),
                         contentRect.right() - textLeft + 1,
                         contentRect.height());

    painter.setPen(textColor);
    painter.setFont(font());
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextShowMnemonic, text());
}

void FluentAnimatedButton::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    updateAnimatedIconGeometry();
}

void FluentAnimatedButton::enterEvent(FluentEnterEvent *event)
{
    QPushButton::enterEvent(event);
    startHoverAnimation(1.0);
    if (isEnabled()) {
        setAnimationState(QStringLiteral("PointerOver"), true);
    }
}

void FluentAnimatedButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    startHoverAnimation(0.0);
    startPressAnimation(0.0);
    syncRestingAnimationState(true);
}

void FluentAnimatedButton::mousePressEvent(QMouseEvent *event)
{
    startPressAnimation(1.0);
    if (isEnabled() && event->button() == Qt::LeftButton) {
        setAnimationState(QStringLiteral("Pressed"), true);
    }
    QPushButton::mousePressEvent(event);
}

void FluentAnimatedButton::mouseReleaseEvent(QMouseEvent *event)
{
    startPressAnimation(0.0);
    QPushButton::mouseReleaseEvent(event);
    if (isEnabled() && event->button() == Qt::LeftButton) {
        if (rect().contains(mousePositionF(event).toPoint())) {
            setAnimationState(QStringLiteral("PointerOver"), true);
        } else {
            syncRestingAnimationState(true);
        }
    }
}

void FluentAnimatedButton::initialize()
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    setAutoDefault(false);
    setMinimumHeight(Style::metrics().height);

    m_animatedIcon = new FluentAnimatedIcon(this);
    m_animatedIcon->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_animatedIcon->setInteractive(false);
    m_animatedIcon->setFallbackIconSize(m_animatedIconSize);
    m_animatedIcon->setFixedSize(m_animatedIconSize);

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
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentAnimatedButton::applyTheme);
    connect(this, &QAbstractButton::toggled, this, [this](bool) {
        updateAnimatedIconTint();
        syncRestingAnimationState(true);
        update();
    });
}

void FluentAnimatedButton::applyTheme()
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
    updateAnimatedIconTint();
    update();
}

void FluentAnimatedButton::startHoverAnimation(qreal endValue)
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

void FluentAnimatedButton::startPressAnimation(qreal endValue)
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

void FluentAnimatedButton::updateAnimatedIconGeometry()
{
    if (!m_animatedIcon) {
        return;
    }

    const QRect contentRect = rect();
    int x = contentRect.center().x() - m_animatedIconSize.width() / 2;

    if (!text().isEmpty()) {
        const int textWidth = fontMetrics().horizontalAdvance(text());
        const int totalWidth = m_animatedIconSize.width() + kIconTextGap + textWidth;
        x = contentRect.center().x() - totalWidth / 2;
    }

    const int y = contentRect.center().y() - m_animatedIconSize.height() / 2;
    m_animatedIcon->setGeometry(QRect(QPoint(x, y), m_animatedIconSize));
}

void FluentAnimatedButton::updateAnimatedIconTint()
{
    if (!m_animatedIcon) {
        return;
    }

    m_animatedIcon->setTintColor(currentForegroundColor());
}

QColor FluentAnimatedButton::currentForegroundColor() const
{
    const bool checked = isCheckable() && isChecked();
    return resolveButtonColors(ThemeManager::instance().colors(), m_primary, checked, isEnabled()).text;
}

void FluentAnimatedButton::syncRestingAnimationState(bool animated)
{
    if (!isEnabled() && hasAnimationState(QStringLiteral("Disabled"))) {
        setAnimationState(QStringLiteral("Disabled"), animated);
        return;
    }

    if (isCheckable() && isChecked() && hasAnimationState(QStringLiteral("Selected"))) {
        setAnimationState(QStringLiteral("Selected"), animated);
        return;
    }

    setAnimationState(QStringLiteral("Normal"), animated);
}

bool FluentAnimatedButton::hasAnimationState(const QString &state) const
{
    if (m_animatedIcon->hasMarker(state)) {
        return true;
    }

    const QString suffix = QStringLiteral("To") + state + QStringLiteral("_End");
    const QStringList names = m_animatedIcon->markerNames();
    for (const QString &name : names) {
        if (name.endsWith(suffix, Qt::CaseInsensitive)) {
            return true;
        }
    }

    return false;
}

} // namespace Fluent
