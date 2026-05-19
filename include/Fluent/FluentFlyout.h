#pragma once

#include "Fluent/FluentBorderEffect.h"

#include <QPointer>
#include <QWidget>

class QPropertyAnimation;
class QVBoxLayout;

namespace Fluent {

class FluentFlyout : public QWidget
{
    Q_OBJECT
public:
    enum class Placement {
        Bottom,
        Top,
        Right,
        Left,
    };
    Q_ENUM(Placement)

    explicit FluentFlyout(QWidget *parent = nullptr);
    ~FluentFlyout() override;

    QWidget *contentWidget() const;
    void setContentWidget(QWidget *widget);

    QMargins contentMargins() const;
    void setContentMargins(const QMargins &margins);

public slots:
    void showAt(const QPoint &globalPos);
    void showFor(QWidget *target, Placement placement = Placement::Bottom);

protected:
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    QPoint placedPosition(const QRect &anchor, const QSize &popupSize, Placement placement) const;
    void startOpenAnimation(const QPoint &finalPos);

    QVBoxLayout *m_layout = nullptr;
    QPointer<QWidget> m_content;
    QPropertyAnimation *m_fadeAnim = nullptr;
    QPropertyAnimation *m_slideAnim = nullptr;
    FluentBorderEffect m_border;
};

} // namespace Fluent
