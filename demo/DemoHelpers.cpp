#include "DemoHelpers.h"

#include "DemoCodeEditorSettings.h"

#include "Fluent/FluentAnimatedButton.h"
#include "Fluent/FluentCard.h"
#include "Fluent/FluentCodeEditor.h"
#include "Fluent/FluentLabel.h"
#include "Fluent/FluentLottieWidget.h"
#include "Fluent/FluentScrollArea.h"
#include "Fluent/FluentTheme.h"
#include "Fluent/FluentWidget.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QLocale>
#include <QSettings>
#include <QTranslator>
#include <QVBoxLayout>

namespace Demo {

using namespace Fluent;

namespace {

#ifndef QTFLUENT_RLOTTIE_RESOURCE_DIR
#define QTFLUENT_RLOTTIE_RESOURCE_DIR ""
#endif

QString utf8Text(const char *text)
{
    return QString::fromUtf8(text);
}

class DemoLibraryTranslator final : public QTranslator
{
public:
    QString translate(const char *context,
                      const char *sourceText,
                      const char *disambiguation,
                      int n) const override
    {
        Q_UNUSED(context)
        Q_UNUSED(disambiguation)
        Q_UNUSED(n)

        if (!sourceText) {
            return QString();
        }

        static const QHash<QString, QString> translations = {
            { utf8Text(u8"\u6708"), QStringLiteral("Month") },
            { utf8Text(u8"\u65e5"), QStringLiteral("Day") },
            { utf8Text(u8"\u5e74"), QStringLiteral("Year") },
            { utf8Text(u8"\u4eca\u5929"), QStringLiteral("Today") },
            { utf8Text(u8"\u65f6"), QStringLiteral("Hour") },
            { utf8Text(u8"\u5206"), QStringLiteral("Minute") },
            { utf8Text(u8"\u4e0a\u5348"), QStringLiteral("AM") },
            { utf8Text(u8"\u4e0b\u5348"), QStringLiteral("PM") },
            { utf8Text(u8"\u89d2\u5ea6\uff1a"), QStringLiteral("Angle:") },
            { utf8Text(u8"\u9009\u62e9\u989c\u8272"), QStringLiteral("Select color") },
            { utf8Text(u8"\u7eaf\u8272"), QStringLiteral("Solid") },
            { utf8Text(u8"\u7ebf\u6027\u6e10\u53d8"), QStringLiteral("Linear gradient") },
            { utf8Text(u8"\u5f84\u5411\u6e10\u53d8"), QStringLiteral("Radial gradient") },
            { utf8Text(u8"\u91cd\u7f6e"), QStringLiteral("Reset") },
            { utf8Text(u8"\u5355\u51fb\u6e10\u53d8\u6761\u6dfb\u52a0\u8272\u6807\uff0c\u53f3\u952e\u5220\u9664"), QStringLiteral("Click the gradient bar to add a stop, right-click to remove it") },
            { utf8Text(u8"\u9884\u7f6e\u989c\u8272"), QStringLiteral("Presets") },
            { utf8Text(u8"\u6700\u8fd1\u4f7f\u7528"), QStringLiteral("Recent") },
            { utf8Text(u8"\u53d6\u6d88"), QStringLiteral("Cancel") },
            { utf8Text(u8"\u786e\u5b9a"), QStringLiteral("OK") },
            { utf8Text(u8"\u4ece\u5c4f\u5e55\u53d6\u8272"), QStringLiteral("Pick from screen") },
            { utf8Text(u8"\u5220\u9664\u8272\u6807"), QStringLiteral("Delete stop") },
            { utf8Text(u8"\u62d6\u62fd\u65cb\u8f6c\u8c03\u6574\u89d2\u5ea6"), QStringLiteral("Drag to adjust angle") },
        };

        return translations.value(QString::fromUtf8(sourceText));
    }
};

DemoLibraryTranslator s_demoLibraryTranslator;
DemoLanguage s_currentLanguage = DemoLanguage::Chinese;
bool s_languageInitialized = false;

QLocale localeForLanguage(DemoLanguage language)
{
    if (language == DemoLanguage::English) {
        return QLocale(QLocale::English, QLocale::UnitedStates);
    }
    return QLocale(QLocale::Chinese, QLocale::China);
}

QString languageKey(DemoLanguage language)
{
    return language == DemoLanguage::English ? QStringLiteral("en") : QStringLiteral("zh");
}

DemoLanguage languageFromKey(const QString &key)
{
    return key.compare(QStringLiteral("en"), Qt::CaseInsensitive) == 0
               ? DemoLanguage::English
               : DemoLanguage::Chinese;
}

QSettings languageSettings()
{
    return QSettings();
}

DemoLanguage preferredLanguage()
{
    QSettings settings = languageSettings();
    const QString stored = settings.value(QStringLiteral("ui/language")).toString().trimmed();
    if (stored.isEmpty()) {
        return systemLanguage();
    }
    return languageFromKey(stored);
}

QString configuredRlottieResourceRoot()
{
    const QString configured = QString::fromUtf8(QTFLUENT_RLOTTIE_RESOURCE_DIR).trimmed();
    if (!configured.isEmpty() && QDir(configured).exists()) {
        return QDir(configured).absolutePath();
    }

    QDir probe(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 6; ++i) {
        const QString candidate = probe.filePath(QStringLiteral("third_party/rlottie/example/resource"));
        if (QDir(candidate).exists()) {
            return QDir(candidate).absolutePath();
        }
        if (!probe.cdUp()) {
            break;
        }
    }

    return configured;
}

void syncLanguage(DemoLanguage language)
{
    s_currentLanguage = language;
    QLocale::setDefault(localeForLanguage(language));

    auto *app = qobject_cast<QApplication *>(QCoreApplication::instance());
    if (!app) {
        return;
    }

    app->removeTranslator(&s_demoLibraryTranslator);
    if (language == DemoLanguage::English) {
        app->installTranslator(&s_demoLibraryTranslator);
    }
}

} // namespace

DemoLanguage systemLanguage()
{
    return QLocale::system().language() == QLocale::Chinese
               ? DemoLanguage::Chinese
               : DemoLanguage::English;
}

void initializeLanguage()
{
    if (s_languageInitialized) {
        return;
    }

    syncLanguage(preferredLanguage());
    s_languageInitialized = true;
}

DemoLanguage currentLanguage()
{
    return s_currentLanguage;
}

bool setLanguage(DemoLanguage language)
{
    const bool changed = !s_languageInitialized || s_currentLanguage != language;

    QSettings settings = languageSettings();
    settings.setValue(QStringLiteral("ui/language"), languageKey(language));

    syncLanguage(language);
    s_languageInitialized = true;
    return changed;
}

QString text(const QString &zh, const QString &en)
{
    return s_currentLanguage == DemoLanguage::English ? en : zh;
}

Section makeSection(const QString &title, const QString &subtitle)
{
    auto *card = new FluentCard();
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);

