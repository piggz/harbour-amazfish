import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"
import "../components/ChartColors.js" as ChartColors

PagePL {
    id: page
    title: qsTr("Sleep")

    property alias day: nav.day
    property real sleepGoalHours: 8

    // Values of the night ending on 'day', from the same calculation as before
    readonly property var lastNight: summaryChart.points.length ? summaryChart.points[summaryChart.points.length - 1] : null
    readonly property real lightHours: lastNight ? lastNight.y : 0
    readonly property real deepHours: lastNight ? lastNight.z : 0

    pageMenu: PageMenuPL {
        DownloadDataMenuItem{}
    }

    Column {
        id: column
        x: styler.themeHorizontalPageMargin
        width: parent.width - 2 * x
        spacing: styler.themePaddingLarge

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
            title: qsTr("Last night")
            info: phaseChart.noData ? "" : Qt.formatTime(phaseChart.sleepStart, "hh:mm") + " – " + Qt.formatTime(phaseChart.sleepEnd, "hh:mm")
            onClicked: updateGraphs()

            LabelPL {
                id: lblSleepLastnight
                text: ChartColors.formatDuration((lightHours + deepHours) * 60)
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeHuge
            }

            SleepPhaseChart {
                id: phaseChart
            }

            // share of each phase, as one segmented bar
            Row {
                id: shareBar
                width: parent.width
                height: styler.themePaddingMedium
                visible: lightHours + deepHours > 0
                readonly property real total: deepHours + lightHours + phaseChart.awakeMinutes / 60
                Rectangle { width: shareBar.total ? shareBar.width * deepHours / shareBar.total : 0; height: parent.height; color: ChartColors.deepSleep }
                Rectangle { width: shareBar.total ? shareBar.width * lightHours / shareBar.total : 0; height: parent.height; color: ChartColors.lightSleep }
                Rectangle { width: shareBar.total ? shareBar.width * (phaseChart.awakeMinutes / 60) / shareBar.total : 0; height: parent.height; color: ChartColors.awake }
            }

            DetailRow { label: qsTr("Deep sleep"); value: ChartColors.formatDuration(deepHours * 60) }
            DetailRow { label: qsTr("Light sleep"); value: ChartColors.formatDuration(lightHours * 60) }
            DetailRow { label: qsTr("Awake"); value: ChartColors.formatDuration(phaseChart.awakeMinutes); visible: !phaseChart.noData }
        }

        ChartCard {
            title: qsTr("Sleep Summary")
            info: qsTr("Goal %1 h").arg(sleepGoalHours)
            onClicked: updateGraphs()

            SummaryBarChart {
                id: summaryChart
                goal: sleepGoalHours
                colorY: ChartColors.lightSleep
                colorZ: ChartColors.deepSleep
                valueLabel: function(v) {
                    return v.toLocaleString(Qt.locale(), "f", v % 1 ? 1 : 0) + " h";
                }
            }

            ChartLegend {
                items: [
                    { color: ChartColors.deepSleep, label: qsTr("Deep sleep") },
                    { color: ChartColors.lightSleep, label: qsTr("Light sleep") },
                    { color: ChartColors.goal, label: qsTr("Goal"), line: true },
                    { color: ChartColors.average, label: qsTr("Average"), line: true }
                ]
            }

            DetailRow { label: qsTr("Average"); value: ChartColors.formatDuration(summaryChart.average * 60) }
            DetailRow { label: qsTr("Goal reached"); value: qsTr("%1 of %2 nights").arg(summaryChart.goalDays).arg(summaryChart.count) }
        }
    }

    function updateGraphs() {
        summaryChart.points = dataSource.data(DataSource.SleepSummary, day);
        phaseChart.samples = dataSource.data(DataSource.SleepPhases, day);
    }

    Component.onCompleted: {
        day = new Date();
        updateGraphs();
    }
}
