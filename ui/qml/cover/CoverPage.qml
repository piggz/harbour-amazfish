import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import uk.co.piggz.amazfish 1.0

CoverBackground {
    property int stepCount: 0

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

    Column {
        anchors.fill: parent
        spacing: Theme.paddingLarge
        anchors.margins: Theme.paddingMedium

        Label {
            text: pairedName.value
            anchors.horizontalCenter: parent.horizontalCenter
            color: Theme.secondaryHighlightColor
            font.pixelSize: Theme.fontSizeLarge
        }

        Row {
            Item {
                width: childrenRect.width
                height: childrenRect.height

                BusyIndicator {
                    size: BusyIndicatorSize.Medium
                    visible: DaemonInterfaceInstance.connectionState === "connecting"
                    running: DaemonInterfaceInstance.connectionState === "connecting"
                }
                Image {
                    source: "image://theme/icon-m-bluetooth-device"
                    visible: DaemonInterfaceInstance.connectionState === "connected" || DaemonInterfaceInstance.connectionState === "authenticated"
                }
            }
            Item {
                width: childrenRect.width
                height: childrenRect.height

                BusyIndicator {
                    size: BusyIndicatorSize.Medium
                    visible: DaemonInterfaceInstance.connectionState === "connected"
                    running: DaemonInterfaceInstance.connectionState === "connected"
                }
                Image {
                    source: "image://theme/icon-m-watch"
                    visible: DaemonInterfaceInstance.connectionState === "authenticated"
                }
            }
            Image {
                id: imgBattery
                source: "image://theme/icon-m-battery"
                width: Theme.iconSizeMedium
                height: width
            }
            Label {
                id: lblBattery
                color: Theme.secondaryHighlightColor
                height: Theme.iconSizeMedium
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: Theme.fontSizeLarge
            }
        }

        //Steps
        Row {
            spacing: Theme.paddingLarge
            width: childrenRect.width
            anchors.horizontalCenter: parent.horizontalCenter

            Image {
                id: imgSteps
                source: "../pics/icon-m-steps.png"
                height: Theme.iconSizeMedium
                width: height
            }
            Label {
                id: lblSteps
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
                height: Theme.iconSizeMedium
                verticalAlignment: Text.AlignVCenter
                width: height
                text: stepCount
            }
        }

        //Heartrate
        Row {
            spacing: Theme.paddingLarge
            width: childrenRect.width
            anchors.horizontalCenter: parent.horizontalCenter
            Image {
                id: imgHeartrate
                source: "../pics/icon-m-heartrate.png"
                height: Theme.iconSizeMedium
                width: height
            }
            Label {
                id: lblHeartrate
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
                height: Theme.iconSizeMedium
                verticalAlignment: Text.AlignVCenter
                width: height
            }
        }

    }

    CoverActionList {
        id: coverAction

        CoverAction {
            iconSource: "image://theme/icon-m-refresh";
            onTriggered: {
                DaemonInterfaceInstance.requestManualHeartrate();
            }
        }

        CoverAction {
            iconSource: "image://theme/icon-m-cloud-download";
            onTriggered: {
                DaemonInterfaceInstance.miBandService().fetchActivityData();
            }
        }
    }

    Connections {
        target: DaemonInterfaceInstance
        onConnectionStateChanged: {
            console.log(DaemonInterfaceInstance.connectionState);
            if (DaemonInterfaceInstance.connectionState === "authenticated") {
                DaemonInterfaceInstance.refreshInformation();
            }
        }
        onInformationChanged: {
            console.log("Cover Information changed", infoKey, infoValue);

            switch (infoKey) {
            case DaemonInterface.INFO_BATTERY:
                lblBattery.text = infoValue
                break;
            case DaemonInterface.INFO_HEARTRATE:
                lblHeartrate.text = infoValue
                break;
            case DaemonInterface.INFO_STEPS:
                stepCount = parseInt(infoValue, 10) || 0;
                break
            }
        }
    }

    Component.onCompleted: {
        if (DaemonInterfaceInstance.connectionState === "authenticated") {
            DaemonInterfaceInstance.refreshInformation();
        }
    }
}
