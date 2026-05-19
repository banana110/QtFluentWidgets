#pragma once

#include <QHash>
#include <QPointer>
#include <QWidget>

class QAction;
class QHBoxLayout;
class QIcon;

namespace Fluent {

class FluentCommandBar final : public QWidget
{
    Q_OBJECT
public:
    explicit FluentCommandBar(QWidget *parent = nullptr);

    QAction *addAction(const QString &text);
    QAction *addAction(const QIcon &icon, const QString &text);
    void addCommand(QAction *action);
    void addSeparator();
    void addWidget(QWidget *widget);
    void clear();

    QSize iconSize() const;
    void setIconSize(const QSize &size);

protected:
    void changeEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QWidget *createButtonForAction(QAction *action);
    void syncActionWidget(QAction *action);

    QHBoxLayout *m_layout = nullptr;
    QSize m_iconSize = QSize(16, 16);
    QList<QPointer<QAction>> m_ownedActions;
    QHash<QAction *, QPointer<QWidget>> m_actionWidgets;
};

} // namespace Fluent
