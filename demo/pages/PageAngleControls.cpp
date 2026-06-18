#include "PageAngleControls.h"

#include "../DemoHelpers.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QVBoxLayout>

#include "Fluent/FluentAngleSelector.h"
#include "Fluent/FluentCard.h"
#include "Fluent/FluentDial.h"
#include "Fluent/FluentLabel.h"
#include "Fluent/FluentMainWindow.h"
#include "Fluent/FluentScrollArea.h"

namespace Demo::Pages {

using namespace Fluent;

void fillAngleControls(QVBoxLayout *page, FluentMainWindow *window)
{
        Q_UNUSED(window)

        auto s = Demo::makeSection(DEMO_TEXT("角度控件", "Angle Controls"),
                                   DEMO_TEXT("FluentDial / FluentAngleSelector：指针、高亮、复合编辑器", "FluentDial / FluentAngleSelector: indicator, highlight, and composite editor"));
        page->addWidget(s.card);

        // Angle control state matrix
        {
            const QString code = QStringLiteral(
                "auto *dial = new FluentDial();\n"
                "dial->setValue(135);\n"
                "dial->setTicksVisible(true);\n"
                "dial->setMajorTickStep(45);\n"
                "auto *disabled = new FluentDial();\n"
                "disabled->setValue(270);\n"
                "disabled->setTicksVisible(true);\n"
                "disabled->setEnabled(false);\n");

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("Angle State Matrix"),
                DEMO_TEXT("Dial 与 AngleSelector 的 value / ticks / compact / disabled 横向对比",
                          "Side-by-side value / ticks / compact / disabled states for Dial and AngleSelector"),
                DEMO_TEXT("要点：\n"
                          "-Dial 行用于检查 accent 弧、刻度、指针和 disabled 弱化是否协调\n"
                          "-AngleSelector 行用于检查复合控件在完整、紧凑和禁用形态下的基线与密度\n"
                          "-可配合右侧 Theme / Accent 面板快速检查深浅色和强调色变化",
                          "Highlights:\n"
                          "-Dial rows inspect accent arcs, ticks, pointers, and disabled muting\n"
                          "-AngleSelector rows inspect baseline and density in full, compact, and disabled forms\n"
                          "-Use the Theme / Accent panel to check light, dark, and accent changes quickly"),
                code,
                [=](QVBoxLayout *body) {
                    auto *grid = new QGridLayout();
                    grid->setContentsMargins(0, 0, 0, 0);
                    grid->setHorizontalSpacing(14);
                    grid->setVerticalSpacing(10);

                    auto makeCaption = [](const QString &text, bool strong = false) {
                        auto *label = new FluentLabel(text);
                        label->setWordWrap(true);
                        label->setStyleSheet(strong
                                                 ? QStringLiteral("font-size: 12px; font-weight: 600; opacity: 0.9;")
                                                 : QStringLiteral("font-size: 12px; opacity: 0.78;"));
                        return label;
                    };

                    auto prepareCell = [](QWidget *widget, int width = 176) {
                        if (qobject_cast<FluentDial *>(widget)) {
                            widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                            return widget;
                        }
                        widget->setMinimumWidth(width);
                        widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
                        return widget;
                    };

                    auto makeDial = [](int value, bool ticks, bool pointer, bool enabled) {
                        auto *dial = new FluentDial();
                        dial->setValue(value);
                        dial->setTicksVisible(ticks);
                        dial->setMajorTickStep(45);
                        dial->setPointerVisible(pointer);
                        dial->setEnabled(enabled);
                        return dial;
                    };

                    auto makeSelector = [](int value, bool showLabel, bool showDial, bool showSpin, bool enabled) {
                        auto *selector = new FluentAngleSelector();
                        selector->setValue(value);
                        selector->setLabelText(DEMO_TEXT("角度：", "Angle:"));
                        selector->setLabelVisible(showLabel);
                        selector->setDialVisible(showDial);
                        selector->setSpinBoxVisible(showSpin);
                        selector->setEnabled(enabled);
                        return selector;
                    };

                    grid->addWidget(makeCaption(DEMO_TEXT("控件", "Control"), true), 0, 0);
                    grid->addWidget(makeCaption(DEMO_TEXT("Default value", "Default value"), true), 0, 1);
                    grid->addWidget(makeCaption(DEMO_TEXT("Ticks / Compact", "Ticks / Compact"), true), 0, 2);
                    grid->addWidget(makeCaption(DEMO_TEXT("Disabled", "Disabled"), true), 0, 3);

                    grid->addWidget(makeCaption(QStringLiteral("FluentDial")), 1, 0);
                    grid->addWidget(prepareCell(makeDial(45, false, true, true), 128), 1, 1);
                    grid->addWidget(prepareCell(makeDial(135, true, true, true), 128), 1, 2);
                    grid->addWidget(prepareCell(makeDial(270, true, true, false), 128), 1, 3);

                    grid->addWidget(makeCaption(QStringLiteral("FluentDial no pointer")), 2, 0);
                    grid->addWidget(prepareCell(makeDial(90, false, false, true), 128), 2, 1);
                    grid->addWidget(prepareCell(makeDial(210, true, false, true), 128), 2, 2);
                    grid->addWidget(prepareCell(makeDial(315, true, false, false), 128), 2, 3);

                    grid->addWidget(makeCaption(QStringLiteral("FluentAngleSelector")), 3, 0);
                    grid->addWidget(prepareCell(makeSelector(128, true, true, true, true), 250), 3, 1);
                    grid->addWidget(prepareCell(makeSelector(210, false, true, true, true), 220), 3, 2);
                    grid->addWidget(prepareCell(makeSelector(300, true, true, true, false), 250), 3, 3);

                    grid->addWidget(makeCaption(QStringLiteral("Selector variants")), 4, 0);
                    grid->addWidget(prepareCell(makeSelector(30, true, false, true, true), 210), 4, 1);
                    grid->addWidget(prepareCell(makeSelector(180, false, true, false, true), 150), 4, 2);
                    grid->addWidget(prepareCell(makeSelector(330, true, false, true, false), 210), 4, 3);

                    grid->setColumnStretch(1, 1);
                    grid->setColumnStretch(2, 1);
                    grid->setColumnStretch(3, 1);
                    body->addLayout(grid);
                },
                false,
                330));
        }

        {
            QString code;
#define ANGLE_DIALS(X) \
    X(auto *row = new QHBoxLayout(); ) \
    X(row->setContentsMargins(0, 0, 0, 0); ) \
    X(row->setSpacing(14); ) \
    X(auto *dialA = new FluentDial(); ) \
    X(dialA->setValue(30); ) \
    X(auto *dialB = new FluentDial(); ) \
    X(dialB->setValue(135); ) \
    X(dialB->setTicksVisible(true); ) \
    X(dialB->setMajorTickStep(45); ) \
    X(auto *dialC = new FluentDial(); ) \
    X(dialC->setValue(270); ) \
    X(dialC->setTicksVisible(true); ) \
    X(dialC->setEnabled(false); ) \
    X(row->addWidget(dialA); ) \
    X(row->addWidget(dialB); ) \
    X(row->addWidget(dialC); ) \
    X(row->addStretch(1); ) \
    X(body->addLayout(row); )
#define X(line) code += QStringLiteral(#line "\n");
            ANGLE_DIALS(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("FluentDial"),
                DEMO_TEXT("基础角度旋钮", "Basic angle dial"),
                DEMO_TEXT("要点：\n"
                          "-角度指针\n"
                          "-刻度与禁用态\n"
                          "-hover/focus 高亮",
                          "Highlights:\n"
                          "-Angle indicator\n"
                          "-Ticks and disabled state\n"
                          "-Hover and focus highlight"),
                code,
                [=](QVBoxLayout *body) {
#define X(line) line
                    ANGLE_DIALS(X)
#undef X
                },
                false,
                170));

