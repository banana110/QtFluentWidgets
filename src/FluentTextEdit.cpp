#include "Fluent/FluentTextEdit.h"
#include "Fluent/FluentMotion.h"
#include "Fluent/FluentScrollBar.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"
#include "FluentInputVisuals_p.h"

#include <QAbstractAnimation>
#include <QEvent>
#include <QFocusEvent>
#include <QFrame>
#include <QPainter>
#include <QPalette>
#include <QResizeEvent>
#include <QShowEvent>
#include <QVariantAnimation>
#include <QWidget>

namespace Fluent {

namespace {

class BorderOverlay final : public QWidget
{
public:
    explicit BorderOverlay(FluentTextEdit *owner)
        : QWidget(owner)
        , m_owner(owner)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAutoFillBackground(false);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        if (!m_owner) {
            return;
        }

        QPainter p(this);
        if (!p.isActive()) {
            return;
        }

        const auto m = Style::metrics();
        const auto &colors = ThemeManager::instance().colors();

        const QColor stroke = InputVisuals::inputStroke(colors, m_owner->isEnabled());

        p.setRenderHint(QPainter::Antialiasing, true);
        const QRectF r = QRectF(rect());

        // Pixel-align like Style::paintControlSurface.
        qreal dpr = 1.0;
        if (p.device()) {
            dpr = p.device()->devicePixelRatioF();
            if (dpr <= 0.0) {
                dpr = 1.0;
            }
        }
        const qreal px = 0.5 / dpr;
        const QRectF rr = r.adjusted(px, px, -px, -px);

        p.setPen(QPen(stroke, 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawPath(Style::roundedRectPath(rr, m.radius));

        InputVisuals::paintFocusRing(p, rr, colors, m_owner->isEnabled() ? m_owner->focusLevel() : 0.0);
    }

private:
    FluentTextEdit *m_owner = nullptr;
};

} // namespace

FluentTextEdit::FluentTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    setFrameStyle(QFrame::NoFrame);
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);

    const auto metrics = Style::metrics();
    setViewportMargins(metrics.paddingX, metrics.paddingY, metrics.paddingX, metrics.paddingY);
    setMinimumHeight(metrics.height);

    setVerticalScrollBar(new FluentScrollBar(Qt::Vertical, this));
    setHorizontalScrollBar(new FluentScrollBar(Qt::Horizontal, this));

    if (viewport()) {
        viewport()->setMouseTracking(true);
        viewport()->setAutoFillBackground(false);
        viewport()->setAttribute(Qt::WA_StyledBackground, true);
        viewport()->setAttribute(Qt::WA_TranslucentBackground, true);
        viewport()->installEventFilter(this);
    }

    m_hoverAnim = new QVariantAnimation(this);
    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    connect(m_hoverAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_hoverLevel = qBound<qreal>(0.0, value.toReal(), 1.0);
        update();
        if (m_borderOverlay) {
            m_borderOverlay->update();
        }
    });

    m_focusAnim = new QVariantAnimation(this);
    FluentMotion::configure(m_focusAnim, FluentMotionRole::Focus);
    connect(m_focusAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_focusLevel = qBound<qreal>(0.0, value.toReal(), 1.0);
        update();
        if (m_borderOverlay) {
            m_borderOverlay->update();
        }
    });

    m_borderOverlay = new BorderOverlay(this);
    updateBorderOverlayGeometry();
    // Defer showing until the widget is shown to avoid early paints.
    m_borderOverlay->hide();

    // Defer theme application until the widget is shown to avoid triggering early viewport paints.
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this] {
        if (!m_themeAppliedOnce && !isVisible()) {
            return;
        }
        applyTheme();
    });
}

QSize FluentTextEdit::sizeHint() const
{
    return QSize(200, 80);
}

qreal FluentTextEdit::hoverLevel() const
{
    return m_hoverLevel;
}

void FluentTextEdit::setHoverLevel(qreal value)
{
    m_hoverLevel = qBound<qreal>(0.0, value, 1.0);
    update();
    if (m_borderOverlay) {
        m_borderOverlay->update();
    }
}

qreal FluentTextEdit::focusLevel() const
{
    return m_focusLevel;
}

void FluentTextEdit::setFocusLevel(qreal value)
{
    m_focusLevel = qBound<qreal>(0.0, value, 1.0);
    update();
    if (m_borderOverlay) {
        m_borderOverlay->update();
    }
}

void FluentTextEdit::changeEvent(QEvent *event)
{
    QTextEdit::changeEvent(event);
    if (event && event->type() == QEvent::EnabledChange) {
        if (!m_themeAppliedOnce && !isVisible()) {
            return;
        }
        applyTheme();
    }
}

