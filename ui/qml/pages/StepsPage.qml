import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Steps")

    property var day: new Date()

    pageMenu: PageMenuPL {
        DownloadDataMenuItem{}
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
            text: _InfoSteps > 0 ? _InfoSteps : graphStepSummary.lastValue
            horizontalAlignment: Text.AlignHCenter
        }

        DateNavigation {
            id: nav
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
            id: graphStepSummary
            graphTitle: qsTr("Steps")
            graphHeight: 300

            axisX.mask: "MM/dd"
            axisY.units: "Steps"
            type: DataSource.StepSummary
            graphType: 2

            minY: 0
            maxY: 2 * AmazfishConfig.profileFitnessGoal
            valueConverter: function(value) {
                return value.toFixed(0);
            }
            onClicked: {
                updateGraph(day);
            }
        }
    }

    function updateGraphs() {
        graphStepSummary.updateGraph(day);
    }

    Component.onCompleted: {
        updateGraphs();
        _InfoSteps = parseInt(DaemonInterfaceInstance.information(Amazfish.INFO_STEPS), 10) || 0;
    }

    onPageStatusActive: {
        pushAttached(Qt.resolvedUrl("SleepPage.qml"))
    }
}
