import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import org.SfietKonstantin.weatherfish 1.0
import uk.co.piggz.amazfish 1.0
import org.nemomobile.dbus 2.0
import "../components/"

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property bool needsProfileSet: false
    property var day: new Date()

    property bool serviceActiveState: false
    property bool serviceEnabledState: false

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
    
    onStatusChanged: {
        if (status === PageStatus.Active) {
            //            if (!pageStack._currentContainer.attachedContainer) {
            pageStack.pushAttached(Qt.resolvedUrl("StepsPage.qml"))
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
                text: DaemonInterfaceInstance.connectionState == "disconnected" ? qsTr("Connect to watch") : qsTr("Disconnect from watch")
                onClicked: {
                    if (DaemonInterfaceInstance.connectionState == "disconnected") {
                        DaemonInterfaceInstance.connectToDevice(pairedAddress.value);
                    } else {
                        DaemonInterfaceInstance.disconnect();
                    }
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
                        DaemonInterfaceInstance.requestManualHeartrate();
                    }
                }
            }

            Row {
                spacing: Theme.paddingLarge
                width: parent.width

                Button {
                    text: serviceActiveState == false ? qsTr("Start Service") : qsTr("Stop Service")

                    onClicked: {
                        systemdServiceIface.call(serviceActiveState ? "Stop" : "Start", ["replace"])                }
                }

                Button {
                    text: serviceEnabledState == false ? qsTr("Enable Service") : qsTr("Disable Service")

                    onClicked: {
                        if (serviceEnabledState) {
                            systemdManager.disableService();
                        } else {
                            systemdManager.enableService();
                        }
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
        target: DaemonInterfaceInstance
        onConnectionStateChanged: {
            console.log(DaemonInterfaceInstance.connectionState);
            if (DaemonInterfaceInstance.connectionState === "authenticated") {
                DaemonInterfaceInstance.refreshInformation();
            }
        }
        onInformationChanged: {
            console.log("Information changed", infoKey, infoValue);

            switch (infoKey) {
            case DaemonInterface.INFO_BATTERY:
                lblBattery.text = infoValue
                break;
            case DaemonInterface.INFO_HEARTRATE:
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
    }

    //SystemD Service
    Timer {
        id: checkState
        interval: 5000
        repeat: true
        running: true
        onTriggered: {
            systemdServiceIface.updateProperties()
        }
    }

    DBusInterface {
        id: systemdServiceIface
        bus: DBus.SessionBus
        service: 'org.freedesktop.systemd1'
        path: '/org/freedesktop/systemd1/unit/harbour_2damazfish_2eservice'
        iface: 'org.freedesktop.systemd1.Unit'

        signalsEnabled: true
        function updateProperties() {
            var activeProperty = systemdServiceIface.getProperty("ActiveState");
            if (activeProperty === "active") {
                serviceActiveState = true;
            } else {
                serviceActiveState = false;
            }

            var serviceEnabledProperty = systemdServiceIface.getProperty("UnitFileState");
            if (serviceEnabledProperty === "enabled") {
                serviceEnabledState = true;
            }
            else {
                serviceEnabledState = false;
            }

        }

        onPropertiesChanged: updateProperties()
        Component.onCompleted: updateProperties()
    }

    DBusInterface {
        id: systemdManager
        bus: DBus.SessionBus
        service: "org.freedesktop.systemd1"
        path: "/org/freedesktop/systemd1"
        iface: "org.freedesktop.systemd1.Manager"
        signalsEnabled: true

        signal unitNew(string name)
        onUnitNew: {
            if (name == "harbour-amazfish.service") {
                systemdServiceIface.updateProperties()
            }
        }

        function enableService() {
            systemdManager.typedCall("EnableUnitFiles", [{"type":"as", "value":["harbour-amazfish.service"]}, {"type":"b", "value":false}, {"type":"b", "value":true}])
        }

        function disableService() {
            systemdManager.typedCall("DisableUnitFiles", [{"type":"as", "value":["harbour-amazfish.service"]}, {"type":"b", "value":false}])

        }
    }

}
