import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Battery")

    property var day: new Date()

    Column {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        LabelPL {
            id: lblCurrentHeartrate
            font.pixelSize: styler.themeFontSizeExtraLarge * 3
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("%1 %").arg(_InfoBatteryPercent)
            horizontalAlignment: Text.AlignHCenter
        }

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
            id: graphBat
            graphTitle: qsTr("Battery")
            graphHeight: 500

            axisY.units: "%"
            type: DataSource.BatteryLog
            graphType: 1

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
        graphBat.updateGraph(day);
    }

    Component.onCompleted: {
        updateGraphs();
//        DaemonInterfaceInstance.requestManualHeartrate();
    }

}
