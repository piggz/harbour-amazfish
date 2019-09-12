import QtQuick 2.0
import Sailfish.Silica 1.0

MoreButton {
    height: Theme.itemSizeMedium

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: Theme.rgba(Theme.highlightBackgroundColor, 0.15) }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }
}
