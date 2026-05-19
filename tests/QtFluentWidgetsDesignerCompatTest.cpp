#include "Fluent/FluentMainWindow.h"
#include "Fluent/FluentMenu.h"
#include "Fluent/FluentMenuBar.h"

#include "ui_DesignerCompatMainWindow.h"
#include "ui_DesignerPromotedMainWindow.h"

#include <QtTest/QtTest>

#include <QAction>
#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPointer>
#include <QStringList>
#include <QWidget>

using namespace Fluent;

namespace {

bool visualModeEnabled()
{
    return qEnvironmentVariableIntValue("QTFLUENT_DESIGNER_COMPAT_TEST_VISIBLE") != 0;
}

int visualHoldMs()
{
    bool ok = false;
    const int value = qEnvironmentVariableIntValue("QTFLUENT_DESIGNER_COMPAT_TEST_VISIBLE_MS", &ok);
    if (!ok) {
        return 3000;
    }
    return value < 0 ? 0 : value;
}

void showForInspection(QWidget *window, const QString &title)
{
    window->setWindowTitle(title);
    window->resize(900, 600);
    window->show();
    QCoreApplication::processEvents();

    if (visualModeEnabled()) {
        const int holdMs = visualHoldMs();
        if (holdMs == 0) {
            while (window->isVisible()) {
                QTest::qWait(50);
            }
            return;
        }
        QTest::qWait(holdMs);
        return;
    }

    // Even in automated/offscreen runs, exercise the actual show path once.
    QTest::qWait(20);
}

int visibleMenuBarCount(QWidget *window)
{
    int count = 0;
    const auto menuBars = window->findChildren<QMenuBar *>();
    for (QMenuBar *menuBar : menuBars) {
        if (menuBar && menuBar->isVisible()) {
            ++count;
        }
    }
    return count;
}

void verifyMenuBarOpensOneNonNativePopup(QMenuBar *menuBar)
{
    QVERIFY(menuBar != nullptr);
    QVERIFY(!menuBar->actions().isEmpty());

    QAction *firstAction = menuBar->actions().first();
    const QRect firstActionRect = menuBar->actionGeometry(firstAction);
    QVERIFY(firstActionRect.isValid());

    QTest::mouseClick(menuBar, Qt::LeftButton, Qt::NoModifier, firstActionRect.center());
    QTRY_VERIFY(QApplication::activePopupWidget() != nullptr);

    int popupCount = 0;
    int nativeMenuPopupCount = 0;
    QStringList popupDescriptions;
    for (QWidget *widget : QApplication::topLevelWidgets()) {
        if (!widget || !widget->isVisible()) {
            continue;
        }
        if (widget->windowFlags().testFlag(Qt::Popup)) {
            ++popupCount;
            popupDescriptions << QStringLiteral("%1/%2")
                                     .arg(QString::fromLatin1(widget->metaObject()->className()),
                                          widget->objectName());
        }
        if (qobject_cast<QMenu *>(widget)) {
            ++nativeMenuPopupCount;
        }
    }

    QVERIFY2(popupCount == 1, qPrintable(popupDescriptions.join(QStringLiteral(", "))));
    QCOMPARE(nativeMenuPopupCount, 0);

    QApplication::activePopupWidget()->close();
    QCoreApplication::processEvents();
}

} // namespace

