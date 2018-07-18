import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

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
                    DeviceInterface.infoService().refreshInformation();
                    DeviceInterface.miBandService().requestGPSVersion();
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
                text: qsTr("Serial No: ") + DeviceInterface.infoService().serialNumber
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                text: qsTr("Hardware Rev: ")+ DeviceInterface.infoService().hardwareRevision
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                text: qsTr("Software Rev: ") + DeviceInterface.infoService().softwareRevision
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                text: qsTr("Connection State: ") + DeviceInterface.connectionState
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                text: qsTr("GPS Ver: ") + DeviceInterface.miBandService().gpsVersion
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Button {
                text: qsTr("Test Notification")
                onClicked: {
                    DeviceInterface.alertNotificationService().sendAlert("Somebody", "Title", "Hello from SailfishOS.  This is a long message sent over BLE!");
                }
            }
            Button {
                text: qsTr("Test Email")
                onClicked: {
                    DeviceInterface.alertNotificationService().sendAlert("someone@somewhere.com", "Donald Duck", "Hello, this is an email from Sailfish OS!");
                }
            }
            Button {
                text: qsTr("Test Call")
                onClicked: {
                    DeviceInterface.alertNotificationService().incomingCall("Somebody");
                }
            }
            Button {
                text: qqsTr("Fetch debug log")
                onClicked: {
                    DeviceInterface.miBandService().fetchLogs();
                }
            }
            Button {
                text: qsTr("Test Popup")
                onClicked: {
                    app.showMessage("This is a test notification");
                }
            }
        }
    }

    Component.onCompleted: {
        DeviceInterface.infoService().refreshInformation();
        DeviceInterface.miBandService().requestGPSVersion();
    }
}
