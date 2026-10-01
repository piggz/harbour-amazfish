import QtQuick 2.0
import "GlassStyle.js" as Glass

// Dark translucent surface with hairline border and a light top edge.
// No real backdrop blur: on Sailfish (Qt 5.6) that would mean re-rendering and
// blurring the ambience behind every tile on each frame.
Rectangle {
    id: panel

    property bool pressed: false
    property bool round: false          // fully rounded ends (pills, circular buttons)

    radius: round ? Math.min(width, height) / 2 : Glass.radiusLarge(styler)
    color: Glass.fill
    border.width: 1
    border.color: Glass.border

    Rectangle {
        x: panel.radius
        y: 1
        width: Math.max(0, panel.width - 2 * panel.radius)
        height: 1
        color: Glass.edge
    }

    Rectangle {
        anchors.fill: parent
        radius: panel.radius
        color: Glass.pressedOverlay
        visible: panel.pressed
    }
}
