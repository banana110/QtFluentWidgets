#include "Fluent/FluentFlyout.h"

#include "Fluent/FluentPopupSurface.h"
#include "Fluent/FluentTheme.h"

#include <QApplication>
#include <QCursor>
#include <QHideEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QScreen>
#include <QShowEvent>
#include <QVBoxLayout>

namespace Fluent {

FluentFlyout::FluentFlyout(QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint)
    , m_border(this, this)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(12, 12, 12, 12);
    m_layout->setSpacing(8);

    m_border.setRequestUpdate([this]() { update(); });
    m_border.syncFromTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        if (isVisible()) {
            m_border.onThemeChanged();
        } else {
            m_border.syncFromTheme();
        }
        update();
    });
}

FluentFlyout::~FluentFlyout() = default;

QWidget *FluentFlyout::contentWidget() const
{
    return m_content.data();
}

void FluentFlyout::setContentWidget(QWidget *widget)
{
    if (m_content == widget) {
        return;
    }

    if (m_content) {
        m_layout->removeWidget(m_content);
        m_content->setParent(nullptr);
    }

    m_content = widget;
    if (m_content) {
        m_content->setParent(this);
        m_layout->addWidget(m_content);
    }
    adjustSize();
}

QMargins FluentFlyout::contentMargins() const
{
    return m_layout->contentsMargins();
}

void FluentFlyout::setContentMargins(const QMargins &margins)
{
    m_layout->setContentsMargins(margins);
}

void FluentFlyout::showAt(const QPoint &globalPos)
{
    adjustSize();
    startOpenAnimation(globalPos);
}

void FluentFlyout::showFor(QWidget *target, Placement placement)
{
    if (!target) {
        showAt(QCursor::pos());
        return;
    }

    adjustSize();
    const QRect anchor(target->mapToGlobal(QPoint(0, 0)), target->size());
    startOpenAnimation(placedPosition(anchor, sizeHint(), placement));
}

void FluentFlyout::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    setWindowOpacity(1.0);
    m_border.resetInitial();
}

void FluentFlyout::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    const auto &colors = ThemeManager::instance().colors();
    {
        QPainter clear(this);
        if (!clear.isActive()) {
            return;
        }
        clear.setCompositionMode(QPainter::CompositionMode_Source);
        clear.fillRect(rect(), Qt::transparent);
    }

    QPainter p(this);
    if (!p.isActive()) {
        return;
    }
    p.setRenderHint(QPainter::Antialiasing, true);
    PopupSurface::paintPanel(p, rect(), colors, &m_border);
}

void FluentFlyout::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    m_border.playInitialTraceOnce(0);
}

QPoint FluentFlyout::placedPosition(const QRect &anchor, const QSize &popupSize, Placement placement) const
{
    constexpr int gap = 8;
    QPoint pos;
    switch (placement) {
    case Placement::Top:
        pos = QPoint(anchor.center().x() - popupSize.width() / 2, anchor.top() - popupSize.height() - gap);
        break;
    case Placement::Right:
        pos = QPoint(anchor.right() + gap, anchor.center().y() - popupSize.height() / 2);
        break;
    case Placement::Left:
        pos = QPoint(anchor.left() - popupSize.width() - gap, anchor.center().y() - popupSize.height() / 2);
        break;
    case Placement::Bottom:
    default:
        pos = QPoint(anchor.center().x() - popupSize.width() / 2, anchor.bottom() + gap);
        break;
    }

    QScreen *screen = QApplication::screenAt(anchor.center());
    if (!screen) {
        screen = QApplication::primaryScreen();
    }
    const QRect avail = screen ? screen->availableGeometry() : QRect();
    if (avail.isValid()) {
        pos.setX(qBound(avail.left() + 4, pos.x(), avail.right() - popupSize.width() - 4));
        pos.setY(qBound(avail.top() + 4, pos.y(), avail.bottom() - popupSize.height() - 4));
    }
    return pos;
}

void FluentFlyout::startOpenAnimation(const QPoint &finalPos)
{
    if (m_fadeAnim) {
        m_fadeAnim->stop();
        m_fadeAnim->deleteLater();
    }
    if (m_slideAnim) {
        m_slideAnim->stop();
        m_slideAnim->deleteLater();
    }

    move(finalPos + QPoint(0, PopupSurface::kOpenSlideOffsetPx));
    setWindowOpacity(0.0);
    show();

    m_fadeAnim = new QPropertyAnimation(this, "windowOpacity", this);
    m_fadeAnim->setDuration(PopupSurface::kOpenDurationMs);
    m_fadeAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_fadeAnim->setStartValue(0.0);
    m_fadeAnim->setEndValue(1.0);

    m_slideAnim = new QPropertyAnimation(this, "pos", this);
    m_slideAnim->setDuration(PopupSurface::kOpenDurationMs);
    m_slideAnim->setEasingCurve(QEasingCurve::OutCubic);
    m_slideAnim->setStartValue(pos());
    m_slideAnim->setEndValue(finalPos);

    connect(m_fadeAnim, &QPropertyAnimation::finished, this, [this]() {
        m_fadeAnim->deleteLater();
        m_fadeAnim = nullptr;
    });
    connect(m_slideAnim, &QPropertyAnimation::finished, this, [this]() {
        m_slideAnim->deleteLater();
        m_slideAnim = nullptr;
    });

    m_fadeAnim->start();
    m_slideAnim->start();
}

} // namespace Fluent
