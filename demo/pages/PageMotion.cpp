#include "PageMotion.h"

#include "../DemoHelpers.h"

#include <QColor>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QSlider>
#include <QtGlobal>
#include <QVector>
#include <QVBoxLayout>

#include <utility>

#include "Fluent/FluentButton.h"
#include "Fluent/FluentCard.h"
#include "Fluent/FluentLabel.h"
#include "Fluent/FluentLottieWidget.h"
#include "Fluent/FluentScrollArea.h"
#include "Fluent/FluentTheme.h"
#include "Fluent/FluentToggleSwitch.h"
#include "Fluent/FluentWidget.h"

namespace Demo::Pages {

using namespace Fluent;

namespace {

struct MotionSample
{
    QString title;
    QString fileName;
    QString description;
};

QWidget *makeSampleTile(const MotionSample &sample, const QSize &animationSize, QWidget *parent = nullptr)
{
    auto *tile = new QWidget(parent);
    tile->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    tile->setMinimumWidth(132);

    auto *layout = new QVBoxLayout(tile);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(6);

    auto *animation = new FluentLottieWidget(tile);
    animation->setFixedSize(animationSize);
    animation->setToolTip(Demo::demoLottieResourcePath(sample.fileName));
    if (Demo::loadDemoLottieResource(animation, sample.fileName)) {
        animation->play();
    }

    auto *title = new FluentLabel(sample.title, tile);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 650;"));

    auto *description = new FluentLabel(sample.description, tile);
    description->setAlignment(Qt::AlignCenter);
    description->setWordWrap(true);
    description->setStyleSheet(QStringLiteral("font-size: 11px; opacity: 0.82;"));

    layout->addWidget(animation, 0, Qt::AlignCenter);
    layout->addWidget(title);
    layout->addWidget(description);
    return tile;
}

void applyTint(FluentLottieWidget *animation, const QColor &color)
{
    if (animation) {
        animation->setTintColor(color);
    }
}

} // namespace

QWidget *createMotionPage()
{
    return Demo::makePage([&](QVBoxLayout *page) {
        auto s = Demo::makeSection(
            DEMO_TEXT("动态", "Motion"),
            DEMO_TEXT("展示 FluentLottieWidget 加载内嵌 JSON 资源、播放控制、主题联动以及图标型 Lottie 的 tint 效果。",
                      "Show FluentLottieWidget loading embedded JSON assets, playback controls, theme linkage, and tinting for icon-like Lottie files."));
        page->addWidget(s.card);

        auto *summary = new FluentLabel(DEMO_TEXT(
            "这些示例使用 demo/lottie_assets.qrc 内嵌的图标型 JSON。它们适合导航、展开按钮、设置入口和滚动提示等微交互动效；按钮状态动画仍建议使用专门制作的 marker 资源。",
            "These examples use icon-like JSON files embedded through demo/lottie_assets.qrc. They fit navigation, expander buttons, settings entry points, and scroll hints; button state animations should still use purpose-built marker assets."));
        summary->setWordWrap(true);
        summary->setStyleSheet(QStringLiteral("font-size: 12px; opacity: 0.88;"));
        s.body->addWidget(summary);

        // Resource gallery
        {
            QString code;
#define MOTION_GALLERY(X) \
    X(auto *animation = new FluentLottieWidget();) \
    X(animation->setFixedSize(72, 72);) \
    X(animation->load(Demo::demoLottieResourcePath(QStringLiteral("dropdown.json")));) \
    X(animation->play();)

#define X(line) code += QStringLiteral(#line "\n");
            MOTION_GALLERY(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                QStringLiteral("FluentLottieWidget"),
                DEMO_TEXT("内嵌 Lottie 图标矩阵", "Embedded Lottie icon matrix"),
                DEMO_TEXT("要点：\n"
                          "-load() 可直接加载 Lottie JSON 文件\n"
                          "-play()/pause()/stop() 控制播放\n"
                          "-适合导航、展开、设置、滚动提示等轻量动效",
                          "Highlights:\n"
                          "-load() can load a Lottie JSON file directly\n"
                          "-play()/pause()/stop() control playback\n"
                          "-Works well for navigation, expanders, settings, and scroll hints"),
                code,
                [](QVBoxLayout *body) {
                    const QVector<MotionSample> samples = {
                        {DEMO_TEXT("菜单", "Menu"), QStringLiteral("menu.json"), DEMO_TEXT("导航入口", "Navigation")},
                        {DEMO_TEXT("展开", "Dropdown"), QStringLiteral("dropdown.json"), DEMO_TEXT("折叠状态", "Collapse state")},
                        {DEMO_TEXT("设置", "Settings"), QStringLiteral("setting.json"), DEMO_TEXT("系统入口", "System entry")},
                        {DEMO_TEXT("左箭头", "Left arrow"), QStringLiteral("left-arrow.json"), DEMO_TEXT("返回动作", "Back action")},
                        {DEMO_TEXT("右箭头", "Right arrow"), QStringLiteral("right-arrow.json"), DEMO_TEXT("前进动作", "Forward action")},
                        {DEMO_TEXT("向下滚动", "Scroll down"), QStringLiteral("scroll-down.json"), DEMO_TEXT("滚动提示", "Scroll hint")},
                    };

                    auto *gridHost = new QWidget();
                    auto *grid = new QGridLayout(gridHost);
                    grid->setContentsMargins(0, 0, 0, 0);
                    grid->setHorizontalSpacing(12);
                    grid->setVerticalSpacing(10);

                    for (int i = 0; i < samples.size(); ++i) {
                        grid->addWidget(makeSampleTile(samples[i], QSize(82, 82), gridHost), i / 3, i % 3);
                    }

                    body->addWidget(gridHost);
                },
                false,
                120));

#undef MOTION_GALLERY
        }

        // Playback controls
        {
            QString code;
#define MOTION_PLAYBACK(X) \
    X(auto *animation = new FluentLottieWidget();) \
    X(animation->setFixedSize(120, 92);) \
    X(animation->load(Demo::demoLottieResourcePath(QStringLiteral("menu.json")));) \
    X(animation->setLooping(true);) \
    X(animation->setSpeed(1.0);) \
    X(animation->play();) \
    X(auto *play = new FluentButton(QStringLiteral("Play"));) \
    X(auto *pause = new FluentButton(QStringLiteral("Pause"));) \
    X(auto *stop = new FluentButton(QStringLiteral("Stop"));) \
    X(auto *speed = new QSlider(Qt::Horizontal);) \
    X(speed->setRange(25, 400);) \
    X(speed->setValue(100);) \
    X(QObject::connect(play, &QPushButton::clicked, animation, &FluentLottieWidget::play);) \
    X(QObject::connect(pause, &QPushButton::clicked, animation, &FluentLottieWidget::pause);) \
    X(QObject::connect(stop, &QPushButton::clicked, animation, &FluentLottieWidget::stop);) \
    X(QObject::connect(speed, &QSlider::valueChanged, animation, [animation](int value) {) \
    X(    animation->setSpeed(value / 100.0);) \
    X(});)

#define X(line) code += QStringLiteral(#line "\n");
            MOTION_PLAYBACK(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                DEMO_TEXT("播放控制", "Playback"),
                DEMO_TEXT("播放、暂停、停止与进度定位", "Play, pause, stop, and seek progress"),
                DEMO_TEXT("要点：\n"
                          "-progressChanged(qreal) 可绑定滑块或状态文本\n"
                          "-setProgress(qreal) 可定位到任意帧\n"
                          "-setSpeed(qreal) 可调整播放速度，Card 折叠指示器会自动同步到自身展开/收起时长\n"
                          "-setLooping(false) 可用于一次性完成反馈",
                          "Highlights:\n"
                          "-progressChanged(qreal) can drive a slider or status text\n"
                          "-setProgress(qreal) seeks to an arbitrary frame\n"
                          "-setSpeed(qreal) adjusts playback speed; Card indicators auto-sync to their expand/collapse duration\n"
                          "-setLooping(false) is useful for one-shot completion feedback"),
                code,
                [](QVBoxLayout *body) {
                    auto *row = new QHBoxLayout();
                    row->setContentsMargins(0, 0, 0, 0);
                    row->setSpacing(14);

                    auto *animation = new FluentLottieWidget();
                    animation->setFixedSize(128, 96);
                    animation->setLooping(true);
                    if (Demo::loadDemoLottieResource(animation, QStringLiteral("menu.json"))) {
                        animation->play();
                    }

                    auto *controls = new QWidget();
                    auto *controlsLayout = new QVBoxLayout(controls);
                    controlsLayout->setContentsMargins(0, 0, 0, 0);
                    controlsLayout->setSpacing(8);

                    auto *buttons = new QHBoxLayout();
                    buttons->setContentsMargins(0, 0, 0, 0);
                    buttons->setSpacing(8);

                    auto *play = new FluentButton(QStringLiteral("Play"));
                    auto *pause = new FluentButton(QStringLiteral("Pause"));
                    auto *stop = new FluentButton(QStringLiteral("Stop"));

                    buttons->addWidget(play);
                    buttons->addWidget(pause);
                    buttons->addWidget(stop);
                    buttons->addStretch(1);

                    auto *progress = new QSlider(Qt::Horizontal);
                    progress->setRange(0, 1000);
                    auto *status = new FluentLabel(QStringLiteral("0%"));
                    status->setMinimumWidth(42);

                    auto *progressRow = new QHBoxLayout();
                    progressRow->setContentsMargins(0, 0, 0, 0);
                    progressRow->setSpacing(8);
                    progressRow->addWidget(progress, 1);
                    progressRow->addWidget(status);

                    auto *speed = new QSlider(Qt::Horizontal);
                    speed->setRange(25, 400);
                    speed->setValue(100);
                    speed->setSingleStep(25);
                    speed->setPageStep(50);
                    auto *speedLabel = new FluentLabel(QStringLiteral("Speed 1.00x"));
                    speedLabel->setMinimumWidth(92);

                    auto *speedRow = new QHBoxLayout();
                    speedRow->setContentsMargins(0, 0, 0, 0);
                    speedRow->setSpacing(8);
                    speedRow->addWidget(speed, 1);
                    speedRow->addWidget(speedLabel);

                    controlsLayout->addLayout(buttons);
                    controlsLayout->addLayout(progressRow);
                    controlsLayout->addLayout(speedRow);
                    controlsLayout->addStretch(1);

                    QObject::connect(play, &QPushButton::clicked, animation, &FluentLottieWidget::play);
                    QObject::connect(pause, &QPushButton::clicked, animation, &FluentLottieWidget::pause);
                    QObject::connect(stop, &QPushButton::clicked, animation, &FluentLottieWidget::stop);

                    QObject::connect(animation, &FluentLottieWidget::progressChanged, progress, [progress, status](qreal value) {
                        const int next = qRound(value * 1000.0);
                        const QSignalBlocker blocker(progress);
                        progress->setValue(next);
                        status->setText(QStringLiteral("%1%").arg(qRound(value * 100.0)));
                    });
                    QObject::connect(progress, &QSlider::sliderPressed, animation, &FluentLottieWidget::pause);
                    QObject::connect(progress, &QSlider::valueChanged, animation, [animation, progress](int value) {
                        if (progress->isSliderDown()) {
                            animation->setProgress(static_cast<qreal>(value) / 1000.0);
                        }
                    });
                    QObject::connect(speed, &QSlider::valueChanged, animation, [animation, speedLabel](int value) {
                        const qreal speedValue = static_cast<qreal>(value) / 100.0;
                        animation->setSpeed(speedValue);
                        speedLabel->setText(QStringLiteral("Speed %1x").arg(speedValue, 0, 'f', 2));
                    });

                    row->addWidget(animation);
                    row->addWidget(controls, 1);
                    body->addLayout(row);
                },
                true,
                170));

#undef MOTION_PLAYBACK
        }

        // Tinting
        {
            QString code;
#define MOTION_TINT(X) \
    X(auto *animation = new FluentLottieWidget();) \
    X(animation->setFixedSize(72, 72);) \
    X(animation->load(Demo::demoLottieResourcePath(QStringLiteral("setting.json")));) \
    X(animation->setTintColor(ThemeManager::instance().colors().accent);) \
    X(animation->play();)

#define X(line) code += QStringLiteral(#line "\n");
            MOTION_TINT(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                DEMO_TEXT("单色 Tint", "Single-color Tint"),
                DEMO_TEXT("把图标型 Lottie 重染为当前主题色", "Recolor icon-like Lottie animations to theme colors"),
                DEMO_TEXT("要点：\n"
                          "-setTintColor() 按 alpha 把整帧重染为单色\n"
                          "-resetTintColor() 恢复 Lottie 原始颜色\n"
                          "-适合图标型动画，不适合复杂插画",
                          "Highlights:\n"
                          "-setTintColor() recolors the rendered frame by alpha\n"
                          "-resetTintColor() restores the original Lottie colors\n"
                          "-Best for icon-like motion, not full illustrations"),
                code,
                [](QVBoxLayout *body) {
                    auto *row = new QHBoxLayout();
                    row->setContentsMargins(0, 0, 0, 0);
                    row->setSpacing(18);

                    auto makeTinted = [](const QString &title, const QString &fileName, QWidget *parent = nullptr) {
                        auto *host = new QWidget(parent);
                        auto *layout = new QVBoxLayout(host);
                        layout->setContentsMargins(0, 0, 0, 0);
                        layout->setSpacing(6);

                        auto *animation = new FluentLottieWidget(host);
                        animation->setFixedSize(76, 76);
                        if (Demo::loadDemoLottieResource(animation, fileName)) {
                            animation->play();
                        }

                        auto *label = new FluentLabel(title, host);
                        label->setAlignment(Qt::AlignCenter);
                        label->setStyleSheet(QStringLiteral("font-size: 12px; opacity: 0.86;"));
                        layout->addWidget(animation, 0, Qt::AlignCenter);
                        layout->addWidget(label);
                        return std::make_pair(host, animation);
                    };

                    auto original = makeTinted(DEMO_TEXT("原色", "Original"), QStringLiteral("setting.json"));
                    auto accent = makeTinted(DEMO_TEXT("Accent", "Accent"), QStringLiteral("setting.json"));
                    auto text = makeTinted(DEMO_TEXT("Text", "Text"), QStringLiteral("dropdown.json"));
                    auto disabled = makeTinted(DEMO_TEXT("Disabled", "Disabled"), QStringLiteral("scroll-down.json"));

                    auto applyThemeTint = [accentAnim = accent.second, textAnim = text.second, disabledAnim = disabled.second]() {
                        const auto &colors = ThemeManager::instance().colors();
                        applyTint(accentAnim, colors.accent);
                        applyTint(textAnim, colors.text);
                        applyTint(disabledAnim, colors.disabledText);
                    };
                    applyThemeTint();

                    QObject::connect(&ThemeManager::instance(),
                                     &ThemeManager::themeChanged,
                                     accent.first,
                                     applyThemeTint);

                    row->addWidget(original.first);
                    row->addWidget(accent.first);
                    row->addWidget(text.first);
                    row->addWidget(disabled.first);
                    row->addStretch(1);
                    body->addLayout(row);
                },
                true,
                130));

#undef MOTION_TINT
        }