    auto *t = new FluentLabel(title);
    t->setStyleSheet("font-size: 16px; font-weight: 600;");
    layout->addWidget(t);

    if (!subtitle.isEmpty()) {
        auto *st = new FluentLabel(subtitle);
        st->setStyleSheet("font-size: 12px; opacity: 0.85;");
        layout->addWidget(st);
    }

    auto *body = new QVBoxLayout();
    body->setSpacing(10);
    layout->addLayout(body);

    return {card, body};
}

FluentCard *makeCollapsedCard(const QString &title,
                              const QString &subtitle,
                              const QString &features,
                              const QString &code,
                              bool collapsed)
{
    auto *card = new FluentCard();
    card->setCollapsible(true);
    card->setTitle(title);
    card->setCollapsed(collapsed);
    card->loadCollapseIndicatorAnimation(demoLottieResourcePath(QStringLiteral("dropdown.json")));

    auto *body = card->contentLayout();
    if (!body) {
        return card;
    }

    if (!subtitle.isEmpty()) {
        auto *st = new FluentLabel(subtitle);
        st->setWordWrap(true);
        st->setStyleSheet("font-size: 12px; opacity: 0.85;");
        body->addWidget(st);
    }

    if (!features.isEmpty()) {
        auto *ft = new FluentLabel(features);
        ft->setWordWrap(true);
        ft->setStyleSheet("font-size: 12px; opacity: 0.9;");
        body->addWidget(ft);
    }

    if (!code.isEmpty()) {
        auto *edit = new FluentCodeEditor();
        edit->setReadOnly(true);
        edit->setAutoFormatEnabled(false);
        edit->setAutoBraceNewlineEnabled(false);
        edit->setClangFormatMissingHintEnabled(false);
        QString pretty = code;
        pretty.replace("\\\"", "\"");
        edit->setPlainText(pretty);
        DemoCodeEditorSettings::instance().attach(edit, true);
        edit->setFixedHeight(170);
        body->addWidget(edit);
    }

    return card;
}

