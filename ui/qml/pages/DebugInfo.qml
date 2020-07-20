import QtQuick 2.0
import Sailfish.Silica 1.0
import uk.co.piggz.amazfish 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property date activityDate
    property date sportDate

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // Tell SilicaFlickable the height of its content.
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                text: qsTr("Refresh")
                onClicked: {
                    DaemonInterfaceInstance.refreshInformation();
                    //DaemonInterfaceInstance.infoService().refreshInformation();
                    //DaemonInterfaceInstance.miBandService().requestGPSVersion();
                }
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
                title: qsTr("AmazFish")
            }

            SectionHeader {
                text: qsTr("Informations")
            }

            Label {
                text: qsTr("Address: ") + AmazfishConfig.pairedAddress
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeMedium
            }
            Label {
                id: lblSerial
                text: qsTr("Serial No: ")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeMedium
            }
            Label {
                id: lblHWRev
                text: qsTr("Hardware Rev: ")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeMedium
            }
            Label {
                id: lblSWRev
                text: qsTr("Software Rev: ")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeMedium
            }
            Label {
                id: lblModel
                text: qsTr("Model: ")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeMedium
            }
            Label {
                id: lblFWRev
                text: qsTr("Firmware Rev: ")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeMedium
            }
            Label {
                id: lblManufacturer
                text: qsTr("Manufacturer: ")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeMedium
            }
            Label {
                text: qsTr("Connection State: ") + DaemonInterfaceInstance.connectionState
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeMedium
            }
            Label {
                id: lblGPSVer
                text: qsTr("GPS Ver: ")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeMedium
            }

            Row {
                width: parent.width
                Label {
                    id: lblActivitySync
                    width: parent.width * 0.66
                    text: qsTr("A:" ) + activityDate.toISOString()
                }
                Button {
                    id: btnMinusActivitySync
                    width: parent.width * 0.16
                    text: "-"
                    onClicked: {
                        AmazfishConfig.lastActivitySync -= 3600000
                        activityDate = new Date(AmazfishConfig.lastActivitySync);
                    }
                }
                Button {
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

                Label {
                    id: lblSportSync
                    width: parent.width * 0.66
                    text: qsTr("S:" ) + sportDate.toISOString()
                }
                Button {
                    id: btnMinusSportSync
                    width: parent.width * 0.16
                    text: "-"
                    onClicked: {
                        AmazfishConfig.lastSportSync -= 3600000
                        sportDate = new Date(AmazfishConfig.lastSportSync);
                    }
                }
                Button {
                    id: btnAddSportSync
                    width: parent.width * 0.16
                    text: "+"
                    onClicked: {
                        AmazfishConfig.lastSportSync += 3600000
                        sportDate = new Date(AmazfishConfig.lastSportSync);
                    }
                }
            }

            SectionHeader {
                text: qsTr("function tests")
            }

            Button {
                text: qsTr("Test Notification")
                visible: supportsFeature(DaemonInterface.FEATURE_ALERT)
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width * 0.8
                onClicked: {
                    DaemonInterfaceInstance.sendAlert("Somebody", "Title", "Hello from SailfishOS.  This is a long message sent over BLE!");
                }
            }
            Button {
                text: qsTr("Test Email")
                visible: supportsFeature(DaemonInterface.FEATURE_ALERT)
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width * 0.8
                onClicked: {
                    DaemonInterfaceInstance.sendAlert("someone-somewhere.com", "Donald Duck", "Hello, this is an email from Sailfish OS!X");
                }
            }
            Button {
                text: qsTr("Test Call")
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width * 0.8
                onClicked: {
                    DaemonInterfaceInstance.incomingCall("Somebody");
                }
            }
            Button {
                text: qsTr("Fetch debug log")
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width * 0.8
                onClicked: {
                    DaemonInterfaceInstance.miBandService().fetchLogs();
                }
            }
            Button {
                text: qsTr("Test Popup")
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width * 0.8
                onClicked: {
                    app.showMessage("This is a test notification");
                }
            }
            Button {
                text: qsTr("Send Weather")
                visible: supportsFeature(DaemonInterface.FEATURE_WEATHER)

                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width * 0.8
                onClicked: {
                    DaemonInterfaceInstance.triggerSendWeather();
                }
            }
            Button {
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

            Button {
                text: qsTr("Music Control")
                visible: supportsFeature(DaemonInterface.FEATURE_MUSIC_CONTROL)
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width * 0.8
                onClicked: {
                    DaemonInterfaceInstance.enableFeature(DaemonInterface.FEATURE_MUSIC_CONTROL);
                }
            }
        }
    }

    Component.onCompleted: {
        DaemonInterfaceInstance.refreshInformation();
        activityDate = new Date(AmazfishConfig.lastActivitySync);
        sportDate = new Date(AmazfishConfig.lastSportSync);
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