        // Theme linkage
        {
            QString code;
#define MOTION_THEME_LINK(X) \
    X(auto *animation = new FluentLottieWidget();) \
    X(animation->load(Demo::demoLottieResourcePath(QStringLiteral("setting.json")));) \
    X(animation->play();) \
    X(auto applyTheme = [animation]() {) \
    X(    animation->setTintColor(ThemeManager::instance().colors().accent);) \
    X(};) \
    X(applyTheme();) \
    X(QObject::connect(&ThemeManager::instance(), &ThemeManager::themeChanged, animation, applyTheme);)

#define X(line) code += QStringLiteral(#line "\n");
            MOTION_THEME_LINK(X)
#undef X

            page->addWidget(Demo::makeCollapsedExample(
                DEMO_TEXT("主题联动", "Theme Linkage"),
                DEMO_TEXT("FluentLottieWidget 随 ThemeManager 实时更新颜色", "FluentLottieWidget updates color with ThemeManager"),
                DEMO_TEXT("要点：\n"
                          "-监听 ThemeManager::themeChanged 后重新设置 tintColor\n"
                          "-同一份 Lottie 可分别映射为 accent/text/error 等语义色\n"
                          "-切换浅色/深色主题时不需要重新加载 JSON",
                          "Highlights:\n"
                          "-Listen to ThemeManager::themeChanged and refresh tintColor\n"
                          "-The same Lottie can map to semantic colors such as accent/text/error\n"
                          "-Light/dark theme changes do not require reloading JSON"),
                code,
                [](QVBoxLayout *body) {
                    auto *host = new QWidget();
                    auto *layout = new QVBoxLayout(host);
                    layout->setContentsMargins(0, 0, 0, 0);
                    layout->setSpacing(12);

                    auto *modeRow = new QHBoxLayout();
                    modeRow->setContentsMargins(0, 0, 0, 0);
                    modeRow->setSpacing(10);

                    auto *darkMode = new FluentToggleSwitch(DEMO_TEXT("深色模式", "Dark mode"), host);
                    auto *modeLabel = new FluentLabel(host);
                    modeLabel->setStyleSheet(QStringLiteral("font-size: 12px; opacity: 0.86;"));
                    modeRow->addWidget(darkMode);
                    modeRow->addWidget(modeLabel, 1);

                    auto *sampleRow = new QHBoxLayout();
                    sampleRow->setContentsMargins(0, 0, 0, 0);
                    sampleRow->setSpacing(18);

                    struct ThemeLinkedTile
                    {
                        QWidget *host = nullptr;
                        FluentLottieWidget *animation = nullptr;
                        FluentLabel *value = nullptr;
                    };

                    auto makeThemeTile = [](const QString &title, const QString &fileName, QWidget *parent) {
                        ThemeLinkedTile tile;
                        tile.host = new QWidget(parent);
                        tile.host->setObjectName(QStringLiteral("MotionThemeTile"));

                        auto *tileLayout = new QVBoxLayout(tile.host);
                        tileLayout->setContentsMargins(10, 8, 10, 8);
                        tileLayout->setSpacing(6);

                        tile.animation = new FluentLottieWidget(tile.host);
                        tile.animation->setFixedSize(76, 76);
                        if (Demo::loadDemoLottieResource(tile.animation, fileName)) {
                            tile.animation->play();
                        }

                        auto *titleLabel = new FluentLabel(title, tile.host);
                        titleLabel->setAlignment(Qt::AlignCenter);
                        titleLabel->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 650;"));

                        tile.value = new FluentLabel(tile.host);
                        tile.value->setAlignment(Qt::AlignCenter);
                        tile.value->setStyleSheet(QStringLiteral("font-size: 11px; opacity: 0.78;"));

                        tileLayout->addWidget(tile.animation, 0, Qt::AlignCenter);
                        tileLayout->addWidget(titleLabel);
                        tileLayout->addWidget(tile.value);
                        return tile;
                    };

                    ThemeLinkedTile accent = makeThemeTile(QStringLiteral("Accent"), QStringLiteral("setting.json"), host);
                    ThemeLinkedTile text = makeThemeTile(QStringLiteral("Text"), QStringLiteral("dropdown.json"), host);
                    ThemeLinkedTile error = makeThemeTile(QStringLiteral("Error"), QStringLiteral("delete.json"), host);

                    auto applyTheme = [host, darkMode, modeLabel, accent, text, error]() {
                        const auto &colors = ThemeManager::instance().colors();
                        const bool isDark = ThemeManager::instance().themeMode() == ThemeManager::ThemeMode::Dark;

                        darkMode->setChecked(isDark);
                        modeLabel->setText(DEMO_TEXT("当前主题：%1，切换后 Lottie tint 会跟随语义色刷新",
                                                     "Current theme: %1. Lottie tint follows semantic colors after switching")
                                               .arg(isDark ? DEMO_TEXT("深色", "Dark") : DEMO_TEXT("浅色", "Light")));

                        accent.animation->setTintColor(colors.accent);
                        text.animation->setTintColor(colors.text);
                        error.animation->setTintColor(colors.error);

                        accent.value->setText(colors.accent.name(QColor::HexRgb).toUpper());
                        text.value->setText(colors.text.name(QColor::HexRgb).toUpper());
                        error.value->setText(colors.error.name(QColor::HexRgb).toUpper());

                        const QString tileStyle = QStringLiteral(
                            "#MotionThemeTile { background: %1; border: 1px solid %2; border-radius: 8px; }")
                                                        .arg(colors.surface.name(QColor::HexRgb),
                                                             colors.border.name(QColor::HexRgb));
                        accent.host->setStyleSheet(tileStyle);
                        text.host->setStyleSheet(tileStyle);
                        error.host->setStyleSheet(tileStyle);
                    };

                    QObject::connect(darkMode, &FluentToggleSwitch::toggled, host, [](bool checked) {
                        ThemeManager::instance().setThemeMode(checked ? ThemeManager::ThemeMode::Dark
                                                                      : ThemeManager::ThemeMode::Light);
                    });
                    QObject::connect(&ThemeManager::instance(), &ThemeManager::themeChanged, host, applyTheme);
                    applyTheme();

                    sampleRow->addWidget(accent.host);
                    sampleRow->addWidget(text.host);
                    sampleRow->addWidget(error.host);
                    sampleRow->addStretch(1);

                    layout->addLayout(modeRow);
                    layout->addLayout(sampleRow);
                    body->addWidget(host);
                },
                true,
                150));

#undef MOTION_THEME_LINK
        }
    });
}

} // namespace Demo::Pages
