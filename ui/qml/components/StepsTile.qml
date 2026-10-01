import QtQuick 2.0
import QtQuick.Layouts 1.1
import "./platform"
import "GlassStyle.js" as Glass

// Steps tile: goal gauge with the step count in the middle.
Tile {
    id: tile

    property int stepCount: 0
    property int stepGoal: 0

    // bottom of the tile: progress towards the daily goal
    value: stepGoal > 0 ? qsTr("%1 %").arg(Math.round(stepCount / stepGoal * 100)) : ""
    text: stepGoal > 0 ? qsTr("Steps · daily goal") : qsTr("Steps")

    contentItem: GaugeArc {
        id: gauge
        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height)
        height: width
        value: stepGoal > 0 ? stepCount / stepGoal : 0
        color: styler.themeHighlightColor
        trackColor: Glass.track

        Column {
            anchors.centerIn: parent
            width: gauge.width * 0.7

            LabelPL {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                color: styler.themePrimaryColor
                font.pixelSize: Math.min(styler.themeFontSizeLarge, gauge.width * 0.22)
                fontSizeMode: Text.HorizontalFit
                minimumPixelSize: font.pixelSize * 0.6
                text: Number(stepCount).toLocaleString(Qt.locale(), "f", 0)
            }
            LabelPL {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                color: styler.themeSecondaryColor
                font.pixelSize: Math.min(styler.themeFontSizeExtraSmall, gauge.width * 0.11)
                fontSizeMode: Text.HorizontalFit
                minimumPixelSize: font.pixelSize * 0.6
                text: stepGoal > 0 ? qsTr("of %1").arg(Number(stepGoal).toLocaleString(Qt.locale(), "f", 0)) : ""
            }
        }
    }
}