void FluentTextEdit::applyTheme()
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
        "QTextEdit { background: transparent; color: %1; border: none; "
        "selection-background-color: %2; selection-color: %3; }"
        "QTextEdit:disabled { color: %4; }"
        "QTextEdit QAbstractScrollArea::viewport { background: transparent; }")
        .arg(textColor.name(QColor::HexArgb),
             selectionBg.name(QColor::HexArgb),
             colors.text.name(QColor::HexArgb),
             colors.disabledText.name(QColor::HexArgb));

    if (styleSheet() != next) {
        setStyleSheet(next);
    }

    QPalette pal = palette();
    pal.setColor(QPalette::Base, QColor(Qt::transparent));
    pal.setColor(QPalette::Window, QColor(Qt::transparent));
    pal.setColor(QPalette::WindowText, textColor);
    pal.setColor(QPalette::Disabled, QPalette::WindowText, colors.disabledText);
    // QSS owns text color; QPalette::Text is left for the native caret color.
    pal.setColor(QPalette::Text, isEnabled() ? tokens.accent.base : colors.disabledText);
    pal.setColor(QPalette::Disabled, QPalette::Text, colors.disabledText);
    pal.setColor(QPalette::Highlight, selectionBg);
    pal.setColor(QPalette::HighlightedText, colors.text);
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    QColor placeholder = colors.subText;
    placeholder.setAlphaF(dark ? 0.55 : 0.60);
    pal.setColor(QPalette::PlaceholderText, placeholder);
#endif
    setPalette(pal);
    if (viewport()) {
        viewport()->setPalette(pal);
    }

    if (isVisible()) {
        update();
        if (m_borderOverlay) {
            m_borderOverlay->update();
        }
    }
}

void FluentTextEdit::paintEvent(QPaintEvent *event)
{
    // Workaround: keep viewport marked as "in paint" while QTextEdit paints it.
    struct ScopedViewportPaintFlag {
        QWidget *vp = nullptr;
        bool prev = false;
        explicit ScopedViewportPaintFlag(QWidget *v)
            : vp(v)
        {
            if (!vp) {
                return;
            }
            prev = vp->testAttribute(Qt::WA_WState_InPaintEvent);
            if (!prev) {
                vp->setAttribute(Qt::WA_WState_InPaintEvent, true);
            }
        }
        ~ScopedViewportPaintFlag()
        {
            if (vp && !prev) {
                vp->setAttribute(Qt::WA_WState_InPaintEvent, false);
            }
        }
    } scopedViewportPaint(viewport());

    QTextEdit::paintEvent(event);
}

bool FluentTextEdit::viewportEvent(QEvent *event)
{
    return QTextEdit::viewportEvent(event);
}

void FluentTextEdit::resizeEvent(QResizeEvent *event)
{
    QTextEdit::resizeEvent(event);
    updateBorderOverlayGeometry();
}

void FluentTextEdit::showEvent(QShowEvent *event)
{
    QTextEdit::showEvent(event);

    if (!m_themeAppliedOnce) {
        m_themeAppliedOnce = true;
        applyTheme();
    }

    if (m_borderOverlay) {
        updateBorderOverlayGeometry();
        m_borderOverlay->raise();
        m_borderOverlay->show();
        m_borderOverlay->update();
    }
    update();
}

void FluentTextEdit::updateBorderOverlayGeometry()
{
    if (!m_borderOverlay) {
        return;
    }
    m_borderOverlay->setGeometry(rect());
}

bool FluentTextEdit::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == viewport() && event) {
        if (event->type() == QEvent::Enter || event->type() == QEvent::HoverEnter) {
            startHoverAnimation(1.0);
        } else if (event->type() == QEvent::Leave || event->type() == QEvent::HoverLeave) {
            startHoverAnimation(0.0);
        }
    }

    return QTextEdit::eventFilter(watched, event);
}

void FluentTextEdit::enterEvent(FluentEnterEvent *event)
{
    QTextEdit::enterEvent(event);
    startHoverAnimation(1.0);
}

void FluentTextEdit::leaveEvent(QEvent *event)
{
    QTextEdit::leaveEvent(event);
    startHoverAnimation(0.0);
}

void FluentTextEdit::focusInEvent(QFocusEvent *event)
{
    QTextEdit::focusInEvent(event);
    startFocusAnimation(1.0);
}

void FluentTextEdit::focusOutEvent(QFocusEvent *event)
{
    QTextEdit::focusOutEvent(event);
    startFocusAnimation(0.0);
}

void FluentTextEdit::startHoverAnimation(qreal endValue)
{
    if (!m_hoverAnim) {
        return;
    }
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

void FluentTextEdit::startFocusAnimation(qreal endValue)
{
    if (!m_focusAnim) {
        return;
    }
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
