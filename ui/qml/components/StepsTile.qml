import QtQuick 2.0
import QtQuick.Layouts 1.1
import uk.co.piggz.amazfish 1.0
import "./platform"
import "ChartColors.js" as ChartColors
import "GlassStyle.js" as Glass

// Gadgetbridge-style "Today" widget: 24 h ring coloured by sleep / activity,
// with a goal gauge and the step count in the middle.
Tile {
    id: tile

    property int stepCount: 0
    property int stepGoal: 0
    property var samples: []
    property var startTime: 0

    // bottom of the tile: progress towards the daily goal
    value: stepGoal > 0 ? qsTr("%1 %").arg(Math.round(stepCount / stepGoal * 100)) : ""
    text: stepGoal > 0 ? qsTr("Steps · daily goal") : qsTr("Steps")

    function refresh() {
        var start = new Date();
        start.setHours(0, 0, 0, 0);
        startTime = start.getTime() / 1000;
        samples = dataSource.data(DataSource.Activity, new Date());
    }

    // new samples arrive together with new step counts; don't query the database on every tick
    onStepCountChanged: refreshTimer.restart()
    Timer {
        id: refreshTimer
        interval: 5000
        onTriggered: tile.refresh()
    }

    Component.onCompleted: refresh()

    // Reload when a download from the watch has finished: new sleep or activity
    // data may have arrived while the step count stayed the same (e.g. in the
    // morning before the first steps).
    Connections {
        target: DaemonInterfaceInstance
        onOperationRunningChanged: {
            if (!DaemonInterfaceInstance.operationRunning) {
                refreshTimer.stop();
                tile.refresh();
            }
        }
    }

    // Also when the app returns to the foreground; refresh() starts the ring at
    // today's midnight, so this covers a change of day as well.
    Connections {
        target: Qt.application
        onStateChanged: {
            if (Qt.application.state === Qt.ApplicationActive) {
                tile.refresh();
            }
        }
    }

    // Ring on the left, legend on the right (mockup variant B)
    contentItem: Item {
        id: area
        anchors.fill: parent

        DayRing {
            id: ring
            anchors.verticalCenter: parent.verticalCenter
            width: Math.min(area.height, area.width * 0.58)
            height: width
            samples: tile.samples
            startTime: tile.startTime
            showHourMarks: true

            GaugeArc {
                id: gauge
                anchors.centerIn: parent
                width: ring.innerRadius * 2
                height: width
                value: stepGoal > 0 ? stepCount / stepGoal : 0
                color: styler.themeHighlightColor
                trackColor: Glass.track
                lineWidth: ring.lineWidth * 0.75

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

        Column {
            id: legend
            anchors.left: ring.right
            anchors.leftMargin: styler.themePaddingMedium
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: styler.themePaddingSmall

            Repeater {
                // colours come from the same values the ring is drawn with
                model: [
                    { color: ChartColors.deepSleep, label: qsTr("Deep sleep"), line: false },
                    { color: ChartColors.lightSleep, label: qsTr("Light sleep"), line: false },
                    { color: ChartColors.active, label: qsTr("Active"), line: false },
                    { color: ring.idleColor, label: qsTr("Inactive"), line: false },
                    { color: styler.themeHighlightColor, label: qsTr("Step goal"), line: true }
                ]
                delegate: Item {
                    width: legend.width
                    height: legendLabel.height

                    Rectangle {
                        id: swatch
                        anchors.verticalCenter: parent.verticalCenter
                        width: modelData.line ? legendLabel.font.pixelSize * 1.2 : legendLabel.font.pixelSize * 0.7
                        height: modelData.line ? Math.max(2, legendLabel.font.pixelSize * 0.3) : width
                        radius: height / 2
                        color: modelData.color
                    }
                    LabelPL {
                        id: legendLabel
                        anchors.left: swatch.right
                        anchors.leftMargin: styler.themePaddingSmall
                        anchors.right: parent.right
                        text: modelData.label
                        color: styler.themeSecondaryColor
                        font.pixelSize: styler.themeFontSizeExtraSmall
                        // long translations shrink a little before they are faded out
                        fontSizeMode: Text.HorizontalFit
                        minimumPixelSize: styler.themeFontSizeExtraSmall * 0.75
                        truncMode: truncModes.fade
                    }
                }
            }
        }
    }
}
