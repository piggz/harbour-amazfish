import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import org.SfietKonstantin.weatherfish 1.0
import uk.co.piggz.amazfish 1.0

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
        target: DeviceInterfaceInstance
        onConnectionStateChanged: {
            console.log(DeviceInterfaceInstance.connectionState);
        }
    }
    
    onStatusChanged: {
        if (status === PageStatus.Active) {
            //            if (!pageStack._currentContainer.attachedContainer) {
            pageStack.pushAttached(Qt.resolvedUrl("SummaryPage.qml"))
            //        }
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
                text: qsTr("Download File")
                onClicked: pageStack.push(Qt.resolvedUrl("BipFirmwarePage.qml"))
            }
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("Settings-menu.qml"))
            }
            MenuItem {
                text: qsTr("Disconnect from watch")
                onClicked: {
                    manualDisconnect = true;
                    DeviceInterfaceInstance.disconnect();
                }
            }
            MenuItem {
                text: qsTr("Connect to watch")
                onClicked: {
                    manualDisconnect = false;
                    DeviceInterfaceInstance.connectToDevice(pairedAddress.value);
                }
            }
        }

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
                        visible: DeviceInterfaceInstance.connectionState === "connecting"
                        running: DeviceInterfaceInstance.connectionState === "connecting"
                    }
                    Image {
                        source: "image://theme/icon-m-bluetooth-device"
                        visible: DeviceInterfaceInstance.connectionState === "connected" || DeviceInterfaceInstance.connectionState === "authenticated"
                    }
                }
                Item {
                    width: childrenRect.width
                    height: childrenRect.height
                    BusyIndicator {
                        size: BusyIndicatorSize.Medium
                        visible: DeviceInterfaceInstance.connectionState === "connected"
                        running: DeviceInterfaceInstance.connectionState === "connected"
                    }
                    Image {
                        source: "image://theme/icon-m-watch"
                        visible: DeviceInterfaceInstance.connectionState === "authenticated"
                    }
                }

            }
            Row {
                spacing: Theme.paddingLarge
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
                spacing: Theme.paddingLarge
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
                spacing: Theme.paddingLarge
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
                    width: parent.width - imgHeartrate.width - btnHR.width - 2* Theme.paddingLarge
                }

                IconButton {
                    id: btnHR
                    icon.source: "image://theme/icon-m-refresh"
                    onClicked: {
                        DeviceInterfaceInstance.requestManualHeartrate();
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
        target: DeviceInterfaceInstance
        onConnectionStateChanged: {
            if (DeviceInterfaceInstance.connectionState === "authenticated") {
                DeviceInterfaceInstance.refreshInformation();
            }
        }
        onInformationChanged: {
            console.log("Information changed", infoKey, infoValue);

            switch (infoKey) {
                case AbstractDevice.INFO_BATTERY:
                    lblBattery.text = infoValue
               	 break;
                case AbstractDevice.INFO_STEPS:
                    lblSteps.text = infoValue
                    break;
                case AbstractDevice.INFO_HEARTRATE:
                    lblHeartrate.text = infoValue
                    break;
            }
        }
    }

    Component.onCompleted: {
        if (profileName.value === "") {
            needsProfileSet = true;
            return;
        }

        if (pairedAddress.value !== "") {
            DeviceInterfaceInstance.connectToDevice(pairedAddress.value);
        }
    }
}
