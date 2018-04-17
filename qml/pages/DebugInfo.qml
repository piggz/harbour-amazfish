import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property bool manualDisconnect: false

    Connections {
        target: BipInterface
        onConnectionStateChanged: {
            console.log(BipInterface.connectionState);
        }
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent        

        // Tell SilicaFlickable the height of its content.
        contentHeight: column.height

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
                text: "Address: " + pairedAddress.value
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraLarge
            }
            Label {
                text: "Serial No: " + BipInterface.infoService().serialNumber
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                text: "Hardware Rev: "+ BipInterface.infoService().hardwareRevision
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                text: "Software Rev: " + BipInterface.infoService().softwareRevision
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                text: "GPS Ver: " + BipInterface.miBandService().gpsVersion
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Button {
                text: "Test Notification"
                onClicked: {
                    BipInterface.alertNotificationService().sendAlert("Somebody", "Title", "Hello from SailfishOS.  This is a long message sent over BLE!");
                }
            }
            Button {
                text: "Test Call"
                onClicked: {
                    BipInterface.alertNotificationService().incomingCall("Somebody");
                }
            }
        }
    }

    Connections {
        target: BipInterface
        onReadyChanged: {
            if (BipInterface.ready){
                BipInterface.infoService().refreshInformation();
                BipInterface.miBandService().requestGPSVersion();
            }
        }
    }
}
