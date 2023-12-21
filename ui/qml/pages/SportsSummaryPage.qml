import QtQuick 2.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PageListPL {
    id: page
    title: qsTr("Sports Activities")
    model: SportsModel

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
            text: qsTr("Download Next Activity")
            onClicked: DaemonInterfaceInstance.downloadSportsData();
            enabled: DaemonInterfaceInstance.connectionState === "authenticated"
        }
    }

    delegate: ListItemPL {
        id: listItem
        contentHeight: styler.themeItemSizeSmall + (styler.themePaddingMedium * 2)

        IconPL
        {
            id: workoutImage
            anchors.top: parent.top
            anchors.topMargin: styler.themePaddingMedium
            x: styler.themePaddingMedium
            width: styler.themeItemSizeSmall
            height: width
            iconName: styler.customIconPrefix + "icon-m-" + kindstring.toLowerCase() + styler.customIconSuffix
        }
        LabelPL
        {
            id: nameLabel
            width: listItem
            .width - dateLabel.width - 2*styler.themePaddingLarge
            anchors.top: parent.top
            anchors.topMargin: styler.themePaddingMedium
            anchors.left: workoutImage.right
            anchors.leftMargin: styler.themePaddingMedium
            text: name
        }
        LabelPL
        {
            id: dateLabel
            anchors.top: parent.top
            anchors.topMargin: styler.themePaddingMedium
            anchors.right: parent.right
            anchors.rightMargin: styler.themePaddingSmall
            text: startdate
        }
        IconPL {
            id: distangeImage
            anchors.top: nameLabel.bottom
            anchors.left: workoutImage.right
            anchors.leftMargin: styler.themePaddingMedium
            iconName: styler.iconLocation
            height: distLabel.height
            width: height
        }

        LabelPL
        {
            id: distLabel
            anchors.top: distangeImage.top
            anchors.left: distangeImage.right
            anchors.leftMargin: styler.themePaddingMedium
            text: baselatitude.toFixed(3) + "," + baselongitude.toFixed(3) + " " + basealtitude + "m"
        }
        IconPL {
            id: timeImage
            anchors.top: timeLabel.top
            anchors.right: timeLabel.left
            iconName: styler.iconClock
            height: timeLabel.height
            width: height
        }
        LabelPL
        {
            id: timeLabel
            anchors.top: nameLabel.bottom
            anchors.right: parent.right
            anchors.rightMargin: styler.themePaddingMedium
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
            //var activityPage = app.pages.push(Qt.resolvedUrl("StravaActivityPage.qml"));
            //activityPage.loadActivity(stravaList.model[index]["id"]);
            var sportpage = app.pages.push(Qt.resolvedUrl("SportPage.qml"));
            sportpage.activitytitle = name;
            sportpage.date = startdate;
            sportpage.location = baselatitude.toFixed(3) + "," + baselongitude.toFixed(3) + " " + basealtitude + "m";
            sportpage.startdate = startdate;
            sportpage.duration = fncCovertSecondsToString((enddate - startdate) / 1000);
            sportpage.kindstring = kindstring;
            SportsMeta.update(id);
            sportpage.tcx = SportsModel.gpx(id);
            sportpage.update();
        }

        menu: ContextMenuPL {
            id: contextMenu
            ContextMenuItemPL {
                iconName: styler.iconDelete
                text: qsTr("Remove")
                onClicked: {
                    SportsModel.deleteRecord(id);
                }
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
        target: DaemonInterfaceInstance

        onOperationRunningChanged: {
            SportsModel.update();
        }
    }

    onPageStatusActive: {
        pushAttached(Qt.resolvedUrl("BatteryPage.qml"))
    }

}
