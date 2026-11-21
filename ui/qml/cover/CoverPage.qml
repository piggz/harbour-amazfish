import QtQuick 2.0
import Sailfish.Silica 1.0
import uk.co.piggz.amazfish 1.0

CoverBackground {
    Column {
        anchors.fill: parent
        spacing: Theme.paddingMedium
        anchors.margins: Theme.paddingMedium

        Label {
            text: AmazfishConfig.pairedName
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
                text: _InfoBatteryPercent
                color: Theme.secondaryHighlightColor
                height: Theme.iconSizeMedium
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: Theme.fontSizeLarge
            }
        }

        //Steps
        Row {
            spacing: Theme.paddingMedium
            width: childrenRect.width
            anchors.horizontalCenter: parent.horizontalCenter
            visible: supportsFeatureRefresh(Amazfish.FEATURE_STEPS)

            Image {
                id: imgSteps
                source: "../custom-icons/icon-m-steps.png"
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
                text: _InfoSteps
            }
        }

        //Heartrate
        Row {
            spacing: Theme.paddingMedium
            width: childrenRect.width
            anchors.horizontalCenter: parent.horizontalCenter
            visible: supportsFeatureRefresh(Amazfish.FEATURE_HRM)

            Image {
                id: imgHeartrate
                source: "../custom-icons/icon-m-heartrate.png"
                height: Theme.iconSizeMedium
                width: height
            }
            Label {
                id: lblHeartrate
                text: _InfoHeartrate
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeLarge
                height: Theme.iconSizeMedium
                verticalAlignment: Text.AlignVCenter
                width: height
            }
        }
        //Spo2
        Row {
            spacing: Theme.paddingMedium
            width: childrenRect.width
            anchors.horizontalCenter: parent.horizontalCenter
            visible: supportsFeature(Amazfish.FEATURE_SPO2)

            Image {
                id: imgSpo2
                source: "../custom-icons/icon-m-spo2.png"
                height: Theme.iconSizeMedium
                width: height
            }
            Label {
                id: lblSpo2
                text: _InfoSpo2
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
                if (supportsFeatureRefresh(Amazfish.FEATURE_HRM)) {
                    DaemonInterfaceInstance.requestManualHeartrate();
                }
            }
        }

        CoverAction {
            iconSource: "image://theme/icon-m-cloud-download";
            onTriggered: {
                DaemonInterfaceInstance.miBandService().fetchActivityData();
            }
        }
    }
}