class QtFluentWidgetsDesignerCompatTest final : public QObject
{
    Q_OBJECT

private slots:
    void designerPromotedFluentWindowAndMenuBarWork()
    {
        auto *window = new FluentMainWindow();
        Ui::DesignerPromotedMainWindow ui;

        // This fixture mirrors the recommended Qt Designer setup for issue #9:
        // the root QMainWindow is promoted to FluentMainWindow, the menubar to
        // FluentMenuBar, and menus to FluentMenu.
        ui.setupUi(window);
        window->setFluentMenuBar(ui.menubar);
        showForInspection(window, QStringLiteral("Promoted Fluent Designer Fixture"));

        QCOMPARE(window->fluentMenuBar(), ui.menubar);
        QVERIFY(qobject_cast<QMenuBar *>(static_cast<QMainWindow *>(window)->menuWidget()) == nullptr);
        QCOMPARE(visibleMenuBarCount(window), 1);
        if (!visualModeEnabled()) {
            verifyMenuBarOpensOneNonNativePopup(window->fluentMenuBar());
        }
        QVERIFY(qobject_cast<FluentMenuBar *>(ui.menubar) != nullptr);
        QVERIFY(qobject_cast<FluentMenu *>(ui.menuFile) != nullptr);
        QVERIFY(qobject_cast<FluentMenu *>(ui.menuView) != nullptr);
        QCOMPARE(window->fluentMenuBar()->actions().size(), 2);
        QVERIFY(window->fluentMenuBar()->actions().contains(ui.menuFile->menuAction()));
        QVERIFY(window->fluentMenuBar()->actions().contains(ui.menuView->menuAction()));

        auto *menuHelp = new FluentMenu(QStringLiteral("Help"), ui.menubar);
        ui.menubar->addAction(menuHelp->menuAction());
        QVERIFY(window->fluentMenuBar()->actions().contains(menuHelp->menuAction()));

        QPointer<QWidget> centralRef = ui.centralwidget;
        QPointer<FluentMenuBar> fluentMenuBarRef = ui.menubar;
        QPointer<FluentMenu> menuFileRef = ui.menuFile;

        delete window;
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();

        QVERIFY(centralRef.isNull());
        QVERIFY(fluentMenuBarRef.isNull());
        QVERIFY(menuFileRef.isNull());
    }

    void designerRootPromotedPlainMenuBarIsAdoptedSafely()
    {
        auto *window = new FluentMainWindow();
        Ui::DesignerCompatMainWindow ui;

        // DesignerCompatMainWindow.ui promotes only the root window. The menubar
        // and menus stay as native Qt widgets, so uic still calls
        // FluentMainWindow::setMenuBar(QMenuBar*) before adding menu actions.
        ui.setupUi(window);

        QTRY_VERIFY(window->fluentMenuBar() != nullptr);
        QTRY_COMPARE(window->fluentMenuBar()->actions().size(), 2);
        QVERIFY(window->fluentMenuBar()->actions().contains(ui.menuFile->menuAction()));
        QVERIFY(window->fluentMenuBar()->actions().contains(ui.menuView->menuAction()));

        // The compat path keeps the source ui->menubar alive and hidden while
        // forwarding its actions into the visible FluentMenuBar.
        QVERIFY(static_cast<QMainWindow *>(window)->menuWidget() != static_cast<QWidget *>(ui.menubar));
        QVERIFY(qobject_cast<QMenuBar *>(static_cast<QMainWindow *>(window)->menuWidget()) == nullptr);
        QCOMPARE(ui.menubar->parentWidget(), static_cast<QWidget *>(window));
        QVERIFY(!ui.menubar->isVisible());
        QVERIFY(ui.menubar->actions().isEmpty());

        auto *menuHelp = new QMenu(ui.menubar);
        menuHelp->setTitle(QStringLiteral("Help"));
        ui.menubar->addAction(menuHelp->menuAction());
        QTRY_VERIFY(window->fluentMenuBar()->actions().contains(menuHelp->menuAction()));
        QVERIFY(ui.menubar->actions().isEmpty());

        window->fluentMenuBar()->removeAction(ui.menuView->menuAction());
        QTRY_VERIFY(!window->fluentMenuBar()->actions().contains(ui.menuView->menuAction()));

        QPointer<QWidget> centralRef = ui.centralwidget;
        QPointer<QMenuBar> sourceMenuBarRef = ui.menubar;
        QPointer<FluentMenuBar> fluentMenuBarRef = window->fluentMenuBar();

        delete window;
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();

        QVERIFY(centralRef.isNull());
        QVERIFY(sourceMenuBarRef.isNull());
        QVERIFY(fluentMenuBarRef.isNull());
    }
};

QTEST_MAIN(QtFluentWidgetsDesignerCompatTest)
#include "QtFluentWidgetsDesignerCompatTest.moc"
