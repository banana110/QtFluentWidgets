#pragma once

#include "DemoHelpers.h"
#include "Fluent/FluentScrollArea.h"
#include "Fluent/FluentToast.h"

#include <QPointer>

class QStringListModel;

namespace Fluent {
class FluentAnnotatedScrollBar;
class FluentListView;
}

namespace Demo {

class DemoSidebar final : public Fluent::FluentScrollArea
{
    Q_OBJECT
public:
    explicit DemoSidebar(QWidget *hostWindow, QWidget *parent = nullptr, bool showNavigation = true);

    void setCurrentPage(int index);
    int currentPage() const;

    Fluent::FluentToast::Position toastPosition() const;

signals:
    void currentPageChanged(int index);
    void toastPositionChanged(Fluent::FluentToast::Position position);
    void languageChanged(DemoLanguage language);

private:
    void resizeEvent(QResizeEvent *event) override;
    void refreshAnnotatedSources();

    QPointer<QWidget> m_hostWindow;
    QStringListModel *m_navModel = nullptr;
    Fluent::FluentListView *m_navView = nullptr;
    Fluent::FluentAnnotatedScrollBar *m_annotatedScrollBar = nullptr;

    Fluent::FluentToast::Position m_toastPosition = Fluent::FluentToast::Position::BottomRight;
};

} // namespace Demo
