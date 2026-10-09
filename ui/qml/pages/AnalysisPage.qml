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
            title: qsTr("HRV")
            info: hrvChart.noData ? "" : qsTr("Avg. %1").arg(Math.round(hrvChart.average))
            visible: supportsDataRefresh(Amazfish.TYPE_HRV)
            onClicked: updateGraphs()

            DayChart {
                id: hrvChart
                color: styler.chartHrvColor
                fillColor: styler.chartHrvFillColor
            }
        }

        ChartCard {
            title: qsTr("Body Temperature")
            info: temperatureChart.noData ? ""
                                          : qsTr("Avg. %1").arg(temperatureChart.valueLabel(temperatureChart.average))
            visible: supportsDataRefresh(Amazfish.TYPE_TEMPERATURE)
            onClicked: updateGraphs()

            DayChart {
                id: temperatureChart
                color: styler.chartTemperatureColor
                fillColor: styler.chartTemperatureFillColor
                valueLabel: function(v) { return v.toLocaleString(Qt.locale(), "f", 1) + " \u00b0C"; }
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
        hrvChart.startTime = start.getTime() / 1000;
        hrvChart.points = dataSource.data(DataSource.HRV, day);
        temperatureChart.startTime = start.getTime() / 1000;
        temperatureChart.points = dataSource.data(DataSource.BodyTemperature, day);
    }

    Component.onCompleted: {
        day = new Date();
    }
}
