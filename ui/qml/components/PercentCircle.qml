import QtQuick 2.3
import QtGraphicalEffects 1.0

Rectangle {

    property double percent
    property int size
    property real widthRatio

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
        border.color: styler.themeSecondaryColor
        border.width: width * widthRatio
    }

    Rectangle
    {
        id: innerRing
        z: 1
        anchors.fill: parent
        anchors.margins: (outerRing.border.width - border.width) / 2
        radius: outerRing.radius
        color: "transparent"
        border.color: styler.themeSecondaryColor
        border.width: width * (widthRatio * 0.6)

        ConicalGradient
        {
            source: innerRing
            anchors.fill: parent
            gradient: Gradient
            {
                GradientStop { position: 0.00; color: styler.themeSecondaryHighlightColor }
                GradientStop { position: percent; color: styler.themeSecondaryHighlightColor }
                GradientStop { position: percent + 0.01; color: "transparent" }
                GradientStop { position: 1.00; color: "transparent" }
            }
        }
    }
}
