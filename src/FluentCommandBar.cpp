#include "Fluent/FluentCommandBar.h"

#include "Fluent/FluentDropDownButton.h"
#include "Fluent/FluentMenu.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"
#include "Fluent/FluentToolButton.h"

#include <QAction>
#include <QAbstractButton>
#include <QHBoxLayout>
#include <QMenu>
#include <QPainter>

namespace Fluent {

FluentCommandBar::FluentCommandBar(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(42);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(6, 5, 6, 5);
    m_layout->setSpacing(6);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, QOverload<>::of(&FluentCommandBar::update));
}

QAction *FluentCommandBar::addAction(const QString &text)
{
    auto *action = new QAction(text, this);
    m_ownedActions.append(action);
    addCommand(action);
    return action;
}

QAction *FluentCommandBar::addAction(const QIcon &icon, const QString &text)
{
    auto *action = new QAction(icon, text, this);
    m_ownedActions.append(action);
    addCommand(action);
    return action;
}

void FluentCommandBar::addCommand(QAction *action)
{
    if (!action) {
        return;
    }

    QWidget::addAction(action);
    QWidget *button = createButtonForAction(action);
    m_actionWidgets.insert(action, button);
    m_layout->addWidget(button);
    syncActionWidget(action);
    connect(action, &QAction::changed, this, [this, action]() { syncActionWidget(action); });
}

void FluentCommandBar::addSeparator()
{
    auto *sep = new QWidget(this);
    sep->setFixedSize(1, 24);
    sep->setProperty("fluentCommandBarSeparator", true);
    m_layout->addWidget(sep);
}

void FluentCommandBar::addWidget(QWidget *widget)
{
    if (!widget) {
        return;
    }
    widget->setParent(this);
    m_layout->addWidget(widget);
}

void FluentCommandBar::clear()
{
    while (QLayoutItem *item = m_layout->takeAt(0)) {
        if (auto *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
    m_actionWidgets.clear();
    qDeleteAll(m_ownedActions);
    m_ownedActions.clear();
}

QSize FluentCommandBar::iconSize() const
{
    return m_iconSize;
}

void FluentCommandBar::setIconSize(const QSize &size)
{
    if (m_iconSize == size || !size.isValid()) {
        return;
    }
    m_iconSize = size;
    for (auto it = m_actionWidgets.constBegin(); it != m_actionWidgets.constEnd(); ++it) {
        syncActionWidget(it.key());
    }
}

void FluentCommandBar::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        for (auto it = m_actionWidgets.constBegin(); it != m_actionWidgets.constEnd(); ++it) {
            if (it.value()) {
                it.value()->setEnabled(isEnabled() && it.key()->isEnabled());
            }
        }
    }
}

void FluentCommandBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    const auto &colors = ThemeManager::instance().colors();

    QPainter p(this);
    if (!p.isActive()) {
        return;
    }
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    p.setPen(QPen(colors.border, 1.0));
    p.setBrush(Style::mix(colors.surface, colors.background, 0.18));
    p.drawRoundedRect(r, 8.0, 8.0);

    for (int i = 0; i < m_layout->count(); ++i) {
        QWidget *widget = m_layout->itemAt(i)->widget();
        if (!widget || !widget->property("fluentCommandBarSeparator").toBool()) {
            continue;
        }
        p.setPen(QPen(colors.border, 1.0));
        const QRect wr = widget->geometry();
        p.drawLine(QPointF(wr.center().x(), wr.top()), QPointF(wr.center().x(), wr.bottom()));
    }
}

QWidget *FluentCommandBar::createButtonForAction(QAction *action)
{
    QWidget *button = nullptr;
    if (action->menu()) {
        auto *drop = new FluentDropDownButton(action->text(), this);
        drop->setMenu(action->menu());
        button = drop;
    } else {
        auto *tool = new FluentToolButton(action->text(), this);
        tool->setDefaultAction(action);
        tool->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button = tool;
    }

    syncActionWidget(action);
    if (auto *tool = qobject_cast<QAbstractButton *>(button)) {
        tool->setIcon(action->icon());
        tool->setIconSize(m_iconSize);
        tool->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    if (button) {
        button->adjustSize();
    }
    return button;
}

void FluentCommandBar::syncActionWidget(QAction *action)
{
    if (!action || !m_actionWidgets.contains(action)) {
        return;
    }

    QWidget *widget = m_actionWidgets.value(action);
    if (!widget) {
        return;
    }

    widget->setVisible(action->isVisible());
    widget->setEnabled(isEnabled() && action->isEnabled());
    widget->setToolTip(action->toolTip());

    if (auto *button = qobject_cast<QAbstractButton *>(widget)) {
        button->setText(action->text());
        button->setIcon(action->icon());
        button->setIconSize(m_iconSize);
    }
}

} // namespace Fluent
