#include "Fluent/FluentLabel.h"
#include "Fluent/FluentTheme.h"

#include <QEvent>

namespace Fluent {

namespace {

QString fluentLabelThemeMarker()
{
    return QStringLiteral("/*FluentLabelTheme*/");
}

QString withoutFluentLabelThemeRule(const QString &styleSheet)
{
    const int idx = styleSheet.indexOf(fluentLabelThemeMarker());
    return idx >= 0 ? styleSheet.left(idx) : styleSheet;
}

} // namespace

FluentLabel::FluentLabel(QWidget *parent)
    : QLabel(parent)
{
    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentLabel::applyTheme);
}

FluentLabel::FluentLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentLabel::applyTheme);
}

void FluentLabel::setStyleSheet(const QString &styleSheet)
{
    const QString userStyle = withoutFluentLabelThemeRule(styleSheet);
    if (QLabel::styleSheet() != userStyle) {
        QLabel::setStyleSheet(userStyle);
    }
    applyTheme();
}

void FluentLabel::changeEvent(QEvent *event)
{
    QLabel::changeEvent(event);

    if (event->type() == QEvent::EnabledChange || event->type() == QEvent::StyleChange) {
        applyTheme();
    }
}

void FluentLabel::applyTheme()
{
    const auto &colors = ThemeManager::instance().colors();

    // Don't clobber user styles (e.g. font-weight) on theme changes.
    // We append a tiny color rule with a marker so we can update it later.
    const QString marker = fluentLabelThemeMarker();
    const QString user = withoutFluentLabelThemeRule(QLabel::styleSheet());

    const QColor text = isEnabled() ? colors.text : colors.disabledText;
    const QString themed = user + marker + QStringLiteral(" color: %1;").arg(text.name(QColor::HexArgb));
    if (QLabel::styleSheet() != themed) {
        QLabel::setStyleSheet(themed);
    }
}

} // namespace Fluent
