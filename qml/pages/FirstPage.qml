import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property bool manualDisconnect: false
    property bool needsProfileSet: false

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

    ConfigurationValue {
        id: profileName
        key: "/uk/co/piggz/amazfish/profile/name"
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
                text: qsTr("Debug Info")
                onClicked: pageStack.push(Qt.resolvedUrl("DebugInfo.qml"))
            }
            MenuItem {
                text: qsTr("Pair with watch")
                onClicked: pageStack.push(Qt.resolvedUrl("PairPage.qml"))
            }
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("Settings-menu.qml"))
            }
            MenuItem {
                text: qsTr("Disconnect from watch")
                onClicked: {
                    manualDisconnect = true;
                    BipInterface.disconnect();
                }
            }
            MenuItem {
                text: qsTr("Connect to watch")
                onClicked: {
                    manualDisconnect = false;
                    BipInterface.connectToDevice(pairedAddress.value);
                }
            }
        }

        // Tell SilicaFlickable the height of its content.
        contentHeight: column.height

        /*
        Timer {
            id: tmrRefresh
            interval: 60000
            repeat: true
            running: true
            onTriggered: {
                if (BipInterface.connectionState == "disconnected" && manualDisconnect == false){
                    BipInterface.connectToDevice(pairedAddress.value);
                }
            }
        }*/

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
                        visible: BipInterface.connectionState == "connected" || BipInterface.connectionState == "authenticated"
                    }
                }
                Item {
                    width: childrenRect.width
                    height: childrenRect.height
                    BusyIndicator {
                        size: BusyIndicatorSize.Medium
                        visible: BipInterface.connectionState == "connected"
                        running: BipInterface.connectionState == "connected"
                    }
                    Image {
                        source: "image://theme/icon-m-watch"
                        visible: BipInterface.connectionState == "authenticated"
                    }
                }

            }
            Row {
                Image {
                    id: imgBattery
                    source: "image://theme/icon-m-battery"
                    width: Theme.iconSizeMedium
                    height: width
                }
                Label {
                    id: lblBattery
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeMedium
                    height: Theme.iconSizeMedium
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Row {
                Image {
                    id: imgSteps
                    source: "../pics/icon-m-steps.png"
                    width: Theme.iconSizeMedium
                    height: width
                }
                Label {
                    id: lblSteps
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeMedium
                    height: Theme.iconSizeMedium
                    verticalAlignment: Text.AlignVCenter
                }
            }

            //Heartrate
            Row {
                width: parent.width
                Image {
                    id: imgHeartrate
                    source: "../pics/icon-m-heartrate.png"
                    width: Theme.iconSizeMedium
                    height: width
                }
                Label {
                    id: lblHeartrate
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeMedium
                    height: Theme.iconSizeMedium
                    verticalAlignment: Text.AlignVCenter
                }
                //Spacer
                Item {
                    width: parent.width -  Theme.iconSizeMedium * 3
                    height: 1
                }

                IconButton {
                    icon.source: "image://theme/icon-m-refresh"
                    onClicked: {
                        BipInterface.hrmService().enableManualHRMeasurement(true);
                    }
                }
            }
        }
    }
    Timer {
        id: tmrStartup
        running: true
        repeat: false
        interval: 200
        onTriggered: {
            if (needsProfileSet) {
                pageStack.push(Qt.resolvedUrl("Settings-profile.qml"))
            }
        }
    }

    Connections {
        target: BipInterface
        onReadyChanged: {
            if (BipInterface.ready){
                BipInterface.infoService().refreshInformation();
                BipInterface.miBandService().requestGPSVersion();
                BipInterface.miBandService().requestBatteryInfo();

            }
        }
    }

    Connections {
        target: BipInterface.miBandService()

        onBatteryInfoChanged: {
            lblBattery.text = BipInterface.miBandService().batteryInfo
        }

        onStepsChanged: {
            lblSteps.text = BipInterface.miBandService().steps
        }
    }

    Connections {
        target: BipInterface.hrmService()

        onHeartRateChanged: {
            lblHeartrate.text = BipInterface.hrmService().heartRate
        }
    }

    Component.onCompleted: {
        if (profileName.value === "") {
            needsProfileSet = true;
            return;
        }

        if (pairedAddress.value !== "") {
            BipInterface.connectToDevice(pairedAddress.value);
        }
    }
}
