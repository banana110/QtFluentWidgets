#pragma once

#include <QObject>
#include <QColor>
#include <QString>

namespace Fluent {

struct ThemeColors {
    QColor accent;
    QColor text;
    QColor subText;
    QColor disabledText;
    QColor background;
    QColor surface;
    QColor border;
    QColor hover;
    QColor pressed;
    QColor focus;
    QColor error;
};

struct FluentAccentRamp {
    QColor base;
    QColor light1;
    QColor light2;
    QColor light3;
    QColor dark1;
    QColor dark2;
    QColor dark3;
};

struct FluentNeutralRamp {
    QColor background;
    QColor layer;
    QColor layerAlt;
    QColor card;
    QColor cardHover;
    QColor stroke;
    QColor strokeSubtle;
    QColor strokeStrong;
    QColor fillSecondary;
    QColor fillTertiary;
};

struct FluentSemanticRamp {
    QColor info;
    QColor success;
    QColor warning;
    QColor error;
};

struct FluentRadiusTokens {
    int control = 6;
    int overlay = 8;
    int card = 10;
    int window = 12;
    int pill = 999;
};

struct FluentSpacingTokens {
    int xxs = 2;
    int xs = 4;
    int s = 6;
    int m = 8;
    int l = 12;
    int xl = 16;
    int xxl = 24;
};

struct FluentTypographyTokens {
    QString family = QStringLiteral("'Segoe UI', 'Microsoft YaHei UI', 'Microsoft YaHei', sans-serif");
    int caption = 12;
    int body = 14;
    int bodyLarge = 16;
    int subtitle = 18;
    int title = 20;
    int titleLarge = 28;
};

struct FluentElevationTokens {
    QColor shadow;
    int lowBlur = 10;
    int mediumBlur = 18;
    int highBlur = 28;
    int lowYOffset = 2;
    int mediumYOffset = 6;
    int highYOffset = 12;
};

struct FluentThemeTokens {
    ThemeColors legacyColors;
    FluentAccentRamp accent;
    FluentNeutralRamp neutral;
    FluentSemanticRamp semantic;
    FluentRadiusTokens radius;
    FluentSpacingTokens spacing;
    FluentTypographyTokens typography;
    FluentElevationTokens elevation;
    QColor onAccent;
    bool dark = false;
};

class Theme final
{
public:
    static ThemeColors light();
    static ThemeColors dark();  // Added dark mode support
    static bool isDark(const ThemeColors &colors);
    static QColor contrastColor(const QColor &background);
    static QColor accentForMode(const QColor &accent, bool dark);
    static FluentAccentRamp accentRamp(const QColor &accent, bool dark);
    static FluentNeutralRamp neutralRamp(const ThemeColors &colors);
    static FluentSemanticRamp semanticRamp(const ThemeColors &colors);
    static FluentThemeTokens tokens(const ThemeColors &colors);
    static QString baseStyleSheet(const ThemeColors &colors);
    static QString buttonStyle(const ThemeColors &colors, bool primary);
    static QString labelStyle(const ThemeColors &colors);
    static QString lineEditStyle(const ThemeColors &colors);
    static QString textEditStyle(const ThemeColors &colors);
    static QString dateTimeStyle(const ThemeColors &colors);
    static QString calendarPopupStyle(const ThemeColors &colors);
    static QString checkBoxStyle(const ThemeColors &colors);
    static QString radioButtonStyle(const ThemeColors &colors);
    static QString toggleSwitchStyle(const ThemeColors &colors);
    static QString comboBoxStyle(const ThemeColors &colors);
    static QString sliderStyle(const ThemeColors &colors);
    static QString progressBarStyle(const ThemeColors &colors);
    static QString spinBoxStyle(const ThemeColors &colors);
    static QString toolButtonStyle(const ThemeColors &colors);
    static QString tabWidgetStyle(const ThemeColors &colors);
    static QString listViewStyle(const ThemeColors &colors);
    static QString tableViewStyle(const ThemeColors &colors);
    static QString treeViewStyle(const ThemeColors &colors);
    static QString groupBoxStyle(const ThemeColors &colors);
    static QString menuBarStyle(const ThemeColors &colors);
    static QString toolBarStyle(const ThemeColors &colors);
    static QString statusBarStyle(const ThemeColors &colors);
    static QString dialogStyle(const ThemeColors &colors);
    static QString cardStyle(const ThemeColors &colors);
};

class ThemeManager final : public QObject
{
    Q_OBJECT
public:
    enum class ThemeMode {
        Light,
        Dark
    };
    
    static ThemeManager &instance();

    const ThemeColors &colors() const;
    const FluentThemeTokens &tokens() const;
    void setColors(const ThemeColors &colors);
    void setAccentColor(const QColor &accent);

    bool accentBorderEnabled() const;
    void setAccentBorderEnabled(bool enabled);
    
    ThemeMode themeMode() const;
    void setThemeMode(ThemeMode mode);
    void setLightMode();
    void setDarkMode();

signals:
    void themeChanged();

private:
    ThemeManager();

    void scheduleThemeChanged();
    void setColorsInternal(const ThemeColors &colors, bool updateBaseAccent);

    ThemeColors m_colors;
    FluentThemeTokens m_tokens;
    QColor m_baseAccent;
    ThemeMode m_mode = ThemeMode::Light;

    bool m_accentBorderEnabled = true;
    bool m_themeChangedPending = false;
};

} // namespace Fluent
