#include "Fluent/FluentDiagnostics.h"

#include <QtGlobal>

#include <atomic>
#include <cstdio>

#include <QByteArray>
#include <QMessageLogContext>
#include <QMutex>
#include <QMutexLocker>
#include <QString>

namespace {

static bool isTruthyEnv(const QByteArray &raw, bool defaultValue)
{
    if (raw.isEmpty()) {
        return defaultValue;
    }
    const QByteArray v = raw.trimmed().toLower();
    if (v == "0" || v == "false" || v == "off" || v == "no") {
        return false;
    }
    if (v == "1" || v == "true" || v == "on" || v == "yes") {
        return true;
    }
    return defaultValue;
}

static bool envSuppressKnownWarnings()
{
    const bool oldName = isTruthyEnv(qgetenv("QTFLUENT_SUPPRESS_KNOWN_PAINT_WARNINGS"), false);
    return isTruthyEnv(qgetenv("QTFLUENT_SUPPRESS_KNOWN_WARNINGS"), oldName);
}

static bool envQtWarningOutputEnabled()
{
    return !isTruthyEnv(qgetenv("QTFLUENT_DISABLE_QT_WARNINGS"), false);
}

static bool shouldAutoInstall()
{
    return envSuppressKnownWarnings() || !envQtWarningOutputEnabled();
}

static bool isKnownNoisyWarning(const QString &msg)
{
    return msg.contains(QStringLiteral("QWidget::paintEngine: Should no longer be called"))
        || msg.contains(QStringLiteral("QPainter::begin: Paint device returned engine == 0"))
        || msg.contains(QStringLiteral("QFont::setPointSize: Point size <= 0"))
        || msg.contains(QStringLiteral("QFont::setPointSizeF: Point size <= 0"));
}

static QtMessageHandler g_prevHandler = nullptr;
static QMutex g_handlerMutex;
static std::atomic_bool g_handlerInstalled {false};
static std::atomic_bool g_suppressKnownWarnings {envSuppressKnownWarnings()};
static std::atomic_bool g_qtWarningOutputEnabled {envQtWarningOutputEnabled()};

static void fluentMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    if (type == QtWarningMsg) {
        if (!g_qtWarningOutputEnabled.load(std::memory_order_relaxed)) {
            return;
        }
        if (g_suppressKnownWarnings.load(std::memory_order_relaxed) && isKnownNoisyWarning(msg)) {
            return;
        }
    }

    if (g_prevHandler) {
        g_prevHandler(type, ctx, msg);
    } else {
        // Fallback to default formatting.
        const QByteArray m = qFormatLogMessage(type, ctx, msg).toLocal8Bit();
        fprintf(stderr, "%s\n", m.constData());
        fflush(stderr);
    }
}

struct FluentDiagnosticsInstaller {
    FluentDiagnosticsInstaller()
    {
        if (shouldAutoInstall()) {
            Fluent::Diagnostics::installMessageHandler();
        }
    }
};

static const FluentDiagnosticsInstaller g_installer;

} // namespace

namespace Fluent {

void Diagnostics::installMessageHandler()
{
    QMutexLocker locker(&g_handlerMutex);
    if (g_handlerInstalled.load(std::memory_order_relaxed)) {
        return;
    }

    g_prevHandler = qInstallMessageHandler(fluentMessageHandler);
    g_handlerInstalled.store(true, std::memory_order_release);
}

bool Diagnostics::messageHandlerInstalled()
{
    return g_handlerInstalled.load(std::memory_order_acquire);
}

void Diagnostics::setKnownQtWarningSuppressionEnabled(bool enabled)
{
    g_suppressKnownWarnings.store(enabled, std::memory_order_release);
    installMessageHandler();
}

bool Diagnostics::knownQtWarningSuppressionEnabled()
{
    return g_suppressKnownWarnings.load(std::memory_order_acquire);
}

void Diagnostics::setQtWarningOutputEnabled(bool enabled)
{
    g_qtWarningOutputEnabled.store(enabled, std::memory_order_release);
    installMessageHandler();
}

bool Diagnostics::qtWarningOutputEnabled()
{
    return g_qtWarningOutputEnabled.load(std::memory_order_acquire);
}

} // namespace Fluent