FluentCard *makeCollapsedExample(const QString &title,
                                 const QString &subtitle,
                                 const QString &features,
                                 const QString &code,
                                 const std::function<void(QVBoxLayout *body)> &buildDemo,
                                 bool collapsed,
                                 int codeBlockHeight)
{
    auto *card = new FluentCard();
    card->setCollapsible(true);
    card->setTitle(title);
    card->setCollapsed(collapsed);
    card->loadCollapseIndicatorAnimation(demoLottieResourcePath(QStringLiteral("dropdown.json")));

    auto *body = card->contentLayout();
    if (!body) {
        return card;
    }

    if (!subtitle.isEmpty()) {
        auto *st = new FluentLabel(subtitle);
        st->setWordWrap(true);
        st->setStyleSheet("font-size: 12px; opacity: 0.85;");
        body->addWidget(st);
    }

    if (!features.isEmpty()) {
        auto *ft = new FluentLabel(features);
        ft->setWordWrap(true);
        ft->setStyleSheet("font-size: 12px; opacity: 0.9;");
        body->addWidget(ft);
    }

    if (buildDemo) {
        buildDemo(body);
    }

    if (!code.isEmpty()) {
        auto *edit = new FluentCodeEditor();
        edit->setReadOnly(true);
        edit->setAutoFormatEnabled(false);
        edit->setAutoBraceNewlineEnabled(false);
        edit->setClangFormatMissingHintEnabled(false);
        QString pretty = code;
        pretty.replace("\\\"", "\"");
        edit->setPlainText(pretty);
        DemoCodeEditorSettings::instance().attach(edit, true);
        edit->setFixedHeight(codeBlockHeight);
        body->addWidget(edit);
    }

    return card;
}

void applyAccent(const QColor &accent)
{
    auto colors = ThemeManager::instance().colors();
    colors.accent = accent;
    colors.focus = accent.lighter(135);
    ThemeManager::instance().setColors(colors);
}

void applyBackground(const QColor &bg)
{
    auto colors = ThemeManager::instance().colors();
    colors.background = bg;
    ThemeManager::instance().setColors(colors);
}

void applySurface(const QColor &surface)
{
    auto colors = ThemeManager::instance().colors();
    colors.surface = surface;
    ThemeManager::instance().setColors(colors);
}

