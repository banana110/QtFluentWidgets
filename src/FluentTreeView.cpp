#include "Fluent/FluentTreeView.h"
#include "Fluent/FluentMotion.h"
#include "Fluent/FluentScrollBar.h"
#include "Fluent/FluentTheme.h"
#include "FluentItemEditorSupport.h"
#include "FluentItemViewPaintSupport.h"
#include "FluentPaintSupport.h"
#include "FluentViewPaletteSupport.h"

#include <QAbstractItemModel>
#include <QEvent>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QVariantAnimation>
#include <QWindow>

namespace Fluent {

namespace {

class FluentHeaderView final : public QHeaderView
{
public:
    explicit FluentHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QHeaderView(orientation, parent)
    {
        setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }

protected:
    bool viewportEvent(QEvent *event) override
    {
        const bool baseResult = QHeaderView::viewportEvent(event);

        if (!event || event->type() != QEvent::Paint) {
            return baseResult;
        }

        if (orientation() != Qt::Horizontal) {
            return baseResult;
        }

        if (!Detail::canBeginWidgetPainter(viewport())) {
            return baseResult;
        }

        const auto &tokens = ThemeManager::instance().tokens();
        const QColor sep = tokens.neutral.strokeSubtle;

        QPainter painter(viewport());
        if (!painter.isActive()) {
            return baseResult;
        }
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setPen(QPen(sep, 1));

        const int inset = 6;
        const int y1 = inset;
        const int y2 = viewport()->height() - 1 - inset;
        if (y2 <= y1) {
            return baseResult;
        }

        for (int logical = 0; logical < count(); ++logical) {
            if (isSectionHidden(logical)) {
                continue;
            }
            const int w = sectionSize(logical);
            if (w <= 0) {
                continue;
            }
            const int left = sectionViewportPosition(logical);
            const int right = left + w - 1;
            if (logical == count() - 1) {
                continue;
            }

            if (right < 0 || right >= viewport()->width() - 1) {
                continue;
            }
            painter.drawLine(QPointF(right + 0.5, y1), QPointF(right + 0.5, y2));
        }

        return baseResult;
    }
};

class FluentTreeItemDelegate final : public QStyledItemDelegate
{
public:
    explicit FluentTreeItemDelegate(FluentTreeView *view)
        : QStyledItemDelegate(view)
        , m_view(view)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);
        opt.state &= ~QStyle::State_HasFocus; 

        const auto &colors = ThemeManager::instance().colors();
        
        // TreeView row selection logic:
        // Selection behavior usually is SelectRows.
        const bool isRowSelection = (m_view && m_view->selectionBehavior() == QAbstractItemView::SelectRows);
        const bool paintsRowBackground = !isRowSelection || isFirstVisibleColumn(index);

        QRectF bgRect = QRectF(opt.rect);
        if (isRowSelection && m_view && m_view->viewport()) {
            bgRect.setLeft(0);
            bgRect.setWidth(m_view->viewport()->width());
        }
        // Vertical breathing room
        bgRect.adjust(isRowSelection ? 2 : 0, 1, isRowSelection ? -2 : 0, -1);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QColor bgColor = Qt::transparent;
        
        const bool enabled = opt.state.testFlag(QStyle::State_Enabled);
        const bool isSelected = opt.state.testFlag(QStyle::State_Selected);
        bool isHovered = false;

        bool isCurrentRow = false;
        if (m_view) {
            const QModelIndex cur = m_view->currentIndex();
            if (cur.isValid()) {
                isCurrentRow = (cur.row() == index.row() && cur.parent() == index.parent());
            }
        }

        if (m_view && index.isValid()) {
             QModelIndex hover = m_view->hoverIndex();
             if (hover.isValid()) {
                 if (isRowSelection) {
                     // Check if same row (row + parent match)
                     if (index.row() == hover.row() && index.parent() == hover.parent()) {
                         isHovered = true;
                     }
                 } else {
                     if (index == hover) isHovered = true;
                 }
             }
        }

        if (isSelected && !isCurrentRow) {
            bgColor = enabled
                ? Detail::fluentItemSelectionFill(colors, 0.86)
                : Detail::fluentItemDisabledSelectionFill(colors, 0.86);
        } else if (enabled && isHovered) {
             bgColor = Detail::fluentItemHoverFill(colors, m_view->hoverLevel());
        }

