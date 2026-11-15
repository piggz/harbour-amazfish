#ifndef SCREENSHOTSTEPS_H
#define SCREENSHOTSTEPS_H

#include <QString>

class QQuickWindow;

// Runs the steps in stepsFile, then quits the app with 0, or 1 if a step failed. One step per line:
//   wait MS          wait before the next step
//   js EXPRESSION    evaluate a QML expression in the root context, for example app.pages.push(...)
//   shot NAME        save the window to outputDir/NAME.png (default: the current directory)
// Empty lines and lines starting with # are ignored.
void runScreenshotSteps(QQuickWindow* window, const QString& stepsFile, const QString& outputDir);

#endif // SCREENSHOTSTEPS_H
