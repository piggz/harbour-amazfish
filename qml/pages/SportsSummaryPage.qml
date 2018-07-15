import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import "../components/"

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property var day: new Date()

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaListView {
        anchors.fill: parent
        model: SportsModel

        header: PageHeader {
            title: qsTr("Sports Activities")
        }

        VerticalScrollDecorator {}

        PullDownMenu {
            MenuItem {
                text: qsTr("Download Next Activity")
                onClicked: DeviceInterface.downloadSportsData();
                enabled: DeviceInterface.connectionState === "authenticated"
            }
        }

        delegate: ListItem {
            id: listItem
            contentHeight: distLabel.y + distLabel.height + Theme.paddingMedium

            Image
            {
                id: workoutImage
                anchors.top: parent.top
                anchors.topMargin: Theme.paddingMedium
                x: Theme.paddingMedium
                width: Theme.paddingMedium * 3
                height: Theme.paddingMedium * 3
                source: "../pics/icon-m-" + kindstring.toLowerCase() + ".png"
            }
            Label
            {
                id: nameLabel
                width: parent.width - dateLabel.width - 2*Theme.paddingLarge
                anchors.top: parent.top
                anchors.topMargin: Theme.paddingMedium
                anchors.left: workoutImage.right
                anchors.leftMargin: Theme.paddingMedium
                truncationMode: TruncationMode.Fade
                text: name
                color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor
            }
            Label
            {
                id: dateLabel
                anchors.top: parent.top
                anchors.topMargin: Theme.paddingMedium
                anchors.right: parent.right
                anchors.rightMargin: Theme.paddingSmall
                text: startdate
                color: listItem.highlighted ? Theme.highlightColor : Theme.primaryColor
            }
            Image {
                id: distangeImage
                anchors.top: nameLabel.bottom
                anchors.left: workoutImage.right
                anchors.leftMargin: Theme.paddingMedium
                source: "image://theme/icon-m-location"
                height: distLabel.height
                width: height
            }

            Label
            {
                id: distLabel
                anchors.top: distangeImage.top
                anchors.left: distangeImage.right
                anchors.leftMargin: Theme.paddingMedium
                color: listItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: baselatitude.toFixed(3) + "," + baselongitude.toFixed(3) + " " + basealtitude + "m"
                //text: (settings.measureSystem === 0) ? (stravaList.model[index]["distance"]/1000).toFixed(2) + "km" : JSTools.fncConvertDistanceToImperial(stravaList.model[index]["distance"]/1000).toFixed(2) + "mi"
            }
            Image {
                id: timeImage
                anchors.top: timeLabel.top
                anchors.right: timeLabel.left
                anchors.rightMargin: Theme.paddingSmall
                source: "image://theme/icon-m-clock"
                height: timeLabel.height
                width: height
            }
            Label
            {
                id: timeLabel
                anchors.top: nameLabel.bottom
                anchors.right: parent.right
                anchors.rightMargin: Theme.paddingMedium
                color: listItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: fncCovertSecondsToString((enddate - startdate) / 1000)
                width: listItem.width / 3
            }
            /*Image {
                id: elevationImage
                anchors.top: nameLabel.bottom
                anchors.right: elevationLabel.left
                anchors.rightMargin: Theme.paddingSmall
                source: "../img/elevation.png"
                height: elevationLabel.height
                width: height
            }

            Label
            {
                id: elevationLabel
                anchors.top: elevationImage.top
                anchors.right: parent.right
                anchors.rightMargin: Theme.paddingSmall
                color: listItem.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: stravaList.model[index]["total_elevation_gain"] + "m"
            }*/
            onClicked: {
                //var activityPage = pageStack.push(Qt.resolvedUrl("StravaActivityPage.qml"));
                //activityPage.loadActivity(stravaList.model[index]["id"]);
            }
        }
    }

    Component.onCompleted: {
        SportsModel.update();
    }

    function fncCovertSecondsToString(sec)
    {
        var iHours = Math.floor(sec / 3600);
        var iMinutes = Math.floor((sec - iHours * 3600) / 60);
        var iSeconds = Math.floor(sec - (iHours * 3600) - (iMinutes * 60));

        return (iHours > 0 ? iHours + "h " : "") + (iMinutes > 0 ? iMinutes + "m " : "") + iSeconds + "s";
    }

    Connections {
        target: DeviceInterface

        onOperationRunningChanged: {
            SportsModel.update();
        }
    }
}
