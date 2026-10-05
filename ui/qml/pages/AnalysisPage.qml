import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"
import "../components/ChartTools.js" as ChartTools

PagePL {
    id: page
    title: qsTr("Analysis")
    property alias day: nav.day
    property int totalSteps: 0

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
            text: qsTr("Download All Data")
            onClicked: DaemonInterfaceInstance.fetchData(Amazfish.TYPE_HRV | Amazfish.TYPE_TEMPERATURE | Amazfish.TYPE_ACTIVITY);
        }
    }

    // Place our content in a Column.  The PageHeader is always placed at the top
    // of the page, followed by our content.
    Column {
        id: column
        x: styler.themeHorizontalPageMargin
        width: parent.width - 2 * x
        anchors.top: parent.top
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
            title: qsTr("Activity")
            info: activityChart.noData ? "" : qsTr("Avg. %1 BPM").arg(activityChart.averageHeartrate)
            onClicked: updateGraphs()

            ActivityChart {
                id: activityChart
                showHeartrate: supportsDataRefresh(Amazfish.TYPE_HEART_RATE)
            }

            ChartLegend {
                items: [
                    { color: styler.chartDeepSleepColor, label: qsTr("Deep sleep") },
                    { color: styler.chartLightSleepColor, label: qsTr("Light sleep") },
                    { color: styler.chartActiveColor, label: qsTr("Active") },
                    { color: styler.chartInactiveColor, label: qsTr("Inactive") }
                ].concat(activityChart.showHeartrate
                          ? [{ color: styler.chartHeartRateColor, label: qsTr("Heartrate"), line: true }] : [])
            }

            DetailRow { label: qsTr("Steps"); value: Number(totalSteps).toLocaleString(Qt.locale(), "f", 0) }
            DetailRow { label: qsTr("Active"); value: ChartTools.formatDuration(activityChart.activeMinutes) }
            DetailRow {
                label: qsTr("Sleep")
                value: ChartTools.formatDuration(activityChart.lightMinutes + activityChart.deepMinutes)
            }
        }

        ChartCard {
            visible: supportsDataRefresh(Amazfish.TYPE_HRV)
            Graph {
                id: graphHRV
                graphTitle: qsTr("HRV")
                graphHeight: 300

                axisY.units: ""
                type: DataSource.HRV

                visible: supportsDataRefresh(Amazfish.TYPE_HRV)

                minY: 0
                maxY: 100

                onClicked: {
                    updateGraph(day);
                }
            }
        }

        ChartCard {
            visible: supportsDataRefresh(Amazfish.TYPE_TEMPERATURE)
            Graph {
                id: graphBodyTemperature
                graphTitle: qsTr("Body Temperature")
                graphHeight: 300

                axisY.units: "\u00b0C"
                axisX.mask: "hh:mm"

                visible: supportsDataRefresh(Amazfish.TYPE_TEMPERATURE)

                type: DataSource.BodyTemperature
                graphType: line

                minY: -20
                maxY: 50

                onClicked: {
                    updateGraph(day);
                }
            }
        }

    }

    function updateGraphs() {
        var start = new Date(day);
        start.setHours(0, 0, 0, 0);
        var samples = dataSource.data(DataSource.Activity, day);
        var steps = 0;
        for (var i = 0; i < samples.length; i++) {
            steps += samples[i].s;
        }
        totalSteps = steps;
        activityChart.startTime = start.getTime() / 1000;
        activityChart.samples = samples;
        graphHRV.updateGraph(day);
        graphBodyTemperature.updateGraph(day);
    }

    Component.onCompleted: {
        day = new Date();
    }
}
