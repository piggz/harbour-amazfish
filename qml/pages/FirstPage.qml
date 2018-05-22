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
        target: DeviceInterface
        onConnectionStateChanged: {
            console.log(DeviceInterface.connectionState);
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
                    DeviceInterface.disconnect();
                }
            }
            MenuItem {
                text: qsTr("Connect to watch")
                onClicked: {
                    manualDisconnect = false;
                    DeviceInterface.connectToDevice(pairedAddress.value);
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
                        visible: DeviceInterface.connectionState == "connecting"
                        running: DeviceInterface.connectionState == "connecting"
                    }
                    Image {
                        source: "image://theme/icon-m-bluetooth-device"
                        visible: DeviceInterface.connectionState == "connected" || DeviceInterface.connectionState == "authenticated"
                    }
                }
                Item {
                    width: childrenRect.width
                    height: childrenRect.height
                    BusyIndicator {
                        size: BusyIndicatorSize.Medium
                        visible: DeviceInterface.connectionState == "connected"
                        running: DeviceInterface.connectionState == "connected"
                    }
                    Image {
                        source: "image://theme/icon-m-watch"
                        visible: DeviceInterface.connectionState == "authenticated"
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
                }
                //Spacer
                Item {
                    width: parent.width -  Theme.iconSizeMedium * 3
                    height: 1
                }

                IconButton {
                    icon.source: "image://theme/icon-m-refresh"
                    onClicked: {
                        DeviceInterface.hrmService().enableManualHRMeasurement(true);
                    }
                }
            }
            /*
            Row {
                spacing: Theme.paddingLarge
                IconButton {
                    id: btnSyncActivities
                    icon.source: "image://theme/icon-m-cloud-download"
                    width: Theme.iconSizeMedium
                    height: width
                    onClicked: {
                        DeviceInterface.miBandService().fetchActivityData();
                    }
                    enabled: DeviceInterface.connectionState == "authenticated"
                }
                Label {
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeMedium
                    height: Theme.iconSizeMedium
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Download activity data");
                }
            }
            */
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
        target: DeviceInterface
        onConnectionStateChanged: {
            if (DeviceInterface.connectionState == "authenticated") {

                DeviceInterface.miBandService().onBatteryInfoChanged.connect(batteryInfoChanged);
                DeviceInterface.miBandService().onStepsChanged.connect(stepsChanged);
                DeviceInterface.hrmService().heartRateChanged.connect(heartRateChanged);

                DeviceInterface.miBandService().requestGPSVersion();
                DeviceInterface.miBandService().requestBatteryInfo();
            }
        }
    }

    function batteryInfoChanged() {
        lblBattery.text = DeviceInterface.miBandService().batteryInfo
    }

    function stepsChanged() {
        lblSteps.text = DeviceInterface.miBandService().steps
    }

    function heartRateChanged() {
        lblHeartrate.text = DeviceInterface.hrmService().heartRate
    }


    Component.onCompleted: {
        if (profileName.value === "") {
            needsProfileSet = true;
            return;
        }

        if (pairedAddress.value !== "") {
            DeviceInterface.connectToDevice(pairedAddress.value);
        }
    }
}
