import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Stress")

    property alias day: nav.day

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
            text: qsTr("Download Stress Data")
            onClicked: DaemonInterfaceInstance.fetchData(Amazfish.TYPE_STRESS);
        }
    }

    Column {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        LabelPL {
            id: lblStressNow
            font.pixelSize: styler.themeFontSizeExtraLarge * 3
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: graphStressAuto.lastValue + "%"
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
            id: graphStressAuto
            graphTitle: qsTr("Stress")
            graphHeight: 300

            axisX.mask: "hh:mm"
            axisY.units: qsTr("%")
            type: DataSource.StressAuto
            graphType: bar

            minY: 1
            maxY: 100

            colorMap: [
                {"limit": 39, "color": "blue"},
                {"limit": 59, "color": "green"},
                {"limit": 79, "color": "orange"},
            ]
            defaultColor: "red"

            valueConverter: function(value) {
                return value.toFixed(0);
            }
            onClicked: {
                updateGraph(day);
            }
        }


    }

    function updateGraphs() {
        graphStressAuto.updateGraph(day);
    }

    Component.onCompleted: {
        day = new Date();
        updateGraphs();
    }
}
