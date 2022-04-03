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

    placeholderText: _placeholderText || qsTr("No devices found")
    placeholderEnabled: discoveryModel.rowCount() > 0

    property string deviceType
    property string _placeholderText
    property string _deviceName
    property string _deviceAddress

    Connections {
        target: DaemonInterfaceInstance

        onConnectionStateChanged: {
            if (DaemonInterfaceInstance.connectionState == "paired" || DaemonInterfaceInstance.connectionState == "connected") {
                AmazfishConfig.pairedAddress = _deviceAddress
                AmazfishConfig.pairedName = _deviceName
                console.log("Paired with", AmazfishConfig.pairedName, AmazfishConfig.pairedAddress, _deviceName, _deviceAddress);
                app.pages.pop(app.rootPage);
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
                AmazfishConfig.pairedAddress = "";
                AmazfishConfig.pairedName = "";
                discoveryModel.running = false;
                _deviceName = model.deviceName;
                _deviceAddress = AmazfishConfig.localAdapter+"/dev_" + model.remoteAddress.replace(/:/g, '_');

                DaemonInterfaceInstance.pair(_deviceName, _deviceAddress)
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
        running: (discoveryModel.running && !page.count) || DaemonInterfaceInstance.connectionState == "pairing"
    }
}

