import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

CoverBackground {

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
            color: Theme.secondaryHighlightColor
            font.pixelSize: Theme.fontSizeMedium
        }

        Label {
            text: pairedAddress.value
            color: Theme.secondaryHighlightColor
            font.pixelSize: Theme.fontSizeMedium
        }

        Item {
            width: childrenRect.width
            height: childrenRect.height
            anchors.horizontalCenter: parent.horizontalCenter

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
            anchors.horizontalCenter: parent.horizontalCenter

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

    CoverActionList {
        id: coverAction

        CoverAction {
            iconSource: "image://theme/icon-cover-sync"
        }
    }
}
