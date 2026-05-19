#include "Fluent/FluentInfoBar.h"

#include "Fluent/FluentButton.h"
#include "Fluent/FluentLabel.h"
#include "Fluent/FluentStyle.h"
#include "Fluent/FluentTheme.h"
#include "Fluent/FluentToolButton.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QVBoxLayout>

namespace Fluent {

namespace {

QColor severityColor(FluentInfoBar::Severity severity, const ThemeColors &colors)
{
    switch (severity) {
    case FluentInfoBar::Severity::Success:
        return QColor(QStringLiteral("#0F7B0F"));
    case FluentInfoBar::Severity::Warning:
        return QColor(QStringLiteral("#F9A825"));
    case FluentInfoBar::Severity::Error:
        return colors.error.isValid() ? colors.error : QColor(QStringLiteral("#C42B1C"));
    case FluentInfoBar::Severity::Info:
    default:
        return colors.accent;
    }
}

QString severityGlyph(FluentInfoBar::Severity severity)
{
    switch (severity) {
    case FluentInfoBar::Severity::Success:
        return QStringLiteral("OK");
    case FluentInfoBar::Severity::Warning:
        return QStringLiteral("!");
    case FluentInfoBar::Severity::Error:
        return QStringLiteral("X");
    case FluentInfoBar::Severity::Info:
    default:
        return QStringLiteral("i");
    }
}

} // namespace

FluentInfoBar::FluentInfoBar(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(48);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(14, 10, 10, 10);
    m_layout->setSpacing(10);

    m_icon = new FluentLabel(this);
    m_icon->setAlignment(Qt::AlignCenter);
    m_icon->setFixedSize(24, 24);
    m_icon->setStyleSheet(QStringLiteral("font-weight: 700;"));

    auto *textColumn = new QVBoxLayout();
    textColumn->setContentsMargins(0, 0, 0, 0);
    textColumn->setSpacing(2);

    m_titleLabel = new FluentLabel(this);
    m_titleLabel->setStyleSheet(QStringLiteral("font-size: 13px; font-weight: 650;"));
    m_messageLabel = new FluentLabel(this);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setStyleSheet(QStringLiteral("font-size: 12px; opacity: 0.88;"));

    textColumn->addWidget(m_titleLabel);
    textColumn->addWidget(m_messageLabel);

    m_actionButton = new FluentButton(this);
    m_actionButton->setVisible(false);
    connect(m_actionButton, &QPushButton::clicked, this, &FluentInfoBar::actionTriggered);

    m_closeButton = new FluentToolButton(this);
    m_closeButton->setProperty("fluentWindowGlyph", 3);
    m_closeButton->setFixedSize(34, 28);
    connect(m_closeButton, &QToolButton::clicked, this, [this]() {
        hide();
        emit closed();
    });

    m_layout->addWidget(m_icon);
    m_layout->addLayout(textColumn, 1);
    m_layout->addWidget(m_actionButton);
    m_layout->addWidget(m_closeButton);

    applyTheme();
    updateContent();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &FluentInfoBar::applyTheme);
}

FluentInfoBar::FluentInfoBar(Severity severity, const QString &title, const QString &message, QWidget *parent)
    : FluentInfoBar(parent)
{
    m_severity = severity;
    m_title = title;
    m_message = message;
    updateContent();
}

FluentInfoBar::Severity FluentInfoBar::severity() const
{
    return m_severity;
}

void FluentInfoBar::setSeverity(Severity severity)
{
    if (m_severity == severity) {
        return;
    }
    m_severity = severity;
    updateContent();
    update();
}

QString FluentInfoBar::title() const
{
    return m_title;
}

void FluentInfoBar::setTitle(const QString &title)
{
    if (m_title == title) {
        return;
    }
    m_title = title;
    updateContent();
}

QString FluentInfoBar::message() const
{
    return m_message;
}

void FluentInfoBar::setMessage(const QString &message)
{
    if (m_message == message) {
        return;
    }
    m_message = message;
    updateContent();
}

bool FluentInfoBar::isClosable() const
{
    return m_closable;
}

void FluentInfoBar::setClosable(bool closable)
{
    if (m_closable == closable) {
        return;
    }
    m_closable = closable;
    updateContent();
}

QString FluentInfoBar::actionText() const
{
    return m_actionText;
}

void FluentInfoBar::setActionText(const QString &text)
{
    if (m_actionText == text) {
        return;
    }
    m_actionText = text;
    updateContent();
}

void FluentInfoBar::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        applyTheme();
    }
}

void FluentInfoBar::applyTheme()
{
    const auto &colors = ThemeManager::instance().colors();
    const QColor accent = severityColor(m_severity, colors);
    m_icon->setStyleSheet(QStringLiteral("font-size: 12px; font-weight: 700; color: %1;").arg(accent.name()));
    update();
}

void FluentInfoBar::updateContent()
{
    m_icon->setText(severityGlyph(m_severity));
    m_titleLabel->setText(m_title);
    m_titleLabel->setVisible(!m_title.isEmpty());
    m_messageLabel->setText(m_message);
    m_messageLabel->setVisible(!m_message.isEmpty());
    m_actionButton->setText(m_actionText);
    m_actionButton->setVisible(!m_actionText.isEmpty());
    m_closeButton->setVisible(m_closable);
    applyTheme();
}

void FluentInfoBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    const auto &colors = ThemeManager::instance().colors();
    const QColor accent = severityColor(m_severity, colors);

    QColor fill = Style::mix(colors.surface, accent, ThemeManager::instance().themeMode() == ThemeManager::ThemeMode::Dark ? 0.10 : 0.07);
    QColor border = Style::mix(colors.border, accent, 0.35);
    if (!isEnabled()) {
        fill = Style::mix(colors.surface, colors.hover, 0.35);
        border = Style::mix(colors.border, colors.disabledText, 0.25);
    }

    QPainter p(this);
    if (!p.isActive()) {
        return;
    }
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    p.setPen(QPen(border, 1.0));
    p.setBrush(fill);
    p.drawRoundedRect(r, 8.0, 8.0);

    p.setPen(Qt::NoPen);
    p.setBrush(accent);
    p.drawRoundedRect(QRectF(r.left() + 1.0, r.top() + 8.0, 3.0, r.height() - 16.0), 1.5, 1.5);
}

} // namespace Fluent
