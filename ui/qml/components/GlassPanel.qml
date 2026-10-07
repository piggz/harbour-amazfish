import QtQuick 2.0

// Surface of tiles, cards and grouped lists. No backdrop blur: it is too slow on Qt 5.6.
Rectangle {
    id: panel

    property bool pressed: false
    property bool round: false          // fully rounded ends (pills, circular buttons)

    radius: round ? Math.min(width, height) / 2 : styler.surfaceRadius
    color: styler.surfaceColor
    border.width: 1
    border.color: styler.surfaceBorderColor

    Rectangle {
        x: panel.radius
        y: 1
        width: Math.max(0, panel.width - 2 * panel.radius)
        height: 1
        color: styler.surfaceEdgeColor
    }

    Rectangle {
        anchors.fill: parent
        radius: panel.radius
        color: styler.surfacePressedColor
        visible: panel.pressed
    }
}