        // Keep selected text color stable (Qt would otherwise use HighlightedText).
        if (isSelected) {
            opt.palette.setColor(QPalette::HighlightedText, opt.palette.color(QPalette::Text));
            opt.palette.setColor(QPalette::Highlight, Qt::transparent);
        }

        if (bgColor.alpha() > 0 && paintsRowBackground) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(bgColor);

            if (isRowSelection) {
                painter->drawRoundedRect(bgRect, 4, 4);
            } else {
                // Single cell
                painter->drawRoundedRect(bgRect.adjusted(2,0,-2,0), 4, 4);
            }
            
        }

        painter->restore();

        QStyledItemDelegate::paint(painter, opt, index);
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return Detail::fluentizeTextEditor(QStyledItemDelegate::createEditor(parent, option, index), parent);
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        if (Detail::setFluentEditorData(editor, index)) {
            return;
        }
        QStyledItemDelegate::setEditorData(editor, index);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        if (Detail::setFluentModelData(editor, model, index)) {
            return;
        }
        QStyledItemDelegate::setModelData(editor, model, index);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (Detail::isFluentTextEditor(editor)) {
            editor->setGeometry(Detail::fluentEditorRect(option, 2, 1));
            return;
        }
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    }

private:
    bool isFirstVisibleColumn(const QModelIndex &index) const
    {
        if (!m_view || !index.isValid() || !index.model()) {
            return true;
        }

        const int columns = index.model()->columnCount(index.parent());
        for (int column = 0; column < columns; ++column) {
            if (!m_view->isColumnHidden(column)) {
                return index.column() == column;
            }
        }
        return index.column() == 0;
    }

    FluentTreeView *m_view = nullptr;
};

} // namespace

FluentTreeView::FluentTreeView(QWidget *parent)
    : QTreeView(parent)
{
    setHeader(new FluentHeaderView(Qt::Horizontal, this));
    header()->setStretchLastSection(true);
    setFrameShape(QFrame::NoFrame);
    setAlternatingRowColors(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setIndentation(18);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
    setItemDelegate(new FluentTreeItemDelegate(this));

    setVerticalScrollBar(new FluentScrollBar(Qt::Vertical, this));
    setHorizontalScrollBar(new FluentScrollBar(Qt::Horizontal, this));

    m_hoverAnim = new QVariantAnimation(this);
    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    connect(m_hoverAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_hoverLevel = value.toReal();
        viewport()->update();
    });

    m_selAnim = new QVariantAnimation(this);
    FluentMotion::configure(m_selAnim, FluentMotionRole::Selection);
    connect(m_selAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        const qreal t = value.toReal();
        const qreal tt = qBound<qreal>(0.0, t, 1.0);
        m_selRect = QRectF(
            m_selStartRect.x() + (m_selTargetRect.x() - m_selStartRect.x()) * tt,
            m_selStartRect.y() + (m_selTargetRect.y() - m_selStartRect.y()) * tt,
            m_selStartRect.width() + (m_selTargetRect.width() - m_selStartRect.width()) * tt,
            m_selStartRect.height() + (m_selTargetRect.height() - m_selStartRect.height()) * tt);
        m_selOpacity = m_selStartOpacity + (m_selTargetOpacity - m_selStartOpacity) * tt;
        viewport()->update();
    });
    connect(m_selAnim, &QVariantAnimation::finished, this, [this]() {
        if (m_selTargetOpacity <= 0.0) {
            m_selRect = QRectF();
            m_selOpacity = 0.0;
        } else {
            m_selOpacity = 1.0;
        }
        viewport()->update();
    });
    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentTreeView::applyTheme);

    QTimer::singleShot(0, this, [this]() { hookSelectionModel(); });
}

QModelIndex FluentTreeView::hoverIndex() const
{
    return m_hoverIndex;
}

qreal FluentTreeView::hoverLevel() const
{
    return m_hoverLevel;
}

void FluentTreeView::changeEvent(QEvent *event)
{
    QTreeView::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        applyTheme();
    }
}

void FluentTreeView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
    hookSelectionModel();
}

void FluentTreeView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    QTreeView::setSelectionModel(selectionModel);
    hookSelectionModel();
}

