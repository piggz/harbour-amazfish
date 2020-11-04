import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Debugging")

    property date activityDate
    property date sportDate

    pageMenu: PageMenuPL {
        PageMenuItemPL {
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
            text: qsTr("Serial No: ")
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblHWRev
            text: qsTr("Hardware Rev: ")
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblSWRev
            text: qsTr("Software Rev: ")
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblModel
            text: qsTr("Model: ")
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblFWRev
            text: qsTr("Firmware Rev: ")
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblManufacturer
            text: qsTr("Manufacturer: ")
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            text: qsTr("Connection State: ") + DaemonInterfaceInstance.connectionState
            color: styler.themeSecondaryHighlightColor
        }
        LabelPL {
            id: lblGPSVer
            text: qsTr("GPS Ver: ")
            color: styler.themeSecondaryHighlightColor
        }

        Row {
            width: parent.width
            LabelPL {
                id: lblActivitySync
                width: parent.width * 0.66
                text: qsTr("A:" ) + activityDate.toISOString()
            }
            ButtonPL {
                id: btnMinusActivitySync
                width: parent.width * 0.16
                text: "-"
                onClicked: {
                    AmazfishConfig.lastActivitySync -= 3600000
                    activityDate = new Date(AmazfishConfig.lastActivitySync);
                }
            }
            ButtonPL {
                id: btnAddActivitySync
                width: parent.width * 0.16
                text: "+"
                onClicked: {
                    AmazfishConfig.lastActivitySync += 3600000
                    activityDate = new Date(AmazfishConfig.lastActivitySync);
                }
            }
        }

        Row {
            width: parent.width

            LabelPL {
                id: lblSportSync
                width: parent.width * 0.66
                text: qsTr("S:" ) + sportDate.toISOString()
            }
            ButtonPL {
                id: btnMinusSportSync
                width: parent.width * 0.16
                text: "-"
                onClicked: {
                    AmazfishConfig.lastSportSync -= 3600000
                    sportDate = new Date(AmazfishConfig.lastSportSync);
                }
            }
            ButtonPL {
                id: btnAddSportSync
                width: parent.width * 0.16
                text: "+"
                onClicked: {
                    AmazfishConfig.lastSportSync += 3600000
                    sportDate = new Date(AmazfishConfig.lastSportSync);
                }
            }
        }

        SectionHeaderPL {
            text: qsTr("Function Tests")
        }

        ButtonPL {
            text: qsTr("Test Notification")
            visible: supportsFeature(DaemonInterface.FEATURE_ALERT)
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.sendAlert("Somebody", "Title", "Hello from SailfishOS.  This is a long message sent over BLE!");
            }
        }
        ButtonPL {
            text: qsTr("Test Email")
            visible: supportsFeature(DaemonInterface.FEATURE_ALERT)
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.sendAlert("someone-somewhere.com", "Donald Duck", "Hello, this is an email from Sailfish OS!X");
            }
        }
        ButtonPL {
            text: qsTr("Test Call")
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.incomingCall("Somebody");
            }
        }
        ButtonPL {
            text: qsTr("Fetch debug log")
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.miBandService().fetchLogs();
            }
        }
        ButtonPL {
            text: qsTr("Test Popup")
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                app.showMessage("This is a test notification");
            }
        }
        ButtonPL {
            text: qsTr("Send Weather")
            visible: supportsFeature(DaemonInterface.FEATURE_WEATHER)

            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.triggerSendWeather();
            }
        }
        ButtonPL {
            text: qsTr("Update Calendar")
            visible: supportsFeature(DaemonInterface.FEATURE_EVENT_REMINDER)
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
            visible: supportsFeature(DaemonInterface.FEATURE_MUSIC_CONTROL)
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width * 0.8
            onClicked: {
                DaemonInterfaceInstance.enableFeature(DaemonInterface.FEATURE_MUSIC_CONTROL);
            }
        }
        Connections {
            target: DaemonInterfaceInstance
            onInformationChanged: {
                switch (infoKey) {
                    case DaemonInterface.INFO_SERIAL:
                    lblSerial.text = qsTr("Serial No: ") + infoValue;
                    break;
                    case DaemonInterface.INFO_HWVER:
                    lblHWRev.text = qsTr("Hardware Rev: ") + infoValue;
                    break;
                    case DaemonInterface.INFO_SWVER:
                    lblSWRev.text = qsTr("Software Rev: ")+ infoValue;
                    break;
                    case DaemonInterface.INFO_GPSVER:
                    lblGPSVer.text = qsTr("GPS Ver: ") +infoValue;
                    break;
                    case DaemonInterface.INFO_MODEL:
                    lblModel.text = qsTr("Model: ") +infoValue;
                    break;
                    case DaemonInterface.INFO_MANUFACTURER:
                    lblManufacturer.text = qsTr("Manufacturer: ") +infoValue;
                    break;
                    case DaemonInterface.INFO_FW_REVISION:
                    lblFWRev.text = qsTr("Firmware Rev: ") +infoValue;
                    break;
                }
            }
        }

    }


Component.onCompleted: {
    DaemonInterfaceInstance.refreshInformation();
    activityDate = new Date(AmazfishConfig.lastActivitySync);
    sportDate = new Date(AmazfishConfig.lastSportSync);
}

}