#undef ANGLE_DIALS
        }

        {
            QString code;
#define ANGLE_SELECTOR_BASIC(X) \
    X(auto *row = new QHBoxLayout(); ) \
    X(row->setContentsMargins(0, 0, 0, 0); ) \
    X(row->setSpacing(12); ) \
    X(auto *editor = new FluentAngleSelector(); ) \
    X(editor->setValue(128); ) \
    X(auto *valueLabel = new FluentLabel(QStringLiteral("128°")); ) \
    X(valueLabel->setStyleSheet("font-size: 12px; opacity: 0.85;"); ) \
    X(QObject::connect(editor, &FluentAngleSelector::valueChanged, valueLabel, [valueLabel](int v) { valueLabel->setText(QString::number(v) + QStringLiteral("°")); }); ) \
    X(row->addWidget(editor); ) \
    X(row->addWidget(valueLabel); ) \
    X(row->addStretch(1); ) \
    X(body->addLayout(row); )
#define X(line) code += QStringLiteral(#line "\n");
            ANGLE_SELECTOR_BASIC(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("FluentAngleSelector"),
                DEMO_TEXT("复合角度编辑器", "Composite angle editor"),
                DEMO_TEXT("要点：\n"
                          "-标签 + Dial + SpinBox 一体化\n"
                          "-只暴露一个 valueChanged(int)\n"
                          "-可直接复用到渐变、图形旋转等场景",
                          "Highlights:\n"
                          "-Combines label, dial, and spin box\n"
                          "-Exposes a single valueChanged(int) signal\n"
                          "-Ready for gradients, object rotation, and similar scenarios"),
                code,
                [=](QVBoxLayout *body) {
#define X(line) line
                    ANGLE_SELECTOR_BASIC(X)
#undef X
                },
                false,
                180));

