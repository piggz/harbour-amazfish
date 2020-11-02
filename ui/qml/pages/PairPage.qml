import QtQuick 2.0
import QtBluetooth 5.2
import QtQml.Models 2.2
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

PageListPL {
    id: page
    //backNavigation: !DaemonInterfaceInstance.pairing
    title: qsTr("Pair Device")

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

        delegate: ListItemPL {
            id: listItem
            contentHeight: styler.themeItemSizeLarge
            onClicked: {
                AmazfishConfig.pairedAddress = ""
                AmazfishConfig.pairedName = ""
                discoveryModel.running = false
                DaemonInterfaceInstance.pair(model.deviceName, model.remoteAddress)
            }

            Item {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: styler.themeHorizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                height: nameLabel.height + addressLabel.height + styler.themePaddingSmall

                LabelPL {
                    id: nameLabel
                    //truncationMode: TruncationMode.Fade
                    width: parent.width
                    text: model.deviceName
                    color: listItem.pressed ? styler.themeHighlightColor : styler.themePrimaryColor
                }

                LabelPL {
                    id: addressLabel
                    anchors {
                        top: nameLabel.bottom
                        topMargin: styler.themePaddingSmall
                    }
                    //truncationMode: TruncationMode.Fade
                    width: parent.width
                    text: model.remoteAddress
                    //font.pixelSize: Theme.fontSizeSmall
                    color: listItem.pressed ? styler.themeSecondaryHighlightColor : styler.themeSecondaryColor
                }
            }
        }
    }

    // Set to undefined when pairing to show busy indicator only
    model: !DaemonInterfaceInstance.pairing && !_placeholderText
           ? delegateModel
           : undefined


    pageMenu: PageMenuPL {
        //busy: discoveryModel.running || DaemonInterfaceInstance.pairing

        PageMenuItemPL {
            enabled: !DaemonInterfaceInstance.pairing
            text: discoveryModel.running
                  ? qsTr("Stop scanning")
                  : qsTr("Scan for devices")
            onClicked: {
                _placeholderText = ""
                discoveryModel.running = !discoveryModel.running
            }
        }

        PageMenuItemPL {
            visible: text
            text: discoveryModel.running
                  ? qsTr("Scanning for devices…")
                  : DaemonInterfaceInstance.pairing
                    ? qsTr("Pairing…")
                    : ""
        }
    }

    BusyIndicatorPL {
        id: busyIndicator
        anchors.centerIn: parent
        running: (discoveryModel.running && !page.count) || DaemonInterfaceInstance.pairing
    }

    //ViewPlaceholder {
    //    enabled: !busyIndicator.running && !page.count
    //    text: _placeholderText || qsTr("No devices found")
    //    hintText: qsTr("Pull down to scan for devices")
    //}
}

