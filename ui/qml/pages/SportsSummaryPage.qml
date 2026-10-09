import "../components/"
import "../components/platform"
import "../components/Translation.js" as T
import QtQuick 2.0
import QtQuick.Layouts 1.1
import uk.co.piggz.amazfish 1.0

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
            onClicked: DaemonInterfaceInstance.fetchData(Amazfish.TYPE_GPS_TRACK)
            enabled: DaemonInterfaceInstance.connectionState === "authenticated"
        }

    }

    delegate: ListItemPL {
        id: listItem

        // grouping: first/last activity of a month (the model is sorted by date)
        readonly property string monthKey: SportsModel.monthKeyAt(index)
        readonly property bool firstOfMonth: SportsModel.monthKeyAt(index - 1) !== monthKey
        readonly property bool lastOfMonth: SportsModel.monthKeyAt(index + 1) !== monthKey
        readonly property real rowHeight: Math.max(styler.themeItemSizeSmall, textColumn.height)
                                          + 2 * styler.themePaddingMedium
        readonly property real headerHeight: firstOfMonth ? monthLabel.height + 2 * styler.themePaddingMedium : 0

        contentHeight: headerHeight + rowHeight + (lastOfMonth ? styler.themePaddingLarge : 0)
        onClicked: {
            var sportpage = app.pages.push(Qt.resolvedUrl("SportPage.qml"), {
                "activityId": model.id,
                "activitytitle": T.translateSportKind(kindstring) + " - " + Qt.formatDateTime(startdate, "yyyy/MM/dd"),
                "date": Qt.formatDateTime(startdate, "yyyy/MM/dd"),
                "location": [baselatitude, baselongitude, basealtitude],
                "starttime": Qt.formatDateTime(startdate, "hh:mm:ss"),
                "duration": durationLabel.text,
                "times": timesText,
                "kindstring": kindstring,
                "tcx": SportsModel.gpx(id),
                "rawGpx": SportsModel.rawGpx(id)
            });
            SportsMeta.update(id);
            sportpage.update();
        }

        // pressed feedback where the platform's list item provides it (Silica, Kirigami, QtControls)
        readonly property bool showPressed: listItem.highlighted === true
        readonly property string timesText: startdate.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + " - "
                                            + enddate.toLocaleTimeString(Qt.locale(), Locale.ShortFormat)

        // Month header above the first activity of each month
        LabelPL {
            id: monthLabel
            visible: listItem.firstOfMonth
            x: styler.themeHorizontalPageMargin + styler.themePaddingSmall
            y: styler.themePaddingMedium
            text: startdate.toLocaleDateString(Qt.locale(), "MMMM yyyy").toUpperCase()
            color: styler.themeSecondaryHighlightColor
            font.pixelSize: styler.themeFontSizeExtraSmall
            font.bold: true
            font.letterSpacing: 1
        }

        // Each row is a slice of the month card; only the first and last slice have round corners.
        Item {
            id: slice
            x: styler.themeHorizontalPageMargin
            y: listItem.headerHeight
            width: parent.width - 2 * x
            height: listItem.rowHeight
            clip: true

            GlassPanel {
                id: groupGlass
                width: parent.width
                y: listItem.firstOfMonth ? 0 : -radius
                height: slice.height + (listItem.firstOfMonth ? 0 : radius) + (listItem.lastOfMonth ? 0 : radius)
            }

            Rectangle {
                visible: !listItem.firstOfMonth
                x: styler.themePaddingLarge
                width: parent.width - 2 * x
                height: 1
                color: styler.surfaceDividerColor
            }
        }

        Item {
            x: slice.x + styler.themePaddingLarge
            y: slice.y
            width: slice.width - 2 * styler.themePaddingLarge
            height: slice.height

            Rectangle {
                anchors.centerIn: workoutImage
                width: styler.themeIconSizeMedium * 1.15
                height: width
                radius: width / 2
                color: styler.surfaceChipColor
            }

            Loader {
                id: workoutImage
                anchors.verticalCenter: parent.verticalCenter
                x: styler.themeIconSizeMedium * 0.075
                width: styler.themeIconSizeMedium
                height: width
                sourceComponent: IconPL {
                    iconSource: styler.activityIconPrefix + "icon-m-" + kindstring.toLowerCase() + styler.customIconSuffix
                    width: workoutImage.width
                    height: width
                    opacity: listItem.showPressed ? styler.surfacePressedOpacity : 1.0
                }
            }

            // Kind and duration in the first line, date and time in the second line.
            Column {
                id: textColumn
                anchors.left: workoutImage.right
                anchors.leftMargin: styler.themePaddingLarge
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter

                Item {
                    width: parent.width
                    height: Math.max(nameLabel.height, durationLabel.height)

                    LabelPL {
                        id: nameLabel
                        anchors.left: parent.left
                        anchors.right: durationLabel.left
                        anchors.rightMargin: styler.themePaddingMedium
                        text: T.translateSportKind(kindstring)
                        color: listItem.showPressed ? styler.themeHighlightColor : styler.themePrimaryColor
                        font.pixelSize: styler.themeFontSizeMedium
                        // Long names shrink to the size of the duration before they are faded out.
                        fontSizeMode: Text.HorizontalFit
                        minimumPixelSize: styler.themeFontSizeSmall
                        truncMode: truncModes.fade
                    }

                    LabelPL {
                        id: durationLabel
                        anchors.right: parent.right
                        anchors.baseline: nameLabel.baseline
                        text: fncCovertSecondsToString((enddate - startdate) / 1000)
                        color: listItem.showPressed ? styler.themeHighlightColor : styler.themePrimaryColor
                        font.pixelSize: styler.themeFontSizeSmall
                        horizontalAlignment: Text.AlignRight
                    }
                }

                LabelPL {
                    id: dateLabel
                    width: parent.width
                    text: Qt.formatDate(startdate, "ddd") + " "
                          + startdate.toLocaleDateString(Qt.locale(), Locale.ShortFormat) + " - " + listItem.timesText
                    color: listItem.showPressed ? styler.themeSecondaryHighlightColor : styler.themeSecondaryColor
                    font.pixelSize: styler.themeFontSizeExtraSmall
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
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