#undef ANGLE_SELECTOR_BASIC
        }

        {
            QString code;
#define ANGLE_SELECTOR_VARIANTS(X) \
    X(auto *col = new QVBoxLayout(); ) \
    X(col->setContentsMargins(0, 0, 0, 0); ) \
    X(col->setSpacing(10); ) \
    X(auto *withLabel = new FluentAngleSelector(); ) \
    X(withLabel->setValue(45); ) \
    X(auto *dialOnly = new FluentAngleSelector(); ) \
    X(dialOnly->setValue(210); ) \
    X(dialOnly->setLabelVisible(false); ) \
    X(auto *spinOnly = new FluentAngleSelector(); ) \
    X(spinOnly->setValue(300); ) \
    X(spinOnly->setDialVisible(false); ) \
    X(col->addWidget(withLabel); ) \
    X(col->addWidget(dialOnly); ) \
    X(col->addWidget(spinOnly); ) \
    X(body->addLayout(col); )
#define X(line) code += QStringLiteral(#line "\n");
            ANGLE_SELECTOR_VARIANTS(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("FluentAngleSelector（可见性变体）"),
                DEMO_TEXT("支持单独隐藏标签或 Dial", "Supports hiding the label or the dial independently"),
                DEMO_TEXT("要点：\n"
                          "-setLabelVisible(bool)\n"
                          "-setDialVisible(bool)\n"
                          "-适应紧凑布局和无标题表单",
                          "Highlights:\n"
                          "-Use setLabelVisible(bool)\n"
                          "-Use setDialVisible(bool)\n"
                          "-Fits compact layouts and label-free forms"),
                code,
                [=](QVBoxLayout *body) {
#define X(line) line
                    ANGLE_SELECTOR_VARIANTS(X)
#undef X
                },
                true,
                190));

#undef ANGLE_SELECTOR_VARIANTS
        }
}

QWidget *createAngleControlsPage(FluentMainWindow *window)
{
    return Demo::makePage([&](QVBoxLayout *page) { fillAngleControls(page, window); });
}

} // namespace Demo::Pages


