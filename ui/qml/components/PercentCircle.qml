import QtQuick 2.3
import QtGraphicalEffects 1.0
import Sailfish.Silica 1.0

Rectangle {

    property double percent
    property int size

    color: "transparent"
    implicitWidth: size
    implicitHeight: implicitWidth

    Rectangle
    {
        id: outerRing
        z: 0
        anchors.fill: parent
        radius: Math.max(width, height) / 2
        color: "transparent"
        border.color: Theme.secondaryColor
        border.width: 12
    }

    Rectangle
    {
        id: innerRing
        z: 1
        anchors.fill: parent
        anchors.margins: (outerRing.border.width - border.width) / 2
        radius: outerRing.radius
        color: "transparent"
        border.color: Theme.secondaryColor
        border.width: 6

        ConicalGradient
        {
            source: innerRing
            anchors.fill: parent
            gradient: Gradient
            {
                GradientStop { position: 0.00; color: Theme.highlightColor }
                GradientStop { position: percent; color: Theme.highlightColor }
                GradientStop { position: percent + 0.01; color: "transparent" }
                GradientStop { position: 1.00; color: "transparent" }
            }
        }
    }

//    Text
//    {
//        id: progressLabel
//        anchors.centerIn: parent
//        color: Theme.highlightColor
//        text: (percent * 100).toFixed() + "%"
//    }
}