QByteArray animatedSearchIconJson()
{
    return QByteArrayLiteral(R"JSON({
  "v": "5.7.4",
  "fr": 60,
  "ip": 0,
  "op": 90,
  "w": 96,
  "h": 96,
  "nm": "QtFluent Search AnimatedIcon",
  "ddd": 0,
  "assets": [],
  "markers": [
    { "tm": 0, "cm": "Normal", "dr": 1 },
    { "tm": 0, "cm": "NormalToPointerOver_Start", "dr": 0 },
    { "tm": 24, "cm": "NormalToPointerOver_End", "dr": 0 },
    { "tm": 24, "cm": "PointerOver", "dr": 1 },
    { "tm": 24, "cm": "PointerOverToPressed_Start", "dr": 0 },
    { "tm": 46, "cm": "PointerOverToPressed_End", "dr": 0 },
    { "tm": 46, "cm": "Pressed", "dr": 1 },
    { "tm": 46, "cm": "PressedToPointerOver_Start", "dr": 0 },
    { "tm": 62, "cm": "PressedToPointerOver_End", "dr": 0 },
    { "tm": 62, "cm": "PointerOverToNormal_Start", "dr": 0 },
    { "tm": 89, "cm": "PointerOverToNormal_End", "dr": 0 }
  ],
  "layers": [
    {
      "ddd": 0,
      "ind": 1,
      "ty": 4,
      "nm": "Magnifier",
      "sr": 1,
      "ks": {
        "o": { "a": 0, "k": 100, "ix": 11 },
        "r": {
          "a": 1,
          "k": [
            { "t": 0, "s": [0], "e": [-8], "i": { "x": [0.4], "y": [1] }, "o": { "x": [0.6], "y": [0] } },
            { "t": 24, "s": [-8], "e": [-18], "i": { "x": [0.4], "y": [1] }, "o": { "x": [0.6], "y": [0] } },
            { "t": 46, "s": [-18], "e": [-8], "i": { "x": [0.4], "y": [1] }, "o": { "x": [0.6], "y": [0] } },
            { "t": 62, "s": [-8], "e": [0], "i": { "x": [0.4], "y": [1] }, "o": { "x": [0.6], "y": [0] } },
            { "t": 89, "s": [0] }
          ],
          "ix": 10
        },
        "p": { "a": 0, "k": [48, 48, 0], "ix": 2 },
        "a": { "a": 0, "k": [0, 0, 0], "ix": 1 },
        "s": {
          "a": 1,
          "k": [
            { "t": 0, "s": [100, 100, 100], "e": [112, 112, 100], "i": { "x": [0.4], "y": [1] }, "o": { "x": [0.6], "y": [0] } },
            { "t": 24, "s": [112, 112, 100], "e": [92, 92, 100], "i": { "x": [0.4], "y": [1] }, "o": { "x": [0.6], "y": [0] } },
            { "t": 46, "s": [92, 92, 100], "e": [112, 112, 100], "i": { "x": [0.4], "y": [1] }, "o": { "x": [0.6], "y": [0] } },
            { "t": 62, "s": [112, 112, 100], "e": [100, 100, 100], "i": { "x": [0.4], "y": [1] }, "o": { "x": [0.6], "y": [0] } },
            { "t": 89, "s": [100, 100, 100] }
          ],
          "ix": 6
        }
      },
      "ao": 0,
      "shapes": [
        {
          "ty": "gr",
          "it": [
            { "d": 1, "ty": "el", "s": { "a": 0, "k": [34, 34], "ix": 2 }, "p": { "a": 0, "k": [-5, -5], "ix": 3 }, "nm": "Lens" },
            { "ty": "st", "c": { "a": 0, "k": [0, 0.42, 0.75, 1], "ix": 3 }, "o": { "a": 0, "k": 100, "ix": 4 }, "w": { "a": 0, "k": 5, "ix": 5 }, "lc": 2, "lj": 2, "ml": 4, "bm": 0, "nm": "Stroke" },
            { "ty": "tr", "p": { "a": 0, "k": [0, 0], "ix": 2 }, "a": { "a": 0, "k": [0, 0], "ix": 1 }, "s": { "a": 0, "k": [100, 100], "ix": 3 }, "r": { "a": 0, "k": 0, "ix": 6 }, "o": { "a": 0, "k": 100, "ix": 7 }, "sk": { "a": 0, "k": 0, "ix": 4 }, "sa": { "a": 0, "k": 0, "ix": 5 }, "nm": "Transform" }
          ],
          "nm": "Lens Group",
          "bm": 0
        },
        {
          "ty": "gr",
          "it": [
            { "ty": "sh", "ks": { "a": 0, "k": { "i": [[0, 0], [0, 0]], "o": [[0, 0], [0, 0]], "v": [[9, 9], [25, 25]], "c": false }, "ix": 2 }, "nm": "Handle Path" },
            { "ty": "st", "c": { "a": 0, "k": [0, 0.42, 0.75, 1], "ix": 3 }, "o": { "a": 0, "k": 100, "ix": 4 }, "w": { "a": 0, "k": 5, "ix": 5 }, "lc": 2, "lj": 2, "ml": 4, "bm": 0, "nm": "Stroke" },
            { "ty": "tr", "p": { "a": 0, "k": [0, 0], "ix": 2 }, "a": { "a": 0, "k": [0, 0], "ix": 1 }, "s": { "a": 0, "k": [100, 100], "ix": 3 }, "r": { "a": 0, "k": 0, "ix": 6 }, "o": { "a": 0, "k": 100, "ix": 7 }, "sk": { "a": 0, "k": 0, "ix": 4 }, "sa": { "a": 0, "k": 0, "ix": 5 }, "nm": "Transform" }
          ],
          "nm": "Handle Group",
          "bm": 0
        }
      ],
      "ip": 0,
      "op": 90,
      "st": 0,
      "bm": 0
    }
  ]
})JSON");
}

