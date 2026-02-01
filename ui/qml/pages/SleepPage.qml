import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Sleep")
    property string qmlUrl: "SleepPage.qml"

    property alias day: nav.day

    pageMenu: PageMenuPL {
        DownloadDataMenuItem{}
    }

    Column {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingMedium

        LabelPL {
            id: lblSleepLastnight
            font.pixelSize: styler.themeFontSizeExtraLarge * 2
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Total %1").arg(decimalToHourMin(graphSleepSummary.lastY + graphSleepSummary.lastZ))
            horizontalAlignment: Text.AlignHCenter
        }

        LabelPL {
            id: lblLightSleepLastnight
            font.pixelSize: styler.themeFontSizeExtraLarge
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Light %1").arg(decimalToHourMin(graphSleepSummary.lastY))
            horizontalAlignment: Text.AlignHCenter
        }

        LabelPL {
            id: lblDeepSleepLastnight
            font.pixelSize:styler.themeFontSizeExtraLarge
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Deep %1").arg(decimalToHourMin(graphSleepSummary.lastZ))
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
    }

    function updateGraphs() {
        graphSleepSummary.updateGraph(day);
    }

    function decimalToHourMin(decTime) {
        var totalMinutes = Math.round(decTime * 60);
        var hours = Math.floor(totalMinutes / 60);
        var minutes = (totalMinutes % 60);
        if (minutes < 10) {
            return hours + ":0" + minutes;
        }

        return hours + ":" + minutes;
    }

    onPageStatusActive: {
        pushAttached(Qt.resolvedUrl(getNextPage()))
    }

    Component.onCompleted: {
        day = new Date();
        updateGraphs();
    }
}
