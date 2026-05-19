#include "PageWindows.h"

#include "../DemoHelpers.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "Fluent/FluentButton.h"
#include "Fluent/FluentCard.h"
#include "Fluent/FluentDialog.h"
#include "Fluent/FluentFlyout.h"
#include "Fluent/FluentInfoBar.h"
#include "Fluent/FluentLabel.h"
#include "Fluent/FluentMainWindow.h"
#include "Fluent/FluentMenu.h"
#include "Fluent/FluentMessageBox.h"
#include "Fluent/FluentScrollArea.h"
#include "Fluent/FluentToast.h"
#include "Fluent/FluentTextEdit.h"
#include "Fluent/FluentTeachingTip.h"
#include "Fluent/FluentToggleSwitch.h"
#include "Fluent/FluentWidget.h"

#include <QCursor>

namespace Demo::Pages {

using namespace Fluent;

QWidget *createWindowsPage(FluentMainWindow *window)
{
    return Demo::makePage([&](QVBoxLayout *page) {
        auto s = Demo::makeSection(DEMO_TEXT("窗口 / 对话框", "Windows / Dialogs"),
                                   DEMO_TEXT("FluentDialog 可选 resize；FluentMessageBox 四种类型", "FluentDialog with optional resize; FluentMessageBox in four variants"));
        page->addWidget(s.card);

        // InfoBar
        {
            QString code;
#define WINDOWS_INFOBAR(X) \
    X(auto *info = new FluentInfoBar(FluentInfoBar::Severity::Info, DEMO_TEXT("同步完成", "Sync complete"), DEMO_TEXT("所有控件样式已跟随当前主题刷新。", "All controls have refreshed with the current theme."));) \
    X(info->setActionText(DEMO_TEXT("查看", "View"));) \
    X(auto *warn = new FluentInfoBar(FluentInfoBar::Severity::Warning, DEMO_TEXT("注意", "Heads up"), DEMO_TEXT("这是一个可关闭的信息提示条。", "This is a closable inline information bar."));) \
    X(body->addWidget(info);) \
    X(body->addWidget(warn);)

#define X(line) code += QStringLiteral(#line "\n");
            WINDOWS_INFOBAR(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("FluentInfoBar"),
                DEMO_TEXT("内嵌信息条：Info / Success / Warning / Error", "Inline information bar: info / success / warning / error"),
                DEMO_TEXT("要点：\n"
                          "-setSeverity() 切换语义色\n"
                          "-setActionText() 增加轻量操作按钮\n"
                          "-closed()/actionTriggered() 处理关闭与操作",
                          "Highlights:\n"
                          "-Use setSeverity() to switch semantic colors\n"
                          "-Use setActionText() to add a lightweight action button\n"
                          "-Handle closed() / actionTriggered() for dismissal and action"),
                code,
                [=](QVBoxLayout *body) {
#define X(line) line
                    WINDOWS_INFOBAR(X)
#undef X
                },
                false,
                210));

#undef WINDOWS_INFOBAR
        }

        // FluentMessageBox
        {
            QString code;
#define WINDOWS_MSGBOX(X) \
    X(auto *row = new QHBoxLayout(); ) \
    X(row->setContentsMargins(0, 0, 0, 0); ) \
    X(row->setSpacing(10); ) \
    X(auto *maskToggle = new FluentToggleSwitch(DEMO_TEXT("启用蒙版", "Enable mask")); ) \
    X(maskToggle->setChecked(false); ) \
    X(auto *openQuestion = new FluentButton(QStringLiteral("Question")); ) \
    X(auto *openWarn = new FluentButton(QStringLiteral("Warning")); ) \
    X(auto *openErr = new FluentButton(QStringLiteral("Error")); ) \
    X(row->addWidget(maskToggle); ) \
    X(row->addWidget(openQuestion); ) \
    X(row->addWidget(openWarn); ) \
    X(row->addWidget(openErr); ) \
    X(row->addStretch(1); ) \
    X(body->addLayout(row); ) \
    X(QObject::connect(openQuestion, &QPushButton::clicked, window, [=]() { FluentMessageBox::question(window, DEMO_TEXT("确认", "Confirm"), DEMO_TEXT("你觉得这个 demo 展示够全面吗？", "Do you think this demo covers the library well enough?"), DEMO_TEXT("切换主题/Accent 观察全控件联动。", "Switch Theme or Accent to observe the full control linkage."), maskToggle->isChecked()); }); ) \
    X(QObject::connect(openWarn, &QPushButton::clicked, window, [=]() { FluentMessageBox::warning(window, DEMO_TEXT("警告", "Warning"), DEMO_TEXT("这是 Warning 示例", "This is a Warning example"), DEMO_TEXT("用于演示 message box 样式与主题联动。", "It demonstrates MessageBox styling and theme linkage."), maskToggle->isChecked()); }); ) \
    X(QObject::connect(openErr, &QPushButton::clicked, window, [=]() { FluentMessageBox::error(window, DEMO_TEXT("错误", "Error"), DEMO_TEXT("这是 Error 示例", "This is an Error example"), DEMO_TEXT("用于演示 error 色与边框/阴影。", "It demonstrates error colors plus border and shadow treatment."), maskToggle->isChecked()); }); )

#define X(line) code += QStringLiteral(#line "\n");
            WINDOWS_MSGBOX(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("FluentMessageBox"),
                DEMO_TEXT("四种类型：info/question/warning/error", "Four variants: info / question / warning / error"),
                DEMO_TEXT("要点：\n"
                          "-静态函数一行调用\n"
                          "-可传入 maskEnabled 控制是否启用 overlay",
                          "Highlights:\n"
                          "-Use the static helpers for one-line calls\n"
                          "-Pass maskEnabled to control the overlay"),
                code,
                [=](QVBoxLayout *body) {
#define X(line) line
                    WINDOWS_MSGBOX(X)
#undef X
                },
                false,
                210));

#undef WINDOWS_MSGBOX
        }

        // FluentDialog (Basics)
        {
            QString code;
#define WINDOWS_DIALOG_BASIC(X) \
    X(auto *row = new QHBoxLayout(); ) \
    X(row->setContentsMargins(0, 0, 0, 0); ) \
    X(row->setSpacing(10); ) \
    X(auto *maskToggle = new FluentToggleSwitch(DEMO_TEXT("启用蒙版", "Enable mask")); ) \
    X(maskToggle->setChecked(false); ) \
    X(auto *resizeToggle = new FluentToggleSwitch(DEMO_TEXT("启用 Resize", "Enable resize")); ) \
    X(resizeToggle->setChecked(false); ) \
    X(auto *openDialog = new FluentButton(DEMO_TEXT("打开 FluentDialog", "Open FluentDialog")); ) \
    X(row->addWidget(maskToggle); ) \
    X(row->addWidget(resizeToggle); ) \
    X(row->addWidget(openDialog); ) \
    X(row->addStretch(1); ) \
    X(body->addLayout(row); ) \
    X(QObject::connect(openDialog, &QPushButton::clicked, window, [=]() { FluentDialog dlg(window); dlg.setWindowTitle(QStringLiteral("FluentDialog")); dlg.setModal(true); dlg.setMaskEnabled(maskToggle->isChecked()); dlg.setFluentResizeEnabled(resizeToggle->isChecked()); dlg.resize(520, 320); auto *content = new FluentWidget(&dlg); content->setBackgroundRole(FluentWidget::BackgroundRole::Transparent); auto *l = new QVBoxLayout(content); l->setContentsMargins(18, 18, 18, 18); l->setSpacing(12); auto *title = new FluentLabel(DEMO_TEXT("对话框内容", "Dialog content")); title->setStyleSheet("font-size: 16px; font-weight: 600;"); auto *hint = new FluentLabel(DEMO_TEXT("这是一个简化示例：只演示蒙版与 Resize 开关。", "This simplified example only demonstrates the mask and resize switches.")); hint->setStyleSheet("font-size: 12px; opacity: 0.85;"); auto *closeBtn = new FluentButton(DEMO_TEXT("关闭", "Close")); closeBtn->setPrimary(true); QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept); l->addWidget(title); l->addWidget(hint); l->addStretch(1); l->addWidget(closeBtn); auto *dlgLayout = new QVBoxLayout(&dlg); dlgLayout->setContentsMargins(0, 0, 0, 0); dlgLayout->addWidget(content); dlg.exec(); }); )

#define X(line) code += QStringLiteral(#line "\n");
            WINDOWS_DIALOG_BASIC(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("FluentDialog"),
                DEMO_TEXT("对话框容器：可选蒙版 overlay、可选 Resize", "Dialog container with optional overlay mask and optional resize"),
                DEMO_TEXT("要点：\n"
                          "-setMaskEnabled(true) 开启蒙版\n"
                          "-setFluentResizeEnabled(true) 开启可调整大小",
                          "Highlights:\n"
                          "-Use setMaskEnabled(true) to enable the overlay\n"
                          "-Use setFluentResizeEnabled(true) to make the dialog resizable"),
                code,
                [=](QVBoxLayout *body) {
#define X(line) line
                    WINDOWS_DIALOG_BASIC(X)
#undef X
                },
                true,
                240));

#undef WINDOWS_DIALOG_BASIC
        }

        // FluentDialog (Window Buttons)
        {
            QString code;
#define WINDOWS_DIALOG_BUTTONS(X) \
    X(auto *row = new QHBoxLayout(); ) \
    X(row->setContentsMargins(0, 0, 0, 0); ) \
    X(row->setSpacing(10); ) \
    X(auto *openDialog = new FluentButton(DEMO_TEXT("窗口按钮示例", "Window buttons example")); ) \
    X(row->addWidget(openDialog); ) \
    X(row->addStretch(1); ) \
    X(body->addLayout(row); ) \
    X(QObject::connect(openDialog, &QPushButton::clicked, window, [=]() { FluentDialog dlg(window); dlg.setWindowTitle(QStringLiteral("Window Buttons")); dlg.setModal(true); dlg.resize(520, 340); auto *content = new FluentWidget(&dlg); content->setBackgroundRole(FluentWidget::BackgroundRole::Transparent); auto *l = new QVBoxLayout(content); l->setContentsMargins(18, 18, 18, 18); l->setSpacing(10); auto *minBtnToggle = new FluentToggleSwitch(DEMO_TEXT("显示最小化按钮", "Show minimize button")); auto *maxBtnToggle = new FluentToggleSwitch(DEMO_TEXT("显示最大化按钮", "Show maximize button")); auto *closeBtnToggle = new FluentToggleSwitch(DEMO_TEXT("显示关闭按钮", "Show close button")); minBtnToggle->setChecked(true); maxBtnToggle->setChecked(true); closeBtnToggle->setChecked(true); auto updateDialogButtons = [&dlg, minBtnToggle, maxBtnToggle, closeBtnToggle]() { FluentDialog::WindowButtons buttons; if (minBtnToggle->isChecked()) buttons |= FluentDialog::MinimizeButton; if (maxBtnToggle->isChecked()) buttons |= FluentDialog::MaximizeButton; if (closeBtnToggle->isChecked()) buttons |= FluentDialog::CloseButton; dlg.setFluentWindowButtons(buttons); }; QObject::connect(minBtnToggle, &FluentToggleSwitch::toggled, &dlg, [&](bool) { updateDialogButtons(); }); QObject::connect(maxBtnToggle, &FluentToggleSwitch::toggled, &dlg, [&](bool) { updateDialogButtons(); }); QObject::connect(closeBtnToggle, &FluentToggleSwitch::toggled, &dlg, [&](bool) { updateDialogButtons(); }); auto *closeBtn = new FluentButton(DEMO_TEXT("关闭", "Close")); closeBtn->setPrimary(true); QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept); updateDialogButtons(); l->addWidget(minBtnToggle); l->addWidget(maxBtnToggle); l->addWidget(closeBtnToggle); l->addStretch(1); l->addWidget(closeBtn); auto *dlgLayout = new QVBoxLayout(&dlg); dlgLayout->setContentsMargins(0, 0, 0, 0); dlgLayout->addWidget(content); dlg.exec(); }); )

