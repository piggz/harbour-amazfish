import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import "../components/"
import uk.co.piggz.amazfish 1.0

Page {
    id: page

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
                onClicked: DaemonInterfaceInstance.downloadActivityData();
                enabled: DaemonInterfaceInstance.connectionState === "authenticated"
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
                title: qsTr("Analysis")
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
    }

    function updateGraphs() {
        graphHeartrate.updateGraph(day);
        graphSteps.updateGraph(day);
        graphIntensity.updateGraph(day);
    }

    onStatusChanged: {
        if (status === PageStatus.Active) {
            //            if (!pageStack._currentContainer.attachedContainer) {
            pageStack.pushAttached(Qt.resolvedUrl("SportsSummaryPage.qml"))
            //        }
        }
    }

    Component.onCompleted: {
        updateGraphs();
    }
}
