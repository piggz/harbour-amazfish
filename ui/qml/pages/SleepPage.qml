import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Sleep")

    property var day: new Date()

    function _formatHours(hours) {
        var offset = new Date().getTimezoneOffset()
        //: Format of sleep hours
        return new Date((hours * 60 + offset) * 60000).toLocaleTimeString(Qt.locale(), qsTr("h:mm"))
    }

    pageMenu: PageMenuPL {
        DownloadDataMenuItem{}
    }

    Column {
        id: column
        width: page.width
        spacing: styler.themePaddingLarge

        LabelPL {
            id: lblSleepLastnight
            font.pixelSize: styler.themeFontSizeExtraLarge * 3
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: _formatHours(graphSleepSummary.lastValue)
            horizontalAlignment: Text.AlignHCenter
        }

        LabelPL {
            id: lblDeepSleepLastnight
            font.pixelSize:styler.themeFontSizeExtraLarge
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Deep %1").arg(_formatHours(graphSleepSummary.lastZ))
            horizontalAlignment: Text.AlignHCenter
        }

        DateNavigation {
            text: day.toDateString();
            onBackward: {
                day.setDate(day.getDate() - 1);
                updateGraphs();
            }
            onForward: {
                day.setDate(day.getDate() + 1);
                updateGraphs();
            }
        }

        Graph {
            id: graphSleepSummary
            graphTitle: qsTr("Sleep Summary")
            graphHeight: 300

            //: Format for day on the sleep summary graph
            axisX.mask: qsTr("MM/dd")
            axisY.units: qsTr("Hours")
            type: DataSource.SleepSummary
            graphType: 2

            minY: 0
            maxY: 12
            valueConverter: function(value) {
                return value.toFixed(1);
            }
            onClicked: {
                updateGraph(day);
            }
        }

        LabelPL {
            id: lblUpdate
            anchors.horizontalCenter: parent.horizontalCenter
            verticalAlignment: Text.AlignVCenter
            width: parent.width
            text: qsTr("Note: last nights sleep is updated at mid-day")
        }
    }


    function updateGraphs() {
        graphSleepSummary.updateGraph(day);
    }

    onPageStatusActive: {
        pushAttached(Qt.resolvedUrl("HeartratePage.qml"))
    }

    Component.onCompleted: {
        updateGraphs();
    }
}
