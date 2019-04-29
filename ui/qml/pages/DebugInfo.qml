import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import uk.co.piggz.amazfish 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    ConfigurationValue {
        id: pairedAddress
        key: "/uk/co/piggz/amazfish/pairedAddress"
        defaultValue: ""
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
                text: qsTr("Reboot watch")
                onClicked: {
                    DaemonInterfaceInstance.rebootWatch();
                }
            }
        }
    }

    Component.onCompleted: {
        DaemonInterfaceInstance.refreshInformation();
    }

    Connections {
        target: DaemonInterfaceInstance
        onInformationChanged: {
            switch (infoKey) {
            case AbstractDevice.INFO_SERIAL:
                lblSerial.text = qsTr("Serial No: ") + infoValue;
                break;
            case AbstractDevice.INFO_HWVER:
                lblHWRev.text = qsTr("Hardware Rev: ") + infoValue;
                break;
            case AbstractDevice.INFO_SWVER:
                lblSWRev.text = qsTr("Software Rev: ")+ infoValue;
                break;
            case AbstractDevice.INFO_GPSVER:
                lblGPSVer.text = qsTr("GPS Ver: ") +infoValue;
                break;
            }
        }
    }
}
