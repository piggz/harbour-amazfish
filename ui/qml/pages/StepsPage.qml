import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"
import "../components/ChartTools.js" as ChartTools

PagePL {
    id: page
    title: qsTr("Steps")

    property alias day: nav.day
    readonly property int stepGoal: AmazfishConfig.profileFitnessGoal

    pageMenu: PageMenuPL {
        DownloadDataMenuItem{}
    }

    function fmt(v) {
        return Number(v).toLocaleString(Qt.locale(), "f", 0);
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
            title: qsTr("Steps")
            info: qsTr("Goal %1").arg(fmt(stepGoal))
            onClicked: updateGraphs()

            Row {
                spacing: styler.themePaddingMedium
                LabelPL {
                    id: lblStepsToday
                    text: fmt(stepChart.lastValue)
                    color: styler.themeHighlightColor
                    font.pixelSize: styler.themeFontSizeHuge
                }
                LabelPL {
                    anchors.baseline: lblStepsToday.baseline
                    text: qsTr("steps")
                    color: styler.themeSecondaryHighlightColor
                    font.pixelSize: styler.themeFontSizeMedium
                }
            }

            SummaryBarChart {
                id: stepChart
                goal: stepGoal
                colorBelowGoal: styler.chartBelowGoalColor
                averageLabel: function(v) { return fmt(v); }
                valueLabel: function(v) {
                    return v >= 1000 ? (v / 1000).toLocaleString(Qt.locale(), "f", v % 1000 ? 1 : 0) + "k" : fmt(v);
                }
            }

            ChartLegend {
                items: [
                    { color: styler.chartActiveColor, label: qsTr("Goal reached") },
                    { color: styler.chartBelowGoalColor, label: qsTr("Below goal") },
                    { color: styler.chartGoalColor, label: qsTr("Daily goal"), line: true },
                    { color: styler.chartAverageColor, label: qsTr("Average"), line: true }
                ]
            }
        }

        ChartCard {
            title: qsTr("Last %n day(s)", "", stepChart.count)
            visible: !stepChart.noData

            DetailRow { label: qsTr("Average"); value: qsTr("%1 steps").arg(fmt(stepChart.average)) }
            DetailRow { label: qsTr("Total"); value: qsTr("%1 steps").arg(fmt(stepChart.total)) }
            DetailRow {
                label: qsTr("Goal reached")
                value: qsTr("%1 of %2 days").arg(stepChart.goalDays).arg(stepChart.count)
            }
            DetailRow {
                label: qsTr("Best day")
                value: stepChart.bestTime
                       ? Qt.formatDate(new Date(stepChart.bestTime * 1000), "ddd d.M.") + " - " + fmt(stepChart.best)
                       : "-"
            }
        }
    }

    function updateGraphs() {
        stepChart.points = dataSource.data(DataSource.StepSummary, day);
    }

    Component.onCompleted: {
        day = new Date();
        updateGraphs();
        _InfoSteps = parseInt(DaemonInterfaceInstance.information(Amazfish.INFO_STEPS), 10) || 0;
    }
}
