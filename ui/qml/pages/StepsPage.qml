import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Steps")

    property int stepCount: 0

    property var day: new Date()

    //        PullDownMenu {
    //            DownloadDataMenuItem {}
    //        }

    // Place our content in a Column.  The PageHeader is always placed at the top
    // of the page, followed by our content.
    Column {
        id: column
        width: page.width
        spacing: styler.themePaddingLarge

        LabelPL {
            id: lblStepsToday
            font.pixelSize: styler.themeFontSizeExtraLarge * 3
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: stepCount > 0 ? stepCount : graphStepSummary.lastValue
            horizontalAlignment: Text.AlignHCenter
        }

        RowLayout {
            spacing: styler.themePaddingLarge
            width: parent.width

            IconButtonPL {
                id: btnPrev
                iconName: "image://theme/icon-m-back"
                onClicked: {
                    day.setDate(day.getDate() - 1);
                    lblDay.text = day.toDateString();
                    updateGraphs();
                }
            }
            LabelPL {
                id: lblDay
                Layout.fillWidth: true
                text: day.toDateString()
                height: btnPrev.height
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            IconButtonPL {
                id: btnNext
                iconName: "image://theme/icon-m-forward"
                onClicked: {
                    day.setDate(day.getDate() + 1);
                    lblDay.text = day.toDateString();
                    updateGraphs();

                }
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
            maxY: 20000
            valueConverter: function(value) {
                return value.toFixed(0);
            }
            onClicked: {
                updateGraph(day);
            }
        }

        Connections {
            target: DaemonInterfaceInstance
            onConnectionStateChanged: {
                if (DaemonInterfaceInstance.connectionState === "authenticated") {
                    DaemonInterfaceInstance.refreshInformation();
                }
            }
            onInformationChanged: {
                if (infoKey === DaemonInterface.INFO_STEPS) {
                    stepCount = parseInt(infoValue, 10) || 0;
                }
            }
        }
    }

    function updateGraphs() {
        graphStepSummary.updateGraph(day);
    }

    Component.onCompleted: {
        updateGraphs();
        stepCount = parseInt(DaemonInterfaceInstance.information(DaemonInterface.INFO_STEPS), 10) || 0;
    }

    onPageStatusActive: {
        pushAttached(Qt.resolvedUrl("SleepPage.qml"))
    }
}