#define X(line) code += QStringLiteral(#line "\n");
            WINDOWS_DIALOG_BUTTONS(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("FluentDialog"),
                DEMO_TEXT("窗口按钮：最小化/最大化/关闭", "Window buttons: minimize / maximize / close"),
                DEMO_TEXT("要点：\n"
                          "-setFluentWindowButtons() 控制窗口按钮组合\n"
                          "-常用于需要自定义标题栏行为的窗口/对话框",
                          "Highlights:\n"
                          "-Use setFluentWindowButtons() to control the button combination\n"
                          "-Useful for windows or dialogs with custom title-bar behavior"),
                code,
                [=](QVBoxLayout *body) {
#define X(line) line
                    WINDOWS_DIALOG_BUTTONS(X)
#undef X
                },
                true,
                250));

#undef WINDOWS_DIALOG_BUTTONS
        }

        // FluentMenu
        {
            QString code;
#define WINDOWS_MENU(X) \
    X(auto *row = new QHBoxLayout(); ) \
    X(row->setContentsMargins(0, 0, 0, 0); ) \
    X(row->setSpacing(10); ) \
    X(auto *openMenu = new FluentButton(DEMO_TEXT("弹出 FluentMenu", "Show FluentMenu")); ) \
    X(row->addWidget(openMenu); ) \
    X(row->addStretch(1); ) \
    X(body->addLayout(row); ) \
    X(QObject::connect(openMenu, &QPushButton::clicked, window, [window]() { auto *menu = new FluentMenu(window); menu->addAction(DEMO_TEXT("操作 A", "Action A")); menu->addAction(DEMO_TEXT("操作 B", "Action B")); auto *sub = menu->addFluentMenu(DEMO_TEXT("更多", "More")); sub->addAction(DEMO_TEXT("子菜单项", "Submenu item")); menu->exec(QCursor::pos()); menu->deleteLater(); }); )

#define X(line) code += QStringLiteral(#line "\n");
            WINDOWS_MENU(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("FluentMenu"),
                DEMO_TEXT("弹出菜单：支持子菜单", "Popup menu with submenu support"),
                DEMO_TEXT("要点：\n"
                          "-addAction() 添加菜单项\n"
                          "-addFluentMenu() 添加子菜单\n"
                          "-exec(QCursor::pos()) 弹出",
                          "Highlights:\n"
                          "-Use addAction() to add menu items\n"
                          "-Use addFluentMenu() to add a submenu\n"
                          "-Use exec(QCursor::pos()) to show the popup"),
                code,
                [=](QVBoxLayout *body) {
#define X(line) line
                    WINDOWS_MENU(X)
#undef X
                },
                true,
                210));

