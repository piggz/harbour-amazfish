import QtQuick 2.0

// Canvas that repaints itself whenever it can be seen again.
// Sailfish's page stack hides the pages below the current one, and a hidden Canvas
// may lose its content on Qt 5.6 without painting again by itself (the steps ring on
// the first page was empty after coming back from the Data page). Same when the app
// returns from the background or the scene graph was recreated.
Canvas {
    id: chartCanvas

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
