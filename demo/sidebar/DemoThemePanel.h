#pragma once

#include "../DemoHelpers.h"

#include <QPointer>
#include <QWidget>

#include "Fluent/FluentToast.h"

namespace Demo {

class DemoThemePanel final : public QWidget
{
    Q_OBJECT
public:
    explicit DemoThemePanel(QWidget *hostWindow,
                            QWidget *parent = nullptr,
                            bool showToastControls = true,
                            bool useSectionCards = false);

    Fluent::FluentToast::Position toastPosition() const;

signals:
    void toastPositionChanged(Fluent::FluentToast::Position position);
    void languageChanged(DemoLanguage language);

private:
    QPointer<QWidget> m_hostWindow;
    bool m_showToastControls = true;
    bool m_useSectionCards = false;
    Fluent::FluentToast::Position m_toastPosition = Fluent::FluentToast::Position::BottomRight;
};

} // namespace Demo