#undef WINDOWS_MENU
        }

        // Popup / Flyout / TeachingTip
        {
            QString code;
#define WINDOWS_POPUP_COMPARE(X) \
    X(auto *row = new QHBoxLayout(); ) \
    X(row->setContentsMargins(0, 0, 0, 0); ) \
    X(row->setSpacing(10); ) \
    X(auto *openPopup = new FluentButton(DEMO_TEXT("Popup：菜单", "Popup: menu")); ) \
    X(auto *openFlyout = new FluentButton(DEMO_TEXT("Flyout：设置面板", "Flyout: panel")); ) \
    X(auto *openTip = new FluentButton(DEMO_TEXT("TeachingTip：引导", "TeachingTip: guidance")); ) \
    X(row->addWidget(openPopup); ) \
    X(row->addWidget(openFlyout); ) \
    X(row->addWidget(openTip); ) \
    X(row->addStretch(1); ) \
    X(body->addLayout(row); ) \
    X(QObject::connect(openPopup, &QPushButton::clicked, window, [=]() { auto *menu = new FluentMenu(window); menu->addAction(DEMO_TEXT("剪切", "Cut")); menu->addAction(DEMO_TEXT("复制", "Copy")); menu->addSeparator(); auto *more = menu->addFluentMenu(DEMO_TEXT("更多", "More")); more->addAction(DEMO_TEXT("固定到顶部", "Pin to top")); QObject::connect(menu, &QMenu::aboutToHide, menu, &QObject::deleteLater); menu->popup(openPopup->mapToGlobal(QPoint(0, openPopup->height() + 4))); }); ) \
    X(QObject::connect(openFlyout, &QPushButton::clicked, window, [=]() { auto *flyout = new FluentFlyout(window); auto *panel = new FluentWidget(); panel->setBackgroundRole(FluentWidget::BackgroundRole::Transparent); auto *l = new QVBoxLayout(panel); l->setContentsMargins(4, 4, 4, 4); l->setSpacing(10); auto *title = new FluentLabel(DEMO_TEXT("当前视图设置", "Current view settings")); title->setStyleSheet(QStringLiteral("font-weight: 650;")); l->addWidget(title); l->addWidget(new FluentToggleSwitch(DEMO_TEXT("紧凑列表", "Compact list"))); l->addWidget(new FluentToggleSwitch(DEMO_TEXT("显示预览", "Show preview"))); auto *close = new FluentButton(DEMO_TEXT("应用", "Apply")); close->setPrimary(true); QObject::connect(close, &QPushButton::clicked, flyout, &QWidget::hide); l->addWidget(close, 0, Qt::AlignLeft); flyout->setContentWidget(panel); flyout->showFor(openFlyout); }); ) \
    X(QObject::connect(openTip, &QPushButton::clicked, window, [=]() { auto *tip = new FluentTeachingTip(window); tip->setTitle(DEMO_TEXT("试试新的命令入口", "Try the new command entry")); tip->setSubtitle(DEMO_TEXT("TeachingTip 用于解释某个目标控件，并给出一个明确的下一步。", "TeachingTip explains a target control and offers one clear next step.")); tip->setActionText(DEMO_TEXT("开始", "Start")); tip->setTarget(openTip); tip->open(); }); )

#define X(line) code += QStringLiteral(#line "\n");
            WINDOWS_POPUP_COMPARE(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("Popup / FluentFlyout / FluentTeachingTip"),
                DEMO_TEXT("命令弹层、上下文面板与教学提示的差异", "Differences between command popup, contextual flyout, and teaching tip"),
                DEMO_TEXT("要点：\n"
                          "-Popup/Menu 用于短命令或选择，不承载复杂表单\n"
                          "-FluentFlyout 是上下文小面板，适合放少量自定义 QWidget\n"
                          "-TeachingTip 用于解释某个目标控件，并提供明确下一步",
                          "Highlights:\n"
                          "-Popup/Menu is for short commands or choices, not complex forms\n"
                          "-FluentFlyout is a contextual mini panel for a small custom QWidget surface\n"
                          "-TeachingTip explains a target control and offers a clear next step"),
                code,
                [=](QVBoxLayout *body) {
#define X(line) line
                    WINDOWS_POPUP_COMPARE(X)
#undef X
                },
                false,
                330));

