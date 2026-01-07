import QtQuick 2.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"
import uk.co.piggz.amazfish 1.0
import "../tools/JSTools.js" as JSTools

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

    IconPL {
        id: sharedIconPosition
        iconName: styler.iconLocation
        visible: false
    }

    IconPL {
        id: sharedIconTime
        iconName: styler.iconClock
        visible: false
    }

    delegate: ListItemPL {
        id: listItem
        contentHeight: styler.themeItemSizeSmall + (styler.themePaddingMedium * 2)
        anchors.left: parent.left
        anchors.leftMargin: styler.themePaddingMedium

        Row {
            width: parent.width
            spacing: units.gu(2)
            Loader {
                id: workoutImage
                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingMedium
                width: styler.themeItemSizeSmall
                height: width
                sourceComponent: IconPL {
                    iconSource: styler.customIconPrefix + "icon-m-" + kindstring.toLowerCase() + styler.customIconSuffix
                    width: styler.themeItemSizeSmall
                    height: width
                }
            }
            Column {
                id: firstColumn
                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingMedium
                width: parent.width * 0.45
                spacing: units.gu(1)
                Row {
                    anchors.left: parent.left
                    spacing: units.gu(1)
                    IconPL {
                        id: timeImage
                        height: timeLabel.height
                        width: height
                        asynchronous: true
                        source: sharedIconTime.source
                    }
                    LabelPL {
                        id: nameLabel
                        text: kindstring + " ~ " + fncCovertSecondsToString((enddate-startdate)/1000)
                    }
                }

                Row {
                    anchors.left: parent.left
                    spacing: units.gu(1)
                    IconPL {
                        id: positionImage
                        height: units.gu(2)
                        width: height
                        asynchronous: true
                        source: sharedIconPosition.source
                    }
                    LabelPL {
                        id: positionLabel
                        text: "%1°; %2°; %3m".arg((Math.round(baselatitude * 1e3 ) / 1e3).toLocaleString()).arg((Math.round( baselongitude * 1e3 ) / 1e3).toLocaleString()).arg(basealtitude.toLocaleString())
                    }
                }
            }

            Column {
                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingMedium
                spacing: units.gu(1)
                LabelPL {
                    id: timesLabel
                    anchors.left: parent.left
                    text: Qt.formatDateTime(startdate, "yyyy/MM/dd hh:mm")
                }
                LabelPL {
                    id: timeLabel
                    anchors.left: parent.left
                    text: Qt.formatDateTime(enddate, "yyyy/MM/dd hh:mm")
                }
            }
        }

        onClicked: {
            //var activityPage = app.pages.push(Qt.resolvedUrl("StravaActivityPage.qml"));
            //activityPage.loadActivity(stravaList.model[index]["id"]);
            var sportpage = app.pages.push(Qt.resolvedUrl("SportPage.qml"));
            sportpage.activitytitle = kindstring + " - " + Qt.formatDateTime(startdate, "yyyy/MM/dd");
            sportpage.date = Qt.formatDateTime(startdate, "yyyy/MM/dd");
            sportpage.location = "%1 °; %2 °; %3 m".arg((Math.round( baselatitude * 1e3 ) / 1e3).toLocaleString()).arg((Math.round( baselongitude * 1e3 ) / 1e3).toLocaleString()).arg(basealtitude.toLocaleString());
            sportpage.starttime = Qt.formatDateTime(startdate, "hh:mm:ss");
            sportpage.duration = timeLabel.text
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
