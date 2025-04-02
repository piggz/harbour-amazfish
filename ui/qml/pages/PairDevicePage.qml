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

    placeholderText: qsTr("No devices found")
    placeholderEnabled: (delegateModel.count === 0) && !busy
    property bool busy: (adapter && adapter.discovering && !page.count) || DaemonInterfaceInstance.connectionState == "pairing"
    //busy: discoveryModel.running || DaemonInterfaceInstance.pairing

    property string deviceType
    property string pattern
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

    DeviceListModel {
        id: deviceListModel
    }

    function deviceListFind(friendlyName) {
        for (var i = 0; i < deviceListModel.count; i++) {
            var device = deviceListModel.get(i);
            try {
                var regex = new RegExp(device.pattern)
                if (regex.test(friendlyName)) {
                    return device;
                }

            } catch(e) {
                console.error("Invalid regex: " + device.pattern, e);
            }
        }
        return {
            deviceType: "",
            icon: "",
            auth: false,
            pattern: ""
        }
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
                var btItem = items.get(i)
                var device = deviceListFind(btItem.model.FriendlyName);
                // console.log(btItem.model.FriendlyName + " " + JSON.stringify(device))
                btItem.inVisible = (device.deviceType !== "");
            }

        }

        delegate: ListItemPL {
            id: listItem
            property var device: deviceListFind(model.FriendlyName)
            contentHeight: styler.themeItemSizeLarge

            onClicked: {
                if (device.auth) {
                    var authdialog = app.pages.push(Qt.resolvedUrl("./AuthKeyDialog.qml"));

                    authdialog.accepted.connect(function() {
                        page.stopDiscovery();
                        _deviceName = model.FriendlyName;
                        _deviceAddress = AmazfishConfig.localAdapter+"/dev_" + model.Address.replace(/:/g, '_');
                        DaemonInterfaceInstance.pair(_deviceName, device.deviceType, _deviceAddress)
                    })
                    return;
                }

                stopDiscovery();
                _deviceName = model.FriendlyName;
                _deviceAddress = AmazfishConfig.localAdapter+"/dev_" + model.Address.replace(/:/g, '_');

                DaemonInterfaceInstance.pair(_deviceName, device.deviceType, _deviceAddress)
            }

            Item {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: styler.themeHorizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                height: nameLabel.height + addressLabel.height + styler.themePaddingSmall

                Image {
                    id: iconImage
                    source: device.icon
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                }


                LabelPL {
                    id: nameLabel
                    //truncationMode: TruncationMode.Fade
                    anchors.left: iconImage.right
                    width: parent.width - iconImage.paintedWidth
                    text: model.FriendlyName
                    color: listItem.pressed ? styler.themeHighlightColor : styler.themePrimaryColor
                }

                LabelPL {
                    id: addressLabel
                    anchors {
                        top: nameLabel.bottom
                        topMargin: styler.themePaddingSmall
                        left: iconImage.right
                    }
                    //truncationMode: TruncationMode.Fade
                    width: nameLabel.width
                    text: model.Address
                    //font.pixelSize: Theme.fontSizeSmall
                    color: listItem.pressed ? styler.themeSecondaryHighlightColor : styler.themeSecondaryColor
                }
            }
        }
    }

    // Set to undefined when pairing to show busy indicator only
    model: (DaemonInterfaceInstance !== undefined) && !DaemonInterfaceInstance.pairing && delegateModel.count > 0
           ? delegateModel
           : undefined


    pageMenu: PageMenuPL {
        //busy: discoveryModel.running || DaemonInterfaceInstance.pairing

        PageMenuItemPL {
            enabled: (DaemonInterfaceInstance !== undefined) && !DaemonInterfaceInstance.pairing
            iconSource: adapter && adapter.discovering ? "" : (styler.iconDeviceScan !== undefined ? styler.iconDeviceScan : "")
            text: adapter && adapter.discovering
                  ? qsTr("Stop scanning")
                  : qsTr("Scan for devices")
            onClicked: {
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

    background: BusyIndicatorPL {
        id: busyIndicator
        anchors.centerIn: parent
        running: busy
    }



}