QString rlottieResourcePath(const QString &fileName)
{
    if (QFileInfo(fileName).isAbsolute()) {
        return QDir::cleanPath(fileName);
    }

    const QString root = configuredRlottieResourceRoot();
    if (root.isEmpty()) {
        return QString();
    }

    return QDir(root).filePath(fileName);
}

bool loadRlottieResource(FluentLottieWidget *widget, const QString &fileName)
{
    if (!widget) {
        return false;
    }

    const QString path = rlottieResourcePath(fileName);
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        return false;
    }

    return widget->load(path);
}

QString demoLottieResourcePath(const QString &fileName)
{
    if (fileName.startsWith(QLatin1Char(':')) || QFileInfo(fileName).isAbsolute()) {
        return QDir::cleanPath(fileName);
    }

    return QStringLiteral(":/qtfluent/demo/lottie/%1").arg(fileName);
}

bool loadDemoLottieResource(FluentLottieWidget *widget, const QString &fileName)
{
    if (!widget) {
        return false;
    }

    return widget->load(demoLottieResourcePath(fileName));
}

FluentAnimatedButton *makeAnimatedSearchButton(const QString &text, QWidget *parent)
{
    auto *button = new FluentAnimatedButton(text, parent);
    button->setAnimatedIconSize(QSize(22, 22));
    button->loadAnimationData(animatedSearchIconJson(), QStringLiteral("demo-search-button-icon"));
    button->setAnimationState(QStringLiteral("Normal"), false);
    return button;
}

FluentScrollArea *makePage(const std::function<void(QVBoxLayout *)> &fill)
{
    auto *area = new FluentScrollArea();
    area->setOverlayScrollBarsEnabled(true);

    auto *content = new FluentWidget();
    content->setBackgroundRole(FluentWidget::BackgroundRole::WindowBackground);
    area->setWidget(content);

    auto *layout = new QVBoxLayout(content);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);
    fill(layout);
    layout->addStretch(1);

    return area;
}

QWidget *makeSidebarCard(QWidget *inner)
{
    auto *card = new FluentCard();
    auto *l = new QVBoxLayout(card);
    l->setContentsMargins(16, 16, 16, 16);
    l->setSpacing(10);
    l->addWidget(inner);
    return card;
}

} // namespace Demo
