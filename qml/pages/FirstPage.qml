import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property bool manualDisconnect: false

    ConfigurationValue {
        id: pairedAddress
        key: "/uk/co/piggz/amazfish/pairedAddress"
        defaultValue: ""
    }

    ConfigurationValue {
        id: pairedName
        key: "/uk/co/piggz/amazfish/pairedName"
        defaultValue: ""
    }

    Connections {
        target: BipInterface
        onConnectionStateChanged: {
            console.log(BipInterface.connectionState);
        }
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
        PullDownMenu {
            MenuItem {
                text: qsTr("Pair with watch")
                onClicked: pageStack.push(Qt.resolvedUrl("SecondPage.qml"))
            }
            MenuItem {
                text: qsTr("Connect to watch")
                onClicked: {
                    manualDisconnect = false;
                    BipInterface.connectToDevice(pairedAddress.value);
                }
            }
            MenuItem {
                text: qsTr("Disconnect from watch")
                onClicked: {
                    manualDisconnect = true;
                    BipInterface.disconnect();
                }
            }
        }

        // Tell SilicaFlickable the height of its content.
        contentHeight: column.height

        Timer {
            id: tmrRefresh
            interval: 5000
            repeat: true
            running: true
            onTriggered: {
                if (BipInterface.connectionState == "disconnected" && manualDisconnect == false){
                    BipInterface.connectToDevice(pairedAddress.value);
                }

                if (BipInterface.ready){
                    BipInterface.infoService().refreshInformation();
                    BipInterface.miBandService().requestGPSVersion();
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
            Row {
                spacing: Theme.paddingLarge

                Label {
                    text: pairedName.value
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeExtraLarge
                }
                Item {
                    width: childrenRect.width
                    height: childrenRect.height
                    BusyIndicator {
                        size: BusyIndicatorSize.Medium
                        visible: BipInterface.connectionState == "connecting"
                        running: BipInterface.connectionState == "connecting"
                    }
                    Image {
                        source: "image://theme/icon-m-bluetooth-device"
                        visible: BipInterface.connectionState == "connected"
                    }
                }
            }
            Label {
                text: pairedAddress.value
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraLarge
            }
            Label {
                text: BipInterface.infoService().serialNumber
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                text: BipInterface.infoService().hardwareRevision
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                text: BipInterface.infoService().softwareRevision
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
            Label {
                text: "GPS:" + BipInterface.miBandService().gpsVersion
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
            }
        }
    }

    Component.onCompleted: {
        if (pairedAddress.value !== "") {
            BipInterface.connectToDevice(pairedAddress.value);
        }
    }
}
