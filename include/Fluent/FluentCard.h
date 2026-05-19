#pragma once

#include <QByteArray>
#include <QString>
#include <QWidget>

class QVBoxLayout;
class QPropertyAnimation;

namespace Fluent {

class FluentCard final : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool collapsible READ isCollapsible WRITE setCollapsible)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool collapsed READ isCollapsed WRITE setCollapsed NOTIFY collapsedChanged)
public:
    explicit FluentCard(QWidget *parent = nullptr);

    void setCollapsible(bool on);
    bool isCollapsible() const;

    void setTitle(const QString &title);
    QString title() const;

    void setCollapsed(bool collapsed);
    bool isCollapsed() const;

    void setCollapseAnimationEnabled(bool enabled);
    bool isCollapseAnimationEnabled() const;

    bool loadCollapseIndicatorAnimation(const QString &path);
    bool loadCollapseIndicatorAnimationData(const QByteArray &json,
                                            const QString &cacheKey = QString(),
                                            const QString &resourcePath = QString());
    void clearCollapseIndicatorAnimation();

    QWidget *contentWidget() const;
    QVBoxLayout *contentLayout() const;

signals:
    void collapsedChanged(bool collapsed);

protected:
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void applyTheme();
    void ensureCollapsibleUi();
    void updateHeaderUi();
    void updateCollapseIndicatorState(bool animated);
    void updateCollapseIndicatorGeometry();
    void syncCollapseIndicatorSpeed(int startFrame, int endFrame);
    void applyCollapsedState(bool animated);

    bool m_collapsible = false;
    bool m_collapsed = false;
    bool m_animateCollapse = true;
    QString m_title;

    QWidget *m_header = nullptr;
    class FluentLabel *m_titleLabel = nullptr;
    class FluentToolButton *m_chevronButton = nullptr;
    class FluentAnimatedIcon *m_chevronAnimation = nullptr;
    bool m_hasChevronAnimation = false;
    QWidget *m_content = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QPropertyAnimation *m_collapseAnim = nullptr;
};

} // namespace Fluent
