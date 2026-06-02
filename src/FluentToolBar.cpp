#include "Fluent/FluentToolBar.h"
#include "Fluent/FluentTheme.h"
#include "Fluent/FluentToolButton.h"

#include <QEvent>
#include <QActionEvent>
#include <QWidgetAction>

#include <utility>

namespace Fluent {

FluentToolBar::FluentToolBar(const QString &title, QWidget *parent)
    : QToolBar(title, parent)
{
    setMovable(false);
    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentToolBar::applyTheme);
}

FluentToolBar::~FluentToolBar()
{
    for (const QMetaObject::Connection &connection : std::as_const(m_actionDestroyConnections)) {
        QObject::disconnect(connection);
    }
    m_actionDestroyConnections.clear();
    m_wrapperActions.clear();
    m_wrappedActions.clear();
}

void FluentToolBar::insertAction(QAction *before, QAction *action)
{
    QWidgetAction *wrappedBefore = wrapperForAction(before);
    QToolBar::insertAction(wrappedBefore ? wrappedBefore : before, action);
}

void FluentToolBar::removeAction(QAction *action)
{
    if (auto *wrapper = wrapperForAction(action)) {
        unregisterWrapper(wrapper);
        QToolBar::removeAction(wrapper);
        wrapper->deleteLater();
        return;
    }

    if (auto *wrapper = qobject_cast<QWidgetAction *>(action);
        wrapper && m_wrapperActions.contains(wrapper)) {
        unregisterWrapper(wrapper);
    }
    QToolBar::removeAction(action);
}

void FluentToolBar::changeEvent(QEvent *event)
{
    QToolBar::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        applyTheme();
    }
}

void FluentToolBar::actionEvent(QActionEvent *event)
{
    QToolBar::actionEvent(event);

    if (event->type() == QEvent::ActionRemoved) {
        if (auto *wrapper = qobject_cast<QWidgetAction *>(event->action())) {
            unregisterWrapper(wrapper);
        }
        return;
    }

    if (!m_wrappingAction && event->type() == QEvent::ActionAdded) {
        QAction *action = event->action();
        if (!action || action->isSeparator() || qobject_cast<QWidgetAction *>(action)) {
            return;
        }

        if (qobject_cast<FluentToolButton *>(widgetForAction(action))) {
            return;
        }

        wrapPlainAction(action, event->before());
    }
}

void FluentToolBar::applyTheme()
{
    const QString next = Theme::toolBarStyle(ThemeManager::instance().colors());
    if (styleSheet() != next) {
        setStyleSheet(next);
    }
}

QWidgetAction *FluentToolBar::wrapperForAction(QAction *action) const
{
    const auto it = m_wrappedActions.constFind(action);
    if (it == m_wrappedActions.cend()) {
        return nullptr;
    }
    return it.value().data();
}

void FluentToolBar::unregisterWrapper(QWidgetAction *wrapper)
{
    if (!wrapper) {
        return;
    }

    const auto original = m_wrapperActions.take(wrapper);
    if (original) {
        QObject::disconnect(m_actionDestroyConnections.take(original.data()));
        m_wrappedActions.remove(original.data());
    }
}

void FluentToolBar::wrapPlainAction(QAction *action, QAction *before)
{
    if (!action || wrapperForAction(action)) {
        return;
    }

    QWidgetAction *wrappedBefore = wrapperForAction(before);
    QToolBar::removeAction(action);

    auto *button = new FluentToolButton(this);
    button->setDefaultAction(action);

    auto *widgetAction = new QWidgetAction(this);
    widgetAction->setDefaultWidget(button);

    m_wrappedActions.insert(action, widgetAction);
    m_wrapperActions.insert(widgetAction, action);

    const QMetaObject::Connection destroyedConnection =
        connect(action, &QObject::destroyed, this, [this, action, wrapper = QPointer<QWidgetAction>(widgetAction)] {
        m_actionDestroyConnections.remove(action);
        m_wrappedActions.remove(action);
        if (!wrapper) {
            return;
        }
        m_wrapperActions.remove(wrapper.data());
        if (actions().contains(wrapper.data())) {
            QToolBar::removeAction(wrapper.data());
        }
        wrapper->deleteLater();
    });
    m_actionDestroyConnections.insert(action, destroyedConnection);

    m_wrappingAction = true;
    QToolBar::insertAction(wrappedBefore ? wrappedBefore : before, widgetAction);
    m_wrappingAction = false;
}

} // namespace Fluent
