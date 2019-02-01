import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import "../components/"
import uk.co.piggz.amazfish 1.0

Page {
    id: page
    property int stepCount: 0

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property var day: new Date()

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // Tell SilicaFlickable the height of its content.
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                text: qsTr("Download Data")
                onClicked: DeviceInterfaceInstance.downloadActivityData();
                enabled: DeviceInterfaceInstance.connectionState === "authenticated"
            }
        }

        // Place our content in a Column.  The PageHeader is always placed at the top
        // of the page, followed by our content.
        Column {
            id: column
            x: Theme.horizontalPageMargin
            width: page.width - 2*Theme.horizontalPageMargin
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("Steps")
            }

            Label {
                id: lblStepsToday
                font.pixelSize: Theme.fontSizeExtraLarge * 3
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                text: stepCount > 0 ? stepCount : graphStepSummary.lastValue
                horizontalAlignment: Text.AlignHCenter
            }

            Row {
                spacing: Theme.paddingLarge
                width: parent.width

                IconButton {
                    id: btnPrev
                    icon.source: "image://theme/icon-m-back"
                    onClicked: {
                        day.setDate(day.getDate() - 1);
                        lblDay.text = day.toDateString();
                        updateGraphs();
                    }
                }
                Label {
                    id: lblDay
                    width: parent.width - btnPrev.width - btnNext.width - (2 * Theme.paddingLarge)
                    text: day.toDateString()
                    height: btnPrev.height
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                IconButton {
                    id: btnNext
                    icon.source: "image://theme/icon-m-forward"
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
        }
    }

    function updateGraphs() {
        graphStepSummary.updateGraph(day);
    }

    onStatusChanged: {
        if (status === PageStatus.Active) {
            //            if (!pageStack._currentContainer.attachedContainer) {
            pageStack.pushAttached(Qt.resolvedUrl("SleepPage.qml"))
            //        }
        }
    }

    Component.onCompleted: {
        updateGraphs();
        stepCount = DeviceInterfaceInstance.information(AbstractDevice.INFO_STEPS);
    }


    Connections {
        target: DeviceInterfaceInstance
        onConnectionStateChanged: {
            if (DeviceInterfaceInstance.connectionState === "authenticated") {
                DeviceInterfaceInstance.refreshInformation();
            }
        }
        onInformationChanged: {
            switch (infoKey) {
            case AbstractDevice.INFO_STEPS:
                stepCount = infoValue
                break;
            }
        }
    }
}
