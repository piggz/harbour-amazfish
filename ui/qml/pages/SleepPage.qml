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

    function _formatHours(hours) {
        var offset = new Date().getTimezoneOffset()
        //: Format of sleep hours
        return new Date((hours * 60 + offset) * 60000).toLocaleTimeString(Qt.locale(), qsTr("h:mm"))
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // Tell SilicaFlickable the height of its content.
        contentHeight: column.height

        PullDownMenu {
            DownloadDataMenuItem {}
        }

        // Place our content in a Column.  The PageHeader is always placed at the top
        // of the page, followed by our content.
        Column {
            id: column
            x: Theme.horizontalPageMargin
            width: page.width - 2*Theme.horizontalPageMargin
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("Sleep")
            }

            Label {
                id: lblSleepLastnight
                font.pixelSize: Theme.fontSizeExtraLarge * 3
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                text: _formatHours(graphSleepSummary.lastValue)
                horizontalAlignment: Text.AlignHCenter
            }

            Label {
                id: lblDeepSleepLastnight
                font.pixelSize: Theme.fontSizeExtraLarge
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                text: qsTr("Deep %1").arg(_formatHours(graphSleepSummary.lastZ))
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
            Label {
                id: lblUpdate
                anchors.horizontalCenter: parent.horizontalCenter
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
                height: Theme.iconSizeMedium
                verticalAlignment: Text.AlignVCenter
                width: parent.width
                text: qsTr("Note: last nights sleep is updated at mid-day")
            }
        }
    }

    function updateGraphs() {
        graphSleepSummary.updateGraph(day);
    }

    onStatusChanged: {
        if (status === PageStatus.Active) {
            //            if (!pageStack._currentContainer.attachedContainer) {
            pageStack.pushAttached(Qt.resolvedUrl("AnalysisPage.qml"))
            //        }
        }
    }

    Component.onCompleted: {
        updateGraphs();
    }
}
