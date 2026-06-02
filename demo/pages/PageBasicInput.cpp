#include "PageBasicInput.h"

#include "../DemoHelpers.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QVBoxLayout>

#include "Fluent/FluentAnimatedButton.h"
#include "Fluent/FluentButton.h"
#include "Fluent/FluentCard.h"
#include "Fluent/FluentCheckBox.h"
#include "Fluent/FluentComboBox.h"
#include "Fluent/FluentLabel.h"
#include "Fluent/FluentLineEdit.h"
#include "Fluent/FluentMainWindow.h"
#include "Fluent/FluentRadioButton.h"
#include "Fluent/FluentScrollArea.h"
#include "Fluent/FluentSlider.h"
#include "Fluent/FluentSpinBox.h"
#include "Fluent/FluentToggleSwitch.h"
#include "Fluent/FluentToolButton.h"

namespace Demo::Pages {

using namespace Fluent;

QWidget *createBasicInputPage(FluentMainWindow *window, const std::function<void(int)> &jumpTo)
{
    return Demo::makePage([&](QVBoxLayout *page) {
        auto hero = Demo::makeSection(
            DEMO_TEXT("基本输入", "Basic Input"),
            DEMO_TEXT("常用输入与命令控件的低密度总览。",
                      "Low-density overview of common input and command controls."));
        page->addWidget(hero.card);

        auto *jumpRow = new QHBoxLayout();
        jumpRow->setContentsMargins(0, 0, 0, 0);
        jumpRow->setSpacing(10);

        auto *inputsBtn = new FluentButton(DEMO_TEXT("查看输入详细页", "Open inputs page"));
        auto *buttonsBtn = new FluentButton(DEMO_TEXT("查看按钮详细页", "Open buttons page"));
        QObject::connect(inputsBtn, &QPushButton::clicked, window, [jumpTo]() { jumpTo(2); });
        QObject::connect(buttonsBtn, &QPushButton::clicked, window, [jumpTo]() { jumpTo(3); });

        jumpRow->addWidget(inputsBtn);
        jumpRow->addWidget(buttonsBtn);
        jumpRow->addStretch(1);
        hero.body->addLayout(jumpRow);

        auto addStateCard = [&](const QString &title,
                                const QString &code,
                                const std::function<void(QVBoxLayout *)> &buildDemo) {
            page->addWidget(Demo::makeCollapsedExample(title, QString(), QString(), code, buildDemo, false, 120));
        };

        addStateCard(DEMO_TEXT("文本输入状态", "Text input states"),
                     QStringLiteral("auto *edit = new FluentLineEdit();\nedit->setPlaceholderText(QStringLiteral(\"Search or type\"));\n"),
                     [](QVBoxLayout *body) {
                         auto *row = new QHBoxLayout();
                         row->setContentsMargins(0, 0, 0, 0);
                         row->setSpacing(12);

                         auto *ready = new FluentLineEdit();
                         ready->setPlaceholderText(DEMO_TEXT("搜索或输入", "Search or type"));
                         auto *filled = new FluentLineEdit();
                         filled->setText(QStringLiteral("QtFluent"));
                         auto *disabled = new FluentLineEdit();
                         disabled->setPlaceholderText(DEMO_TEXT("禁用态", "Disabled"));
                         disabled->setDisabled(true);

                         row->addWidget(ready, 1);
                         row->addWidget(filled, 1);
                         row->addWidget(disabled, 1);
                         body->addLayout(row);
                     });

        addStateCard(DEMO_TEXT("数值与组合", "Numeric and combo"),
                     QStringLiteral("auto *combo = new FluentComboBox();\nauto *spin = new FluentSpinBox();\n"),
                     [](QVBoxLayout *body) {
                         auto *row = new QHBoxLayout();
                         row->setContentsMargins(0, 0, 0, 0);
                         row->setSpacing(12);

                         auto makeCombo = [](int index, bool enabled) {
                             auto *combo = new FluentComboBox();
                             combo->addItems({DEMO_TEXT("选项 A", "Option A"),
                                              DEMO_TEXT("选项 B", "Option B"),
                                              DEMO_TEXT("选项 C", "Option C")});
                             combo->setCurrentIndex(index);
                             combo->setMinimumWidth(150);
                             combo->setEnabled(enabled);
                             return combo;
                         };

                         auto *spin = new FluentSpinBox();
                         spin->setRange(0, 100);
                         spin->setValue(24);
                         auto *slider = new FluentSlider(Qt::Horizontal);
                         slider->setRange(0, 100);
                         slider->setValue(62);
                         slider->setMinimumWidth(180);

                         row->addWidget(makeCombo(0, true));
                         row->addWidget(spin);
                         row->addWidget(makeCombo(1, true));
                         row->addWidget(slider, 1);
                         row->addWidget(makeCombo(2, false));
                         body->addLayout(row);
                     });

        addStateCard(DEMO_TEXT("命令按钮", "Command buttons"),
                     QStringLiteral("auto *primary = new FluentButton(QStringLiteral(\"Primary\"));\nprimary->setPrimary(true);\n"),
                     [](QVBoxLayout *body) {
                         auto *row = new QHBoxLayout();
                         row->setContentsMargins(0, 0, 0, 0);
                         row->setSpacing(12);

                         auto *secondary = new FluentButton(QStringLiteral("Secondary"));
                         auto *primary = new FluentButton(QStringLiteral("Primary"));
                         primary->setPrimary(true);
                         auto *tool = new FluentToolButton(QStringLiteral("Tool"));
                         auto *disabled = new FluentButton(QStringLiteral("Disabled"));
                         disabled->setDisabled(true);

                         row->addWidget(secondary);
                         row->addWidget(primary);
                         row->addWidget(tool);
                         row->addWidget(disabled);
                         row->addStretch(1);
                         body->addLayout(row);
                     });

        addStateCard(DEMO_TEXT("选择控件", "Selection controls"),
                     QStringLiteral("auto *toggle = new FluentToggleSwitch(QStringLiteral(\"Toggle\"));\ntoggle->setChecked(true);\n"),
                     [](QVBoxLayout *body) {
                         auto *row = new QHBoxLayout();
                         row->setContentsMargins(0, 0, 0, 0);
                         row->setSpacing(16);

                         auto *toggle = new FluentToggleSwitch(DEMO_TEXT("开关", "Toggle"));
                         toggle->setChecked(true);
                         auto *check = new FluentCheckBox(DEMO_TEXT("复选", "Check"));
                         check->setChecked(true);
                         auto *radio = new FluentRadioButton(DEMO_TEXT("单选", "Radio"));
                         radio->setChecked(true);
                         auto *disabled = new FluentToggleSwitch(DEMO_TEXT("禁用", "Disabled"));
                         disabled->setChecked(true);
                         disabled->setDisabled(true);

                         row->addWidget(toggle);
                         row->addWidget(check);
                         row->addWidget(radio);
                         row->addWidget(disabled);
                         row->addStretch(1);
                         body->addLayout(row);
                     });

        page->addWidget(Demo::makeCollapsedExample(
            DEMO_TEXT("输入控件概览", "Input overview"),
            DEMO_TEXT("LineEdit / ComboBox / Slider / SpinBox 的组合预览", "Combined preview of LineEdit / ComboBox / Slider / SpinBox"),
            DEMO_TEXT("要点：\n-这个整合页负责总览和分组入口\n-详细行为仍在子页中完整展示\n-父节点和子节点现在都可以独立导航",
                      "Highlights:\n-This hub page provides the overview and group entry point\n-Detailed behavior is still fully demonstrated in the child pages\n-Parent and child navigation items can now be used independently"),
            QString(),
            [window, jumpTo](QVBoxLayout *body) {
                auto *lineRow = new QHBoxLayout();
                lineRow->setContentsMargins(0, 0, 0, 0);
                lineRow->setSpacing(10);

                auto *nameEdit = new FluentLineEdit();
                nameEdit->setPlaceholderText(DEMO_TEXT("搜索或输入内容", "Search or type"));
                auto *searchButton = Demo::makeAnimatedSearchButton(DEMO_TEXT("搜索", "Search"));
                auto *disabledEdit = new FluentLineEdit();
                disabledEdit->setPlaceholderText(DEMO_TEXT("禁用态示例", "Disabled example"));
                disabledEdit->setDisabled(true);
                lineRow->addWidget(nameEdit, 1);
                lineRow->addWidget(searchButton);
                lineRow->addWidget(disabledEdit, 1);
                body->addLayout(lineRow);
                QObject::connect(searchButton, &QPushButton::clicked, nameEdit, [nameEdit]() {
                    nameEdit->setText(QStringLiteral("QtFluent"));
                    nameEdit->setFocus();
                    nameEdit->selectAll();
                });

                auto *valueRow = new QHBoxLayout();
                valueRow->setContentsMargins(0, 0, 0, 0);
                valueRow->setSpacing(10);

                auto *combo = new FluentComboBox();
                combo->addItems({DEMO_TEXT("选项 A", "Option A"), DEMO_TEXT("选项 B", "Option B"), DEMO_TEXT("选项 C", "Option C")});
                combo->setCurrentIndex(1);

                auto *spin = new FluentSpinBox();
                spin->setRange(0, 100);
                spin->setValue(24);

                valueRow->addWidget(combo);
                valueRow->addWidget(spin);
                valueRow->addStretch(1);
                body->addLayout(valueRow);

                auto *sliderRow = new QHBoxLayout();
                sliderRow->setContentsMargins(0, 0, 0, 0);
                sliderRow->setSpacing(10);

                auto *slider = new FluentSlider(Qt::Horizontal);
                slider->setRange(0, 100);
                slider->setValue(35);
                auto *valueLabel = new FluentLabel(QStringLiteral("35"));
                valueLabel->setMinimumWidth(28);
                QObject::connect(slider, &QSlider::valueChanged, valueLabel, [valueLabel](int value) {
                    valueLabel->setText(QString::number(value));
                });

                sliderRow->addWidget(slider, 1);
                sliderRow->addWidget(valueLabel);
                body->addLayout(sliderRow);

                auto *jumpBtn = new FluentButton(DEMO_TEXT("进入输入详细页 →", "Open inputs page ->"));
                QObject::connect(jumpBtn, &QPushButton::clicked, window, [jumpTo]() { jumpTo(2); });
                body->addWidget(jumpBtn);
            },
            false));

        page->addWidget(Demo::makeCollapsedExample(
            DEMO_TEXT("按钮 / 开关概览", "Buttons / toggles overview"),
            DEMO_TEXT("Primary Button / ToolButton / Toggle / Check / Radio 的组合预览", "Combined preview of Primary Button / ToolButton / Toggle / Check / Radio"),
            DEMO_TEXT("要点：\n-父节点现在可直接选中跳到整合页\n-展开箭头只负责子项显隐\n-详细状态展示依然保留在子页",
                      "Highlights:\n-The parent item can now be selected directly to open this hub page\n-The chevron is only responsible for expanding or collapsing child items\n-Detailed state demonstrations remain in the child pages"),
            QString(),
            [window, jumpTo](QVBoxLayout *body) {
                auto *buttonRow = new QHBoxLayout();
                buttonRow->setContentsMargins(0, 0, 0, 0);
                buttonRow->setSpacing(10);

                auto *primary = new FluentButton(QStringLiteral("Primary"));
                primary->setPrimary(true);
                auto *secondary = new FluentButton(QStringLiteral("Secondary"));
                auto *tool = new FluentToolButton(QStringLiteral("Tool"));
                buttonRow->addWidget(primary);
                buttonRow->addWidget(secondary);
                buttonRow->addWidget(tool);
                buttonRow->addStretch(1);
                body->addLayout(buttonRow);

                auto *optionRow = new QHBoxLayout();
                optionRow->setContentsMargins(0, 0, 0, 0);
                optionRow->setSpacing(14);

                auto *toggle = new FluentToggleSwitch(DEMO_TEXT("开关", "Toggle"));
                toggle->setChecked(true);
                auto *check = new FluentCheckBox(DEMO_TEXT("复选", "Check"));
                check->setChecked(true);
                auto *radio = new FluentRadioButton(DEMO_TEXT("单选", "Radio"));
                radio->setChecked(true);

                optionRow->addWidget(toggle);
                optionRow->addWidget(check);
                optionRow->addWidget(radio);
                optionRow->addStretch(1);
                body->addLayout(optionRow);

                auto *jumpBtn = new FluentButton(DEMO_TEXT("进入按钮详细页 →", "Open buttons page ->"));
                QObject::connect(jumpBtn, &QPushButton::clicked, window, [jumpTo]() { jumpTo(3); });
                body->addWidget(jumpBtn);
            },
            false));
    });
}

} // namespace Demo::Pages
