import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Debugging")

    property alias activityDate: activityDateNav.day
    property alias sportDate: sportDateNav.day

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconRefresh !== undefined ? styler.iconRefresh : ""
            text: qsTr("Refresh")
            onClicked: {
                DaemonInterfaceInstance.refreshInformation();
            }
        }
    }
    // Place our content in a Column.  The PageHeader is always placed at the top
    // of the page, followed by our content.
    Column {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        SectionHeaderPL {
            text: qsTr("Information")
        }

        LabelPL {
            text: qsTr("Address: ") + AmazfishConfig.pairedAddress
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblSerial
            text: qsTr("Serial No: ") + DaemonInterfaceInstance.information(Amazfish.INFO_SERIAL)
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblHWRev
            text: qsTr("Hardware Rev: ") + DaemonInterfaceInstance.information(Amazfish.INFO_HWVER)
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblSWRev
            text: qsTr("Software Rev: ") + DaemonInterfaceInstance.information(Amazfish.INFO_SWVER)
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblModel
            text: qsTr("Model: ") + DaemonInterfaceInstance.information(Amazfish.INFO_MODEL)
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblFWRev
            text: qsTr("Firmware Rev: ") + DaemonInterfaceInstance.information(Amazfish.INFO_FW_REVISION)
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblManufacturer
            text: qsTr("Manufacturer: ") + DaemonInterfaceInstance.information(Amazfish.INFO_MANUFACTURER)
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            text: qsTr("Connection State: ") + DaemonInterfaceInstance.connectionState + " (" +DaemonInterfaceInstance.connectionStateChangedCount+")"
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblGPSVer
            text: qsTr("GPS Ver: ") + DaemonInterfaceInstance.information(Amazfish.INFO_GPSVER)
            color: styler.themeSecondaryHighlightColor
        }

        Row {
            width: parent.width
            spacing: styler.themePaddingLarge

            LabelPL {
                id: lblActivitySync
                height: parent.height
                text: "A: "
            }

            DateNavigation {
                id: activityDateNav
                width: parent.width / 2
                onBackward: {
                    var d = new Date(activityDate);
                    d.setDate(day.getDate() - 1);
                    day = d;
                }
                onForward: {
                    var d = new Date(activityDate);
                    d.setDate(day.getDate() + 1);
                    day = d;
                }
                onDayChanged: {
                    activityDate = day;
                    AmazfishConfig.lastActivitySync = activityDate.getTime();
                }
            }

            LabelPL {
                id: lblActivityTime
                text: activityDate.toTimeString()
                height: parent.height
            }

            ButtonPL {
                id: btnMinusActivitySync
                width: height
                text: "-"
                onClicked: {
                    activityDate = new Date(activityDate.getTime() - (1*60*60*1000));;
                    AmazfishConfig.lastActivitySync = activityDate.getTime();
                }
            }
            ButtonPL {
                id: btnAddActivitySync
                width: height

                text: "+"
                onClicked: {
                    activityDate = new Date(activityDate.getTime() + (1*60*60*1000));;
                    AmazfishConfig.lastActivitySync = activityDate.getTime();
                }
            }
        }

        Row {
            width: parent.width
            spacing: styler.themePaddingLarge

            LabelPL {
                id: lblSportSync
                height: parent.height
                text: "S: "
            }

            DateNavigation {
                id: sportDateNav
                width: parent.width / 2
                onBackward: {
                    var d = new Date(sportDate);
                    d.setDate(day.getDate() - 1);
                    day = d;
                }
                onForward: {
                    var d = new Date(sportDate);
                    d.setDate(day.getDate() + 1);
                    day = d;
                }
                onDayChanged: {
                    sportDate = day;
                    AmazfishConfig.lastSportSync = sportDate.getTime();
                }
            }

            LabelPL {
                id: lblSportTime
                text: sportDate.toTimeString()
                height: parent.height
            }

            ButtonPL {
                id: btnMinusSportSync
                width: height
                text: "-"
                onClicked: {
                    sportDate = new Date(sportDate.getTime() - (1*60*60*1000));;
                    AmazfishConfig.lastSportSync = sportDate.getTime();
                }
            }
            ButtonPL {
                id: btnAddSportSync
                width: height
                text: "+"
                onClicked: {
                    sportDate = new Date(sportDate.getTime() + (1*60*60*1000));;
                    AmazfishConfig.lastSportSync = sportDate.getTime();
                }
            }
        }

        SectionHeaderPL {
            text: qsTr("Function Tests")
        }

        ButtonPL {
            text: qsTr("Test Notification")
            visible: supportsFeature(Amazfish.FEATURE_ALERT)
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.sendAlert(qsTr("Somebody"), qsTr("Title"), qsTr("Hello from Sailfish OS. This is a long message sent over BLE!"));
            }
        }
        ButtonPL {
            text: qsTr("Test Email")
            visible: supportsFeature(Amazfish.FEATURE_ALERT)
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.sendAlert("someone-somewhere.com", qsTr("Donald Duck"), qsTr("Hello, this is an email from Sailfish OS!"));
            }
        }
        ButtonPL {
            text: qsTr("Test Call")
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.incomingCall(qsTr("Somebody"));
            }
        }
        ButtonPL {
            property int immediateAlertLevel: 2
            text: qsTr("Test Immediate Alert Service")
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.immediateAlert(immediateAlertLevel);
                immediateAlertLevel = (immediateAlertLevel + 1) % 2;
            }
        }
        ButtonPL {
            text: qsTr("Fetch debug log")
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.fetchLogs();
            }
        }

        Row {
            width: parent.width * 0.8
            anchors.horizontalCenter: parent.horizontalCenter

            TextFieldPL {
                label: "Data Type"
                anchors.leftMargin: 0
                id: txtDataType
                width: parent.width * 0.4
            }
            ButtonPL {
                text: qsTr("Fetch Data")
                width: parent.width * 0.6
                onClicked: {
                    DaemonInterfaceInstance.fetchData(parseInt(txtDataType.text));
                }
            }
        }

        ButtonPL {
            text: qsTr("Request Screenshot")
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            visible: supportsFeature(Amazfish.FEATURE_SCREENSHOT)
            onClicked: {
                DaemonInterfaceInstance.requestScreenshot();
            }
        }
        ButtonPL {
            text: qsTr("Test Popup")
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                app.showMessage(qsTr("This is a test notification"));
            }
        }
        ButtonPL {
            text: qsTr("Send Weather")
            visible: supportsFeature(Amazfish.FEATURE_WEATHER)

            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.triggerSendWeather();
            }
        }
        ButtonPL {
            text: qsTr("Update Calendar")
            visible: supportsFeature(Amazfish.FEATURE_EVENT_REMINDER)
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                var dt = new Date();
                dt.setHours(dt.getHours() + 4);

                DaemonInterfaceInstance.updateCalendar();
            }
        }

        ButtonPL {
            text: qsTr("Music Control")
            visible: supportsFeature(Amazfish.FEATURE_MUSIC_CONTROL)
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.enableFeature(Amazfish.FEATURE_MUSIC_CONTROL);
            }
        }
        Connections {
            target: DaemonInterfaceInstance
            onInformationChanged: {
                switch (infoKey) {
                    case Amazfish.INFO_SERIAL:
                    lblSerial.text = qsTr("Serial No: ") + infoValue;
                    break;
                    case Amazfish.INFO_HWVER:
                    lblHWRev.text = qsTr("Hardware Rev: ") + infoValue;
                    break;
                    case Amazfish.INFO_SWVER:
                    lblSWRev.text = qsTr("Software Rev: ")+ infoValue;
                    break;
                    case Amazfish.INFO_GPSVER:
                    lblGPSVer.text = qsTr("GPS Ver: ") +infoValue;
                    break;
                    case Amazfish.INFO_MODEL:
                    lblModel.text = qsTr("Model: ") +infoValue;
                    break;
                    case Amazfish.INFO_MANUFACTURER:
                    lblManufacturer.text = qsTr("Manufacturer: ") +infoValue;
                    break;
                    case Amazfish.INFO_FW_REVISION:
                    lblFWRev.text = qsTr("Firmware Rev: ") +infoValue;
                    break;
                }
            }
        }

    }


Component.onCompleted: {
    DaemonInterfaceInstance.refreshInformation();
    activityDate = new Date(AmazfishConfig.lastActivitySync);
    activityDateNav.day = activityDate;
    sportDate = new Date(AmazfishConfig.lastSportSync);
    sportDateNav.day = sportDate;
}

}
