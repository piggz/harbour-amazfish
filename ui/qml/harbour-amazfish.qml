import QtQuick 2.0
import "pages"
//import Nemo.Notifications 1.0
import org.SfietKonstantin.weatherfish 1.0
import Nemo.DBus 2.0
import uk.co.piggz.amazfish 1.0
import "./components/"
import "./components/platform"

ApplicationWindowPL
{
    id: app
    initialPage: Component { FirstPage { } }
    //cover: Qt.resolvedUrl("cover/CoverPage.qml")
    //allowedOrientations: defaultAllowedOrientations

    property int _lastNotificationId: 0
    property bool serviceActiveState: false
    property bool serviceEnabledState: false
    property int supportedFeatures: 0

    StylerPL {
        id: styler
    }
    TruncationModes { id: truncModes }

    //    Notification {
    //        id: notification
    //        expireTimeout: 5000
    //    }

    function showMessage(msg)
    {
        //        notification.replacesId = _lastNotificationId
        //        notification.previewBody = msg
        //        notification.publish()
        //        _lastNotificationId = notification.replacesId
    }

    function supportsFeature(feature) {
        console.log("Checking if feature is supported:", feature, (supportedFeatures & feature) === feature);
        return (supportedFeatures & feature) === feature;
    }

    BusyIndicatorPL {
        anchors.centerIn: parent
        visible: DaemonInterfaceInstance.operationRunning
        running: DaemonInterfaceInstance.operationRunning
    }

    CityManager {
        id: cityManager
    }
    
    Connections {
        target: DaemonInterfaceInstance
        onMessage: {
            showMessage(text);
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

    Timer {
        id: tmrStartup
        running: true
        repeat: false
        interval:300
        onTriggered: {
            app.pages.processCurrentItem();
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

    function tr(message) {
        return qsTr(message);
        // Return translated message.
        // In addition to the message, string formatting arguments can be passed
        // as well as short-hand for message.arg(arg1).arg(arg2)...
        //message = qsTranslate("", message);
        //for (var i = 1; i < arguments.length; i++)
        //    message = message.arg(arguments[i]);
        //return message;
    }

    function pushAttached(pagefile, options) {
        return app.pages.pushAttached(pagefile, options);
    }

    function createObject(page, options, parent) {
        var pc = Qt.createComponent(page);
        if (pc.status === Component.Error) {
            console.log('Error while creating component');
            console.log(pc.errorString());
            return null;
        }
        return pc.createObject(parent ? parent : app, options ? options : {})
    }

}
