import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import uk.co.piggz.amazfish 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property date activityDate
    property date sportDate

    ConfigurationValue {
        id: pairedAddress
        key: "/uk/co/piggz/amazfish/pairedAddress"
        defaultValue: ""
    }

    ConfigurationValue {
        id: lastActivitySync
        key: "/uk/co/piggz/amazfish/device/lastactivitysyncmillis"
    }

    ConfigurationValue {
        id: lastSportSync
        key: "/uk/co/piggz/amazfish/device/lastsportsyncmillis"
    }

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

            Label {
                text: qsTr("Address: ") + pairedAddress.value
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                id: lblSerial
                text: qsTr("Serial No: ")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                id: lblHWRev
                text: qsTr("Hardware Rev: ")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                id: lblSWRev
                text: qsTr("Software Rev: ")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                text: qsTr("Connection State: ") + DaemonInterfaceInstance.connectionState
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                id: lblGPSVer
                text: qsTr("GPS Ver: ")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
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
                        lastActivitySync.value -= 3600000
                        activityDate = new Date(lastActivitySync.value);
                    }
                }
                Button {
                    id: btnAddActivitySync
                    width: parent.width * 0.16
                    text: "+"
                    onClicked: {
                        lastActivitySync.value += 3600000
                        activityDate = new Date(lastActivitySync.value);
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
                        lastSportSync.value -= 3600000
                        sportDate = new Date(lastSportSync.value);
                    }
                }
                Button {
                    id: btnAddSportSync
                    width: parent.width * 0.16
                    text: "+"
                    onClicked: {
                        lastSportSync.value += 3600000
                        sportDate = new Date(lastSportSync.value);
                    }
                }
            }

            Button {
                text: qsTr("Test Notification")
                onClicked: {
                    DaemonInterfaceInstance.sendAlert("Somebody", "Title", "Hello from SailfishOS.  This is a long message sent over BLE!");
                }
            }
            Button {
                text: qsTr("Test Email")
                onClicked: {
                    DaemonInterfaceInstance.sendAlert("someone@somewhere.com", "Donald Duck", "Hello, this is an email from Sailfish OS!");
                }
            }
            Button {
                text: qsTr("Test Call")
                onClicked: {
                    DaemonInterfaceInstance.incomingCall("Somebody");
                }
            }
            Button {
                text: qsTr("Fetch debug log")
                onClicked: {
                    DaemonInterfaceInstance.miBandService().fetchLogs();
                }
            }
            Button {
                text: qsTr("Test Popup")
                onClicked: {
                    app.showMessage("This is a test notification");
                }
            }
            Button {
                text: qsTr("Send Weather")
                onClicked: {
                    DaemonInterfaceInstance.triggerSendWeather();
                }
            }
            Button {
                text: qsTr("Update Calendar")
                onClicked: {
                    var dt = new Date();
                    dt.setHours(dt.getHours() + 4);

                    DaemonInterfaceInstance.updateCalendar();
                }
            }
        }
    }

    Component.onCompleted: {
        DaemonInterfaceInstance.refreshInformation();
        activityDate = new Date(lastActivitySync.value);
        sportDate = new Date(lastSportSync.value);

        //console.log(lastActivitySync.value, activityDate.value);
        //console.log(lastSportSync, sportDate);
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
            }
        }
    }
}
