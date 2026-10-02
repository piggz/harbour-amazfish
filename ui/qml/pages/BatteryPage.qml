import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"
import "../components/ChartColors.js" as ChartColors

PagePL {
    id: page
    title: qsTr("Battery")

    property alias day: nav.day
    property var points: []
    readonly property real lastLevel: points.length ? points[points.length - 1].y : -1

    function formatTime(seconds) {
        if (!seconds) {
            return "–";
        }
        var d = new Date(seconds * 1000);
        return Qt.formatDate(d, "ddd d.M.") + " · " + d.toLocaleTimeString(Qt.locale(), Locale.ShortFormat);
    }

    Column {
        id: column
        x: styler.themeHorizontalPageMargin
        width: parent.width - 2 * x
        spacing: styler.themePaddingLarge

        LabelPL {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: lastLevel >= 0 ? qsTr("%1 %").arg(Math.round(lastLevel)) : "–"
            color: lastLevel >= 0 ? ChartColors.batteryColor(lastLevel) : styler.themeSecondaryColor
            font.pixelSize: styler.themeFontSizeExtraLarge * 2
        }

        DateNavigation {
            id: nav
            onBackward: {
                var d = new Date(day);
                d.setDate(day.getDate() - 1);
                day = d;
            }
            onForward: {
                var d = new Date(day);
                d.setDate(day.getDate() + 1);
                day = d;
            }
            onDayChanged: {
                updateGraphs();
            }
        }

        ChartCard {
            title: qsTr("Battery")
            info: qsTr("Last %n day(s)", "", 11)
            onClicked: updateGraphs()

            LevelBarChart {
                id: batteryChart
                points: page.points
            }

            ChartLegend {
                items: [
                    { color: ChartColors.batteryHigh, label: qsTr("50 % and more") },
                    { color: ChartColors.batteryMedium, label: qsTr("20 – 49 %") },
                    { color: ChartColors.batteryLow, label: qsTr("below 20 %") }
                ]
            }
        }

        ChartCard {
            title: qsTr("Details")
            visible: !batteryChart.noData

            DetailRow {
                label: qsTr("Current")
                value: lastLevel >= 0 ? qsTr("%1 %").arg(Math.round(lastLevel)) : "–"
            }
            DetailRow {
                label: qsTr("Lowest")
                value: batteryChart.lowest >= 0
                       ? qsTr("%1 %").arg(Math.round(batteryChart.lowest)) + " · " + formatTime(batteryChart.lowestTime)
                       : "–"
            }
            DetailRow {
                label: qsTr("Last charged")
                value: formatTime(batteryChart.lastChargeTime)
            }
        }
    }

    function updateGraphs() {
        // same window as the query: 10 days before 'day' up to the end of 'day'
        var end = new Date(day);
        end.setHours(0, 0, 0, 0);
        end.setDate(end.getDate() + 1);
        var start = new Date(end);
        start.setDate(start.getDate() - 11);
        batteryChart.startTime = start.getTime() / 1000;
        batteryChart.endTime = end.getTime() / 1000;
        page.points = dataSource.data(DataSource.BatteryLog, day);
    }

    Component.onCompleted: {
        day = new Date();
        updateGraphs();
    }
}