void FluentTreeView::paintEvent(QPaintEvent *event)
{
    {
        const QModelIndex cur = currentIndex();
        const bool paintAnim = (m_selAnim && m_selAnim->state() == QAbstractAnimation::Running && m_selRect.isValid() && m_selOpacity > 0.0);
        const bool paintStatic = (cur.isValid() && selectionModel() && selectionModel()->isSelected(cur));
        if (paintAnim || paintStatic) {
            const auto &colors = ThemeManager::instance().colors();
            const QRectF r = paintAnim ? m_selRect : selectionRectForIndex(cur);
            const qreal opacity = paintAnim ? qBound<qreal>(0.0, m_selOpacity, 1.0) : 1.0;
            if (Detail::canBeginWidgetPainter(viewport())) {
                QPainter p(viewport());
                p.setRenderHint(QPainter::Antialiasing, true);

                p.setPen(Qt::NoPen);
                p.setBrush(isEnabled()
                               ? Detail::fluentItemSelectionFill(colors, opacity)
                               : Detail::fluentItemDisabledSelectionFill(colors, opacity));
                p.drawRoundedRect(r, 4.0, 4.0);

                if (isEnabled()) {
                    Detail::paintFluentItemSelectionIndicator(p, r, colors, opacity, 3.0);
                }
            }
        }
    }

    QTreeView::paintEvent(event);
}

void FluentTreeView::mouseMoveEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (index != m_hoverIndex) {
        m_hoverIndex = index;
        startHoverAnimation(index.isValid() ? 1.0 : 0.0);
    }
    QTreeView::mouseMoveEvent(event);
}

void FluentTreeView::leaveEvent(QEvent *event)
{
    startHoverAnimation(0.0);
    m_hoverIndex = QModelIndex();
    QTreeView::leaveEvent(event);
}

void FluentTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    // Draw default branch lines first (optional)
    // QTreeView::drawBranches(painter, rect, index);
    
    if (!index.isValid()) {
        return;
    }
    
    // Only draw chevron for items with children in first column
    if (index.column() == 0 && model() && model()->hasChildren(index)) {
        const auto &colors = ThemeManager::instance().colors();
        
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        
        // Use same pen style as ComboBox arrow
        QPen pen(colors.subText, 1.5);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        
        // Calculate center position in the branch rect
        int centerX = rect.center().x();
        int centerY = rect.center().y();
        
        QPainterPath path;
        if (isExpanded(index)) {
            // Down chevron (expanded): ∨ shape
            // M centerX-4,centerY-2 L centerX,centerY+2 L centerX+4,centerY-2
            path.moveTo(centerX - 4, centerY - 2);
            path.lineTo(centerX, centerY + 2);
            path.lineTo(centerX + 4, centerY - 2);
        } else {
            // Right chevron (collapsed): > shape  
            // M centerX-2,centerY-4 L centerX+2,centerY L centerX-2,centerY+4
            path.moveTo(centerX - 2, centerY - 4);
            path.lineTo(centerX + 2, centerY);
            path.lineTo(centerX - 2, centerY + 4);
        }
        
        painter->drawPath(path);
        painter->restore();
    }
}

void FluentTreeView::applyTheme()
{
    const bool hoverRunning = m_hoverAnim && m_hoverAnim->state() == QAbstractAnimation::Running;
    const bool selectionRunning = m_selAnim && m_selAnim->state() == QAbstractAnimation::Running;
    const QVariant hoverEnd = m_hoverAnim ? m_hoverAnim->endValue() : QVariant();

    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    FluentMotion::configure(m_selAnim, FluentMotionRole::Selection);
    if (hoverRunning && m_hoverAnim->duration() <= 0) {
        m_hoverAnim->stop();
        m_hoverLevel = qBound<qreal>(0.0, hoverEnd.toReal(), 1.0);
        viewport()->update();
    }
    if (selectionRunning && m_selAnim->duration() <= 0) {
        m_selAnim->stop();
        if (m_selTargetOpacity <= 0.0 || !m_selTargetRect.isValid()) {
            m_selRect = QRectF();
            m_selOpacity = 0.0;
        } else {
            m_selRect = m_selTargetRect;
            m_selOpacity = m_selTargetOpacity;
        }
        viewport()->update();
    }

    const auto &colors = ThemeManager::instance().colors();
    const QString next = Theme::treeViewStyle(colors);
    if (styleSheet() != next) {
        setStyleSheet(next);
    }
    Detail::applyFluentViewPalette(this, viewport(), colors);
    Detail::applyFluentViewPalette(header(), header() ? header()->viewport() : nullptr, colors);
}

