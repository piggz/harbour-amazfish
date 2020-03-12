import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"
import Nemo.Notifications 1.0
import org.nemomobile.mpris 1.0
import org.SfietKonstantin.weatherfish 1.0
import Nemo.Configuration 1.0
import org.nemomobile.dbus 2.0
import uk.co.piggz.amazfish 1.0

ApplicationWindow
{
    id: app
    initialPage: Component { FirstPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    property int _lastNotificationId: 0
    property bool serviceActiveState: false
    property bool serviceEnabledState: false
    property int supportedFeatures: 0

    ConfigurationValue {
        id: appRefreshWeather
        key: "/uk/co/piggz/amazfish/app/refreshweather"
        defaultValue: 60
    }

    ConfigurationValue {
        id: appAutoSyncData
        key: "/uk/co/piggz/amazfish/app/autosyncdata"
        defaultValue: true
    }

    onStateChanged: {
        console.log("State: " + state);
    }

    Notification {
        id: notification
        expireTimeout: 5000
    }

    function showMessage(msg)
    {
        notification.replacesId = _lastNotificationId
        notification.previewBody = msg
        notification.publish()
        _lastNotificationId = notification.replacesId
    }

    function supportsFeature(feature) {
        console.log("Checking if feature is supported:", feature, (supportedFeatures & feature) === feature);
        return (supportedFeatures & feature) === feature;
    }

    BusyIndicator {
        size: BusyIndicatorSize.Large
        anchors.centerIn: parent
        visible: DaemonInterfaceInstance.operationRunning
        running: DaemonInterfaceInstance.operationRunning
    }

    CityManager {
            id: cityManager
    }

    MprisManager {
        id: mprisManager
    }
    
    Connections {
        target: DaemonInterfaceInstance
        onMessage: {
            showMessage(text);
        }

        onButtonPressed: {
            console.log("Button pressed:", presses);
            
            if (presses == 3 && mprisManager.canGoPrevious) {
                mprisManager.previous();
            }
            
            if (presses == 2 && mprisManager.canGoNext) {
                mprisManager.next();
            }
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

    onSupportedFeaturesChanged: {
        console.log("Supported features:", supportedFeatures);
    }
}
