import QtQuick 2.0

// Canvas that repaints when it becomes visible again.
// On Qt 5.6 a hidden Canvas can lose its content and does not repaint by itself.
Canvas {
    id: chartCanvas

    // Every chart colour derives from these, so a theme or ambience change repaints the chart.
    readonly property var themeColors: [styler.themePrimaryColor, styler.themeSecondaryColor,
                                        styler.themeHighlightColor, styler.chartGridColor]
    onThemeColorsChanged: requestPaint()

    onVisibleChanged: if (visible) requestPaint()
    onAvailableChanged: if (available) requestPaint()

    Connections {
        target: Qt.application
        onStateChanged: {
            if (Qt.application.state === Qt.ApplicationActive && chartCanvas.visible) {
                chartCanvas.requestPaint();
            }
        }
    }
}
