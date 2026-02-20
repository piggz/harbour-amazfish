import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Blood Oxygen")

    property alias day: nav.day

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
            text: qsTr("Download SPO2")
            onClicked: DaemonInterfaceInstance.fetchData(Amazfish.TYPE_SPO2);
        }
    }

    Column {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium

        LabelPL {
            id: lblStepsToday
            font.pixelSize: styler.themeFontSizeExtraLarge * 3
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: graphSpo2Normal.lastValue + "%"
            horizontalAlignment: Text.AlignHCenter
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

        Graph {
            id: graphSpo2Normal
            graphTitle: qsTr("Normal SPO2")
            graphHeight: 300

            axisX.mask: "MM/dd"
            axisY.units: qsTr("%")
            type: DataSource.Spo2Normal
            graphType: bar

            minY: 80
            maxY: 100

            valueConverter: function(value) {
                return value.toFixed(0);
            }
            onClicked: {
                updateGraph(day);
            }
        }

        Graph {
            id: graphSpo2Sleep
            graphTitle: qsTr("Sleep SPO2")
            graphHeight: 300

            axisX.mask: "MM/dd"
            axisY.units: qsTr("%")
            type: DataSource.Spo2Sleep
            graphType: bar

            minY: 80
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
        graphSpo2Normal.updateGraph(day);
        graphSpo2Sleep.updateGraph(day);
    }

    Component.onCompleted: {
        day = new Date();
        updateGraphs();
    }
}
