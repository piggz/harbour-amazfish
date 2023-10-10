import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Application Settings")

    property bool _ready: false
    // Place our content in a Column.  The PageHeader is always placed at the top
    // of the page, followed by our content.
    Column {
        id: column
        spacing: styler.themePaddingLarge
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium

        AdapterModel {
            id: adapters
        }

        ComboBoxPL {
            id: cboLocalAdapter
            model: adapters
            textRole: "path"
            label: qsTr("BT Adapter")
            Component.onCompleted: {
                cboLocalAdapter.value =  AmazfishConfig.localAdapter;
            }
        }

        SectionHeaderPL {
            visible: supportsFeature(Amazfish.FEATURE_ALERT)
            text: qsTr("Notifications")
        }

        TextSwitchPL {
            id: chkNotifyConnect
            visible: supportsFeature(Amazfish.FEATURE_ALERT)

            width: parent.width
            text: qsTr("Notify on connect")
        }

        TextSwitchPL {
            id: chkNotifyLowBattery
            visible: supportsFeature(Amazfish.FEATURE_ALERT)

            width: parent.width
            text: qsTr("Low battery notification")
        }

        TextSwitchPL {
            id: chkNavigationNotification
            visible: supportsFeature(Amazfish.FEATURE_ALERT)

            width: parent.width
            text: qsTr("Navigation notifications")
        }

        TextSwitchPL {
            id: chkTransliterate
            visible: supportsFeature(Amazfish.FEATURE_ALERT)

            width: parent.width
            text: qsTr("Transliterate notifications")
        }

        TextSwitchPL {
            id: chkSimulateEventSupport
            visible: supportsFeature(Amazfish.FEATURE_ALERT) && !supportsFeature(Amazfish.FEATURE_EVENT_REMINDER)

            width: parent.width
            text: qsTr("Simulate event reminder support")
        }

        SectionHeaderPL {
            text: qsTr("Refresh rates")
        }

        TextSwitchPL {
            id: chkAutoSyncData
            width: parent.width
            text: qsTr("Sync activity data each hour")
        }

        SliderPL {
            id: sldWeatherRefresh
            visible: supportsFeature(Amazfish.FEATURE_WEATHER)

            width: parent.width
            minimumValue: 15
            maximumValue: 120
            stepSize: 15
            label: qsTr("Refresh weather every (%1) minutes").arg(value)
        }

        SliderPL {
            id: sldCalendarRefresh
            width: parent.width
            minimumValue: 15
            maximumValue: 240
            stepSize: 15
            label: qsTr("Refresh calendar every (%1) minutes").arg(value)
            visible: supportsFeature(Amazfish.FEATURE_EVENT_REMINDER)
        }

        SectionHeaderPL {
            text: qsTr("Amazfish Service")
            visible: (ENABLE_SYSTEMD === "YES")
        }

        TextSwitchPL {
            id: chkServiceEnabled
            visible: (ENABLE_SYSTEMD === "YES")
            text: qsTr("Start service on boot")
            onCheckedChanged: {
                if (_ready) {
                    console.log("service enabled:", serviceEnabledState);
                    if (serviceEnabledState) {
                        systemdManager.disableService();
                    } else {
                        systemdManager.enableService();
                    }
                }
            }
        }

        LabelPL {
            width: parent.width
            text: qsTr("Start/Stop the Amazfish Background Service")
            visible: (ENABLE_SYSTEMD === "YES")
        }

        Row {
            id: serviceButtonRow
            visible: (ENABLE_SYSTEMD === "YES")
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            spacing: 10

            ButtonPL {
                id: start
                text: qsTr("Start")
                enabled: serviceActiveState ? false : true
                onClicked: {
                    systemdServiceIface.call("Start", ["replace"])
                }
            }

            ButtonPL {
                id: stop
                text: qsTr("Stop")
                enabled: serviceActiveState ? true : false
                onClicked: {
                    systemdServiceIface.call("Stop", ["replace"])
                }
            }
        }

        ButtonPL {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Button Actions")
            visible: supportsFeature(Amazfish.FEATURE_BUTTON_ACTION)
            onClicked: {
                app.pages.push(Qt.resolvedUrl("Settings-button-action.qml"))
            }
        }

        SectionHeaderPL {
        }

        ButtonPL {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Save Settings")
            onClicked: {
                saveSettings();
                app.pages.pop();
            }
        }
    }

    Component.onCompleted: {
        chkNotifyConnect.checked = AmazfishConfig.appNotifyConnect;
        sldWeatherRefresh.value = AmazfishConfig.appRefreshWeather;
        sldCalendarRefresh.value = AmazfishConfig.appRefreshCalendar;
        chkAutoSyncData.checked = AmazfishConfig.appAutoSyncData;
        chkNotifyLowBattery.checked = AmazfishConfig.appNotifyLowBattery;
        chkNavigationNotification.checked = AmazfishConfig.appNavigationNotification;
        chkSimulateEventSupport.checked = AmazfishConfig.appSimulateEventSupport;
        chkTransliterate.checked = AmazfishConfig.appTransliterate;

        chkServiceEnabled.checked = serviceEnabledState;
        _ready = true;
    }

    function saveSettings() {
        AmazfishConfig.appNotifyConnect = chkNotifyConnect.checked;
        AmazfishConfig.appRefreshWeather = sldWeatherRefresh.value;
        AmazfishConfig.appRefreshCalendar = sldCalendarRefresh.value;
        AmazfishConfig.appAutoSyncData = chkAutoSyncData.checked;
        AmazfishConfig.appNotifyLowBattery = chkNotifyLowBattery.checked;
        AmazfishConfig.appNavigationNotification = chkNavigationNotification.checked;
        AmazfishConfig.appSimulateEventSupport = chkSimulateEventSupport.checked;
        AmazfishConfig.localAdapter = cboLocalAdapter.value;
        AmazfishConfig.appTransliterate = chkTransliterate.checked ;
    }

}
