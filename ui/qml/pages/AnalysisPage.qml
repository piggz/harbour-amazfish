import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Analysis")
    property var day: new Date()

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
            text: day.toDateString();
            onBackward: {
                day.setDate(day.getDate() - 1);
                text = day.toDateString();
                updateGraphs();
            }
            onForward: {
                day.setDate(day.getDate() + 1);
                text = day.toDateString();
                updateGraphs();
            }
        }

        Graph {
            id: graphHeartrate
            graphTitle: qsTr("Heartrate")
            graphHeight: 300

            axisY.units: "BPM"
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

            axisY.units: "Steps"
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

    }

    function updateGraphs() {
        graphHeartrate.updateGraph(day);
        graphSteps.updateGraph(day);
        graphIntensity.updateGraph(day);
    }

    Component.onCompleted: {
        updateGraphs();
    }
}
