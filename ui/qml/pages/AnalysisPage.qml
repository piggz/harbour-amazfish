import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Analysis")
    property alias day: nav.day

    pageMenu: PageMenuPL {
        DownloadDataMenuItem{}
    }

    // Place our content in a Column.  The PageHeader is always placed at the top
    // of the page, followed by our content.
    Column {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
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


        Graph {
            id: graphHeartrate
            graphTitle: qsTr("Heartrate")
            graphHeight: 300

            axisY.units: qsTr("BPM")
            type: DataSource.Heartrate

            minY: 0
            maxY: 200
            valueConverter: function(value) {
                return value.toFixed(0);
            }
            onClicked: {
                updateGraph(day);
            }
        }
        Graph {
            id: graphSteps
            graphTitle: qsTr("Steps")
            graphHeight: 300

            axisY.units: qsTr("Steps")
            type: DataSource.Steps

            minY: 0
            maxY: 200
            valueConverter: function(value) {
                return value.toFixed(0);
            }
            onClicked: {
                updateGraph(day);
            }
        }
        Graph {
            id: graphIntensity
            graphTitle: qsTr("Intensity")
            graphHeight: 300

            axisY.units: "%"
            type: DataSource.Intensity

            minY: 0
            maxY: 100
            valueConverter: function(value) {
                return value.toFixed(0);
            }
            onClicked: {
                updateGraph(day);
            }
        }
        Graph {
            id: graphHRV
            graphTitle: qsTr("HRV")
            graphHeight: 300

            axisY.units: ""
            type: DataSource.HRV

            minY: 0
            maxY: 100

            onClicked: {
                updateGraph(day);
            }
        }

        Graph {
            id: graphBodyTemperature
            graphTitle: qsTr("Body Temperature")
            graphHeight: 300

            axisY.units: "°C"
            axisX.mask: "hh:mm"

            type: DataSource.BodyTemperature
            graphType: line

            minY: 20
            maxY: 40

            onClicked: {
                updateGraph(day);
            }
        }

    }

    function updateGraphs() {
        graphHeartrate.updateGraph(day);
        graphSteps.updateGraph(day);
        graphIntensity.updateGraph(day);
        graphHRV.updateGraph(day);
        graphBodyTemperature.updateGraph(day);
    }

    Component.onCompleted: {
        day = new Date();
        updateGraphs();
    }
}