void FluentTreeView::hookSelectionModel()
{
    if (m_selectionConnection) {
        QObject::disconnect(m_selectionConnection);
        m_selectionConnection = QMetaObject::Connection();
    }

    if (!selectionModel()) {
        m_selRect = QRectF();
        m_selStartRect = QRectF();
        m_selTargetRect = QRectF();
        m_selOpacity = 0.0;
        m_selStartOpacity = 0.0;
        m_selTargetOpacity = 0.0;
        return;
    }

    m_selectionConnection = connect(selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex &current, const QModelIndex &previous) {
        startSelectionAnimation(previous, current);
    });

    const QModelIndex cur = currentIndex();
    m_selRect = selectionRectForIndex(cur);
    m_selStartRect = m_selRect;
    m_selTargetRect = m_selRect;
    m_selOpacity = m_selRect.isValid() ? 1.0 : 0.0;
    m_selStartOpacity = m_selOpacity;
    m_selTargetOpacity = m_selOpacity;
}

QRectF FluentTreeView::selectionRectForIndex(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QRectF();
    }

    if (selectionBehavior() == QAbstractItemView::SelectRows) {
        const QRect r = visualRect(index);
        if (!r.isValid() || !viewport()) {
            return QRectF();
        }
        return QRectF(0, r.y(), viewport()->width(), r.height()).adjusted(2, 1, -2, -1);
    }

    const QRect r = visualRect(index);
    return r.isValid() ? QRectF(r).adjusted(2, 1, -2, -1) : QRectF();
}

void FluentTreeView::startSelectionAnimation(const QModelIndex &from, const QModelIndex &to)
{
    const bool animRunning = (m_selAnim && m_selAnim->state() == QAbstractAnimation::Running);
    const QRectF startRect = animRunning ? m_selRect : selectionRectForIndex(from);
    const qreal startOpacity = animRunning ? m_selOpacity : (startRect.isValid() ? 1.0 : 0.0);
    const QRectF targetRect = selectionRectForIndex(to);
    const bool targetValid = targetRect.isValid();
    const auto startSelectionMotion = [this]() {
        FluentMotion::configure(m_selAnim, FluentMotionRole::Selection);
        m_selAnim->stop();
        if (m_selAnim->duration() <= 0) {
            if (m_selTargetOpacity <= 0.0) {
                m_selRect = QRectF();
                m_selOpacity = 0.0;
            } else {
                m_selRect = m_selTargetRect;
                m_selOpacity = 1.0;
            }
            viewport()->update();
            return;
        }
        m_selAnim->setStartValue(0.0);
        m_selAnim->setEndValue(1.0);
        m_selAnim->start();
    };

    if (!targetValid) {
        if (!startRect.isValid()) {
            m_selRect = QRectF();
            m_selOpacity = 0.0;
            viewport()->update();
            return;
        }
        m_selStartRect = startRect;
        m_selTargetRect = startRect;
        m_selStartOpacity = startOpacity;
        m_selTargetOpacity = 0.0;
        m_selRect = startRect;
        m_selOpacity = startOpacity;
        startSelectionMotion();
        return;
    }

    if (!startRect.isValid()) {
        m_selStartRect = targetRect;
        m_selTargetRect = targetRect;
        m_selStartOpacity = 0.0;
        m_selTargetOpacity = 1.0;
        m_selRect = targetRect;
        m_selOpacity = 0.0;
        startSelectionMotion();
        return;
    }

    if (startRect == targetRect) {
        m_selRect = targetRect;
        m_selOpacity = 1.0;
        viewport()->update();
        return;
    }

    m_selStartRect = startRect;
    m_selTargetRect = targetRect;
    m_selStartOpacity = 0.75;
    m_selTargetOpacity = 1.0;
    m_selRect = startRect;
    m_selOpacity = m_selStartOpacity;

    startSelectionMotion();
}

void FluentTreeView::startHoverAnimation(qreal endValue)
{
    FluentMotion::configure(m_hoverAnim, FluentMotionRole::Hover);
    m_hoverAnim->stop();
    if (m_hoverAnim->duration() <= 0) {
        m_hoverLevel = qBound<qreal>(0.0, endValue, 1.0);
        viewport()->update();
        return;
    }
    m_hoverAnim->setStartValue(m_hoverLevel);
    m_hoverAnim->setEndValue(endValue);
    m_hoverAnim->start();
}

} // namespace Fluent
