import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Steps")
    property string qmlUrl: "StepsPage.qml"

    property alias day: nav.day    

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
            text: graphStepSummary.lastValue.toLocaleString()
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
            id: graphStepSummary
            graphTitle: qsTr("Steps")
            graphHeight: 300

            axisX.mask: "MM/dd"
            axisY.units: qsTr("Steps")
            type: DataSource.StepSummary
            graphType: 2

            minY: 0
            maxY: (2 * AmazfishConfig.profileFitnessGoal > suggestedMaxY)
                  ? 2 * AmazfishConfig.profileFitnessGoal
                  : Math.ceil(suggestedMaxY/1000)*1000

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
        day = new Date();
        updateGraphs();
        _InfoSteps = parseInt(DaemonInterfaceInstance.information(Amazfish.INFO_STEPS), 10) || 0;
    }

    onPageStatusActive: {
        pushAttached(Qt.resolvedUrl(getNextPage()))
    }
}
