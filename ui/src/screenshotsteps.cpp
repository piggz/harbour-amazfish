#include "screenshotsteps.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QImage>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExpression>
#include <QQuickWindow>
#include <QTextStream>
#include <QTimer>

void runScreenshotSteps(QQuickWindow* window, const QString& stepsFile, const QString& outputDir)
{
    QFile file(stepsFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Screenshot steps: cannot open" << stepsFile;
        QCoreApplication::exit(1);
        return;
    }
    const QString dir = outputDir.isEmpty() ? QDir::currentPath() : outputDir;
    QDir().mkpath(dir);

    // The offscreen platform has no icon theme and searches only :/icons.
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << QStringLiteral("/usr/share/icons"));
    QIcon::setFallbackThemeName(QStringLiteral("breeze"));

    // Each step is scheduled at the sum of the waits before it.
    static bool failed = false;
    int time = 0;
    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) {
            continue;
        }
        const QString command = line.section(QLatin1Char(' '), 0, 0);
        const QString argument = line.section(QLatin1Char(' '), 1).trimmed();

        if (command == QLatin1String("wait")) {
            time += argument.toInt();
        } else if (command == QLatin1String("js")) {
            QTimer::singleShot(time, window, [window, argument]() {
                QQmlContext* context = qmlContext(window);
                QQmlExpression expression(context, window, argument);
                expression.evaluate();
                if (expression.hasError()) {
                    qWarning() << "Screenshot steps: js failed:" << argument << expression.error().toString();
                    failed = true;
                }
            });
        } else if (command == QLatin1String("shot")) {
            const QString path = dir + QLatin1Char('/') + argument + QLatin1String(".png");
            QTimer::singleShot(time, window, [window, path]() {
                if (!window->grabWindow().save(path)) {
                    qWarning() << "Screenshot steps: cannot save" << path;
                    failed = true;
                }
            });
        } else {
            qWarning() << "Screenshot steps: unknown step:" << line;
            failed = true;
        }
    }

    QTimer::singleShot(time, window, []() {
        QCoreApplication::exit(failed ? 1 : 0);
    });
}