#undef WINDOWS_POPUP_COMPARE
        }

        // TeachingTip guided flow
        {
            QString code;
#define WINDOWS_TEACHING_TOUR(X) \
    X(auto *row = new QHBoxLayout(); ) \
    X(row->setContentsMargins(0, 0, 0, 0); ) \
    X(row->setSpacing(10); ) \
    X(auto *startTour = new FluentButton(DEMO_TEXT("开始引导", "Start tour")); ) \
    X(startTour->setPrimary(true); ) \
    X(auto *searchTarget = new FluentButton(DEMO_TEXT("搜索入口", "Search entry")); ) \
    X(auto *previewTarget = new FluentToggleSwitch(DEMO_TEXT("预览", "Preview")); ) \
    X(previewTarget->setChecked(true); ) \
    X(auto *exportTarget = new FluentButton(DEMO_TEXT("导出", "Export")); ) \
    X(auto *maskToggle = new FluentToggleSwitch(DEMO_TEXT("蒙版遮罩", "Mask")); ) \
    X(maskToggle->setChecked(true); ) \
    X(row->addWidget(startTour); ) \
    X(row->addWidget(searchTarget); ) \
    X(row->addWidget(previewTarget); ) \
    X(row->addWidget(exportTarget); ) \
    X(row->addWidget(maskToggle); ) \
    X(row->addStretch(1); ) \
    X(body->addLayout(row); ) \
    X(auto *note = new FluentLabel(DEMO_TEXT("这个示例按顺序展示 3 个 TeachingTip 引导节点，并可切换蒙版遮罩。", "This sample walks through 3 TeachingTip nodes and lets you toggle the mask overlay.")); ) \
    X(note->setWordWrap(true); ) \
    X(body->addWidget(note); ) \
    X(QObject::connect(startTour, &QPushButton::clicked, window, [=]() { auto *tip = new FluentTeachingTip(window); tip->setMaskEnabled(maskToggle->isChecked()); tip->setMaskOpacity(0.46); auto *previewContent = new FluentLabel(DEMO_TEXT("自定义 contentWidget：这里可以放一段轻量说明、状态或小控件。", "Custom contentWidget: place lightweight notes, state, or small controls here.")); previewContent->setWordWrap(true); tip->setGuideTargets({ searchTarget, previewTarget, exportTarget }); tip->setGuideStyles({ { DEMO_TEXT("1 / 3  搜索入口", "1 / 3  Search entry"), DEMO_TEXT("第一步可以把引导锚定到入口控件，告诉用户从哪里开始。", "First, anchor guidance to the entry point so users know where to start."), DEMO_TEXT("下一步", "Next") }, { DEMO_TEXT("2 / 3  预览开关", "2 / 3  Preview toggle"), DEMO_TEXT("第二步说明一个状态控件，蒙版会突出当前目标并阻止背景误点。", "Second, explain a state control; the mask highlights the target and blocks background clicks."), DEMO_TEXT("下一步", "Next"), DEMO_TEXT("上一步", "Back") }, { DEMO_TEXT("3 / 3  导出动作", "3 / 3  Export action"), DEMO_TEXT("最后一步给出明确完成动作，适合新功能或短流程引导。", "Finally, point to a clear finishing action for feature discovery or a short workflow."), DEMO_TEXT("完成", "Done"), DEMO_TEXT("上一步", "Back") } }); tip->setGuideContentWidgets({ nullptr, previewContent, nullptr }); QObject::connect(tip, &FluentTeachingTip::guideFinished, previewContent, &QObject::deleteLater); QObject::connect(tip, &FluentTeachingTip::guideFinished, tip, &QObject::deleteLater); tip->startGuide(); }); )

#define X(line) code += QStringLiteral(#line "\n");
            WINDOWS_TEACHING_TOUR(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("FluentTeachingTip Guided Flow"),
                DEMO_TEXT("多节点引导：3 步 TeachingTip 与可选蒙版", "Multi-node guidance: 3-step TeachingTip with optional mask"),
                DEMO_TEXT("要点：\n"
                          "-setGuideTargets() 设置引导控件列表\n"
                          "-setGuideStyles() 设置每一步标题、说明、按钮、上一步和位置\n"
                          "-setGuideContentWidgets() 为步骤提供自定义 contentWidget\n"
                          "-setMaskEnabled(true) 打开蒙版遮罩并高亮当前目标",
                          "Highlights:\n"
                          "-Use setGuideTargets() to set the target control list\n"
                          "-Use setGuideStyles() to set each step's title, message, action, back action, and placement\n"
                          "-Use setGuideContentWidgets() to provide custom contentWidget per step\n"
                          "-Use setMaskEnabled(true) to dim the host and highlight the current target"),
                code,
                [=](QVBoxLayout *body) {
#define X(line) line
                    WINDOWS_TEACHING_TOUR(X)
#undef X
                },
                false,
                360));

