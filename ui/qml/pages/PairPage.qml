import QtQuick 2.0
import org.kde.bluezqt 1.0 as BluezQt
import QtQml.Models 2.2
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

PageListPL {
    id: page
    //backNavigation: !DaemonInterfaceInstance.pairing
    title: qsTr("Pair Device")

    placeholderText: _placeholderText || qsTr("No devices found")
    placeholderEnabled: devicesModel.rowCount() > 0

    property string deviceType
    property variant aliases
    property string _placeholderText
    property string _deviceName
    property string _deviceAddress
    property QtObject adapter: _bluetoothManager ? _bluetoothManager.usableAdapter : null
    property QtObject _bluetoothManager : BluezQt.Manager

    function startDiscovery() {
        if (!adapter) {
            showMessage(qsTr("Bluetooth adapter is not available"))
            return
        }
        if (adapter.discovering) {
            return
        }
        adapter.startDiscovery()
    }

    function stopDiscovery() {
        if (adapter && adapter.discovering) {
            adapter.stopDiscovery()
        }
    }

    Timer {
        id: delayedATimer
        repeat: false
        running: true
        interval: 100
        onTriggered: {
            console.log("Delayed Manager operational:", _bluetoothManager.operational, _bluetoothManager.usableAdapter);
            if (typeof(devicesModel.filters) == 'number') {
                devicesModel.filters = BluezQt.DevicesModelPrivate.AllDevices;
            }
        }
    }

    Component.onCompleted: {
        console.log("Manager operational:", _bluetoothManager.operational, _bluetoothManager.usableAdapter);

    }

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

    BluezQt.DevicesModel {
        id: devicesModel
    }

    DelegateModel {
        id: delegateModel
        model: devicesModel

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
                var visible = false;
                if (item.model.FriendlyName.indexOf(deviceType) !== -1) {
                    visible = true;
                }
                for (var j = 0; j < aliases.count; j++) {
                    var aliasitem = aliases.get(j);
                    if (item.model.FriendlyName.indexOf(aliasitem.name) !== -1) {
                        visible = true;
                    }

                }
                item.inVisible = visible
            }
        }

        delegate: ListItemPL {
            id: listItem
            contentHeight: styler.themeItemSizeLarge
//            visible: model.FriendlyName.indexOf(deviceType) >= 0
            onClicked: {
                AmazfishConfig.pairedAddress = "";
                AmazfishConfig.pairedName = "";
                stopDiscovery();
                _deviceName = model.FriendlyName;
                _deviceAddress = AmazfishConfig.localAdapter+"/dev_" + model.Address.replace(/:/g, '_');

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
                    text: model.FriendlyName
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
                    text: model.Address
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
            iconSource: adapter && adapter.discovering ? "" : (styler.iconDeviceScan !== undefined ? styler.iconDeviceScan : "")
            text: adapter && adapter.discovering
                  ? qsTr("Stop scanning")
                  : qsTr("Scan for devices")
            onClicked: {
                _placeholderText = ""
                if (adapter && adapter.discovering) {
                    stopDiscovery();
                } else {
                    startDiscovery();
                    console.log("devicesModel.rowCount:" + devicesModel.rowCount() );
                }
            }
        }

        PageMenuItemPL {
            visible: text != ""
            text: adapter && adapter.discovering
                  ? qsTr("Scanning for devices…")
                  : DaemonInterfaceInstance.pairing
                    ? qsTr("Pairing…")
                    : ""
        }
    }

    BusyIndicatorPL {
        id: busyIndicator
        anchors.centerIn: parent
        running: (adapter && adapter.discovering && !page.count) || DaemonInterfaceInstance.connectionState == "pairing"
    }
}

