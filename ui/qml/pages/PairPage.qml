import QtQuick 2.0
import Sailfish.Silica 1.0
import QtBluetooth 5.2
import QtQml.Models 2.2
import uk.co.piggz.amazfish 1.0

Page {
    id: page
    allowedOrientations: Orientation.Portrait
    backNavigation: !DaemonInterfaceInstance.pairing

    property string deviceType
    property string _placeholderText

    Connections {
        target: DaemonInterfaceInstance

        onPaired: {
            if (!error) {
                // Set values from the signal
                AmazfishConfig.pairedAddress = address
                AmazfishConfig.pairedName = name
                pageStack.pop(previousPage(previousPage()))
            } else {
                _placeholderText = error
            }
        }
    }

    BluetoothDiscoveryModel {
        id: discoveryModel
        running: true
        discoveryMode: BluetoothDiscoveryModel.DeviceDiscovery
    }

    DelegateModel {
        id: delegateModel
        model: discoveryModel

        groups: DelegateModelGroup {
            id: visibleItems
            name: "visible"
            includeByDefault: false
        }

        filterOnGroup: "visible"

        items.onChanged: {
            var itemsCount = items.count
            if (itemsCount > 0) {
                items.setGroups(0, itemsCount, "items")
            }

            for (var i = 0; i < itemsCount; ++i) {
                var item = items.get(i)
                item.inVisible = item.model.deviceName.indexOf(deviceType) !== -1
            }
        }

        delegate: ListItem {
            id: listItem
            contentHeight: Theme.itemSizeMedium
            onClicked: {
                discoveryModel.running = false
                DaemonInterfaceInstance.pair(model.deviceName, model.remoteAddress)
            }

            Item {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                height: nameLabel.height + addressLabel.height + Theme.paddingSmall

                Label {
                    id: nameLabel
                    truncationMode: TruncationMode.Fade
                    width: parent.width
                    text: model.deviceName
                    color: listItem.pressed ? Theme.highlightColor : Theme.primaryColor
                }

                Label {
                    id: addressLabel
                    anchors {
                        top: nameLabel.bottom
                        topMargin: Theme.paddingSmall
                    }
                    truncationMode: TruncationMode.Fade
                    width: parent.width
                    text: model.remoteAddress
                    font.pixelSize: Theme.fontSizeSmall
                    color: listItem.pressed ? Theme.secondaryHighlightColor : Theme.secondaryColor
                }
            }
        }
    }

    SilicaListView {
        id: listView
        anchors.fill: parent

        // Set to undefined when pairing to show busy indicator only
        model: !DaemonInterfaceInstance.pairing && !_placeholderText
               ? delegateModel
               : undefined

        header: PageHeader {
            title: qsTr("Pair Device")
            description: deviceType
        }

        PullDownMenu {
            busy: discoveryModel.running || DaemonInterfaceInstance.pairing

            MenuItem {
                enabled: !DaemonInterfaceInstance.pairing
                text: discoveryModel.running
                    ? qsTr("Stop scanning")
                    : qsTr("Scan for devices")
                onClicked: {
                    _placeholderText = ""
                    discoveryModel.running = !discoveryModel.running
                }
            }

            MenuLabel {
                visible: text
                text: discoveryModel.running
                    ? qsTr("Scanning for devices…")
                    : DaemonInterfaceInstance.pairing
                        ? qsTr("Pairing…")
                        : ""
            }
        }

        BusyIndicator {
            id: busyIndicator
            anchors.centerIn: parent
            size: BusyIndicatorSize.Large
            running: (discoveryModel.running && !listView.count) || DaemonInterfaceInstance.pairing
        }

        ViewPlaceholder {
            enabled: !busyIndicator.running && !listView.count
            text: _placeholderText || qsTr("No devices found")
            hintText: qsTr("Pull down to scan for devices")
        }
    }
}
