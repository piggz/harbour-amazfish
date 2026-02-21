import "../components/"
import "../components/platform"
import "../components/Translation.js" as T
import QtQuick 2.0
import QtQuick.Layouts 1.1

PageListPL {
    id: page

    function fncCovertSecondsToString(sec) {
        var iHours = Math.floor(sec / 3600);
        var iMinutes = Math.floor((sec - iHours * 3600) / 60);
        var iSeconds = Math.floor(sec - (iHours * 3600) - (iMinutes * 60));
        return (iHours > 0 ? iHours + "h " : "") + (iMinutes > 0 ? iMinutes + "m " : "") + iSeconds + "s";
    }


    title: qsTr("Sports Activities")
    model: SportsModel
    Component.onCompleted: {
        SportsModel.update();
    }
    onPageStatusActive: {
        pushAttached(Qt.resolvedUrl("BatteryPage.qml"));
    }

    IconPL {
        id: sharedIconTime

        iconName: styler.iconClock
        visible: false
    }

    Connections {
        target: DaemonInterfaceInstance
        onOperationRunningChanged: {
            SportsModel.update();
        }
    }

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
            text: qsTr("Download Next Activity")
            onClicked: DaemonInterfaceInstance.downloadSportsData()
            enabled: DaemonInterfaceInstance.connectionState === "authenticated"
        }

    }

    delegate: ListItemPL {
        id: listItem

        contentHeight: styler.themeItemSizeSmall + (styler.themePaddingMedium * 2)
        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width
        onClicked: {
            var sportpage = app.pages.push(Qt.resolvedUrl("SportPage.qml"), {
                "activitytitle": T.translateSportKind(kindstring) + " - " + Qt.formatDateTime(startdate, "yyyy/MM/dd"),
                "date": Qt.formatDateTime(startdate, "yyyy/MM/dd"),
                "location": [baselatitude, baselongitude, basealtitude],
                "starttime": Qt.formatDateTime(startdate, "hh:mm:ss"),
                "duration": durationLabel.text,
                "times": timesLabel.text,
                "kindstring": kindstring,
                "tcx": SportsModel.gpx(id)
            });
            SportsMeta.update(id);
            sportpage.update();
        }

        Row {
            id: listItemRow

            width: parent.width - (2 * styler.themePaddingLarge)
            spacing: styler.themePaddingLarge * 2
            anchors.left: parent.left
            anchors.leftMargin: styler.themePaddingLarge * 2
            anchors.right: parent.right
            anchors.rightMargin: styler.themePaddingLarge * 2

            Loader {
                id: workoutImage

                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingMedium
                width: styler.themeItemSizeSmall
                height: width

                sourceComponent: IconPL {
                    iconSource: styler.activityIconPrefix + "icon-m-" + kindstring.toLowerCase() + styler.customIconSuffix
                    width: styler.themeItemSizeSmall
                    height: width
                }

            }

            Column {
                id: leftColumn

                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingMedium
                width: (parent.width - workoutImage.width - (styler.themePaddingLarge * 4)) * 0.5
                spacing: styler.themePaddingSmall

                LabelPL {
                    id: nameLabel

                    text: T.translateSportKind(kindstring)
                }

                LabelPL {
                    id: dateLabel

                    text: Qt.formatDate(startdate, "ddd") + " " + startdate.toLocaleDateString(Qt.locale(), Locale.ShortFormat)
                }

            }

            Column {
                id: rightColumn

                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingMedium
                width: leftColumn.width
                spacing: styler.themePaddingSmall

                Row {
                    anchors.right: parent.right
                    spacing: styler.themePaddingSmall

                    IconPL {
                        id: durationImage

                        height: durationLabel.height
                        width: height
                        asynchronous: true
                        source: sharedIconTime.source
                    }

                    LabelPL {
                        id: durationLabel

                        text: fncCovertSecondsToString((enddate - startdate) / 1000)
                        horizontalAlignment: Text.AlignRight
                    }

                }

                LabelPL {
                    id: timesLabel

                    anchors.right: parent.right
                    text: startdate.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + " ⟶ " + enddate.toLocaleTimeString(Qt.locale(), Locale.ShortFormat)
                    horizontalAlignment: Text.AlignRight
                }

            }

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

}
