#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QTimer>

#include "Fluent/FluentTheme.h"

#include "DemoHelpers.h"
#include "DemoWindow.h"

int main(int argc, char *argv[])
{
    QElapsedTimer startupTimer;
    startupTimer.start();

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    qInfo().noquote() << QStringLiteral("[DemoStartup] QApplication constructed at %1 ms").arg(startupTimer.elapsed());

    QCoreApplication::setOrganizationName(QStringLiteral("QtFluent"));
    QCoreApplication::setApplicationName(QStringLiteral("QtFluentDemo"));
    Demo::initializeLanguage();
    qInfo().noquote() << QStringLiteral("[DemoStartup] language initialized at %1 ms").arg(startupTimer.elapsed());

    Demo::DemoWindow w;
    qInfo().noquote() << QStringLiteral("[DemoStartup] DemoWindow constructed at %1 ms").arg(startupTimer.elapsed());

    w.resize(1260, 780);
    qInfo().noquote() << QStringLiteral("[DemoStartup] DemoWindow resized at %1 ms").arg(startupTimer.elapsed());

    w.show();
    qInfo().noquote() << QStringLiteral("[DemoStartup] DemoWindow show() returned at %1 ms").arg(startupTimer.elapsed());

    QTimer::singleShot(0, &app, [&startupTimer]() {
        qInfo().noquote() << QStringLiteral("[DemoStartup] first event loop tick at %1 ms").arg(startupTimer.elapsed());
    });

    return app.exec();
}
