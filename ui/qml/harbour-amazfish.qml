import QtQuick 2.0
import "pages"
//import Nemo.Notifications 1.0
import org.SfietKonstantin.weatherfish 1.0
import Nemo.DBus 2.0
import uk.co.piggz.amazfish 1.0
import com.pipacs.o2 1.0
import "./components/"
import "./components/platform"

ApplicationWindowPL
{
    id: app
    initialPage: Component { FirstPage { } }
    property var    rootPage: null
    //cover: Qt.resolvedUrl("cover/CoverPage.qml")
    //allowedOrientations: defaultAllowedOrientations

    property int _lastNotificationId: 0
    property bool serviceActiveState: false
    property bool serviceEnabledState: false
    property int supportedFeatures: 0
    property bool stravaLinked: false
    property bool firstPass: true
    property string _lastMessage: ""
    property string _percentText: ""

    //Device State
    readonly property string _connectionState: DaemonInterfaceInstance.connectionState
    readonly property bool _disconnected: _connectionState === "disconnected"
    readonly property bool _connecting: _connectionState === "connecting"
    readonly property bool _connected: _connectionState === "connected"
    readonly property bool _authenticated: _connectionState === "authenticated"

    //Device Informatino
    property int _InfoSteps: 0
    property int _InfoBatteryPercent: 0
    property int _InfoHeartrate: 0

    StylerPL {
        id: styler
    }
    TruncationModes { id: truncModes }

    PopupPL {
        id: popup
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
        running: ENABLE_SYSTEMD === "YES"
        onTriggered: {
            systemdServiceIface.updateProperties()

            if (serviceActiveState == false && firstPass) {
                systemdServiceIface.call("Start", ["replace"])
                firstPass = false;
            }
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

    O2 {
        id: o2strava
        clientId: STRAVA_CLIENT_ID
        clientSecret: STRAVA_CLIENT_SECRET
        scope: "activity:write,activity:read_all"
        requestUrl: "https://www.strava.com/oauth/authorize"
        tokenUrl: "https://www.strava.com/oauth/token"
        refreshTokenUrl: "https://www.strava.com/api/v3/oauth/token"
        replyContent: "<html><head><body><h1>You can now close this window</h1></body></head></html>"

        onOpenBrowser: {
            Qt.openUrlExternally(url);
        }

        onCloseBrowser: {
        }

        onLinkedChanged: {
            stravaLinked = linked
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
            if (ENABLE_SYSTEMD === "YES"){
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
            if (name == "harbour-amazfish.service" && ENABLE_SYSTEMD === "YES") {
                systemdServiceIface.updateProperties()
            }
        }

        function enableService() {
            if(ENABLE_SYSTEMD === "YES") {
                systemdManager.typedCall("EnableUnitFiles", [{"type":"as", "value":["harbour-amazfish.service"]}, {"type":"b", "value":false}, {"type":"b", "value":true}])
                reload();
            }
        }

        function disableService() {
            if (ENABLE_SYSTEMD === "YES") {
                systemdManager.typedCall("DisableUnitFiles", [{"type":"as", "value":["harbour-amazfish.service"]}, {"type":"b", "value":false}])
                reload();
            }
        }

        function reload() {
            if (ENABLE_SYSTEMD === "YES") {
                systemdManager.call("Reload");
            }
        }
    }

    onSupportedFeaturesChanged: {
        console.log("Supported features:", supportedFeatures);
    }

    on_ConnectionStateChanged: console.log(_connectionState)

    on_AuthenticatedChanged: {
        if (_authenticated) {
            _refreshInformation()
        }
    }

    Connections {
        target: DaemonInterfaceInstance
        onInformationChanged: {
            console.log("Information changed", infoKey, infoValue);

            switch (infoKey) {
            case Amazfish.INFO_BATTERY:
                _InfoBatteryPercent = parseInt(infoValue, 10) || 0;
                break;
            case Amazfish.INFO_HEARTRATE:
                _InfoHeartrate = parseInt(infoValue, 10) || 0;
                break;
            case Amazfish.INFO_STEPS:
                _InfoSteps = parseInt(infoValue, 10) || 0;
                break;
            }
        }
        onDownloadProgress: {
            _percentText = percent + "%";
        }
    }

    //======================Application Global Functions========================

    function showMessage(msg)
    {
        _lastMessage = msg
        popup.showMessage(msg)
    }

    function supportsFeature(feature) {
        // console.log("Checking if feature is supported:", feature, (supportedFeatures & feature) === feature);
        return (supportedFeatures & feature) === feature;
    }

    function _refreshInformation() {
        if (!_authenticated) {
            return
        }

        supportedFeatures = DaemonInterfaceInstance.supportedFeatures();
        console.log("Supported features", supportedFeatures);

        DaemonInterfaceInstance.refreshInformation();
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