#undef WINDOWS_TEACHING_TOUR
        }

        // FluentToast
        {
            QString code;
#define WINDOWS_TOAST(X) \
    X(auto *row = new QHBoxLayout(); ) \
    X(row->setContentsMargins(0, 0, 0, 0); ) \
    X(row->setSpacing(10); ) \
    X(auto *openToast = new FluentButton(DEMO_TEXT("发一条 Toast", "Send a Toast")); ) \
    X(row->addWidget(openToast); ) \
    X(row->addStretch(1); ) \
    X(body->addLayout(row); ) \
    X(QObject::connect(openToast, &QPushButton::clicked, window, [window]() { FluentToast::showToast(window, QStringLiteral("Toast"), DEMO_TEXT("这是一条 Toast（点击可关闭）", "This is a Toast notification (click to dismiss)"), FluentToast::Position::BottomRight, 2400); }); )

#define X(line) code += QStringLiteral(#line "\n");
            WINDOWS_TOAST(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("FluentToast"),
                DEMO_TEXT("轻提示：位置/持续时间可配置", "Lightweight notification with configurable position and duration"),
                DEMO_TEXT("要点：\n"
                          "-showToast(host, title, message, pos, durationMs)\n"
                          "-点击可关闭（默认行为）",
                          "Highlights:\n"
                          "-Use showToast(host, title, message, pos, durationMs)\n"
                          "-Click to dismiss is enabled by default"),
                code,
                [=](QVBoxLayout *body) {
#define X(line) line
                    WINDOWS_TOAST(X)
#undef X
                },
                true,
                200));

#undef WINDOWS_TOAST
        }

        page->addWidget(Demo::makeCollapsedCard(
            QStringLiteral("Window Chrome"),
            DEMO_TEXT("窗口框架相关控件（适合在主窗口中演示）", "Window-shell related controls (best demonstrated from the main window)"),
            DEMO_TEXT("包含：\n"
                      "-FluentMainWindow\n"
                      "-FluentMenuBar / FluentToolBar / FluentStatusBar\n"
                      "-FluentResizeHelper\n"
                      "-FluentTheme / FluentStyle",
                      "Includes:\n"
                      "-FluentMainWindow\n"
                      "-FluentMenuBar / FluentToolBar / FluentStatusBar\n"
                      "-FluentResizeHelper\n"
                      "-FluentTheme / FluentStyle"),
            QStringLiteral(
                "// 通常在 FluentMainWindow 里组合使用\n"
                "// 并监听 ThemeManager::themeChanged 做联动\n")));
    });
}

} // namespace Demo::Pages
