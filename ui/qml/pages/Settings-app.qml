import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
 import uk.co.piggz.amazfish 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    ConfigurationValue {
        id: appNotifyConnect
        key: "/uk/co/piggz/amazfish/app/notifyconnect"
        defaultValue: true
    }

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

    ConfigurationValue {
        id: appNotifyLowBattery
        key: "/uk/co/piggz/amazfish/app/notifylowbattery"
        defaultValue: false
    }

    ConfigurationValue {
        id: appRefreshCalendar
        key: "/uk/co/piggz/amazfish/app/refreshcalendar"
        defaultValue: 60
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView


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
                title: qsTr("Device Settings")
            }

            SectionHeader {
                text: qsTr("Notifications")
            }

            TextSwitch {
                id: chkNotifyConnect
                visible: supportsFeature(DaemonInterface.FEATURE_NOTIFIATION)

                width: parent.width
                text: qsTr("Notify on connect")
            }

            TextSwitch {
                id: chkNotifyLowBattery
                visible: supportsFeature(DaemonInterface.FEATURE_NOTIFIATION)

                width: parent.width
                text: qsTr("Low battery notification")
            }

            SectionHeader {
                text: qsTr("Refresh rates")
            }

            TextSwitch {
                id: chkAutoSyncData
                width: parent.width
                text: qsTr("Sync activity data each hour")
            }

            Slider {
                id: sldWeatherRefresh
                visible: supportsFeature(DaemonInterface.FEATURE_WEATHER)

                width: parent.width
                minimumValue: 15
                maximumValue: 120
                stepSize: 15
                label: qsTr("Refresh weather every (") + value + qsTr(") minutes")
            }

            Slider {
                id: sldCalendarRefresh
                width: parent.width
                minimumValue: 15
                maximumValue: 240
                stepSize: 15
                label: qsTr("Refresh calendar every (") + value + qsTr(") minutes")
            }

            SectionHeader {
                text: "Amazfish Service"
            }

            TextSwitch {
                id: chkServiceEnabled
                checked: serviceEnabledState === false ? false : true
                text: qsTr("Start service on boot")
                onClicked: {
                    if (serviceEnabledState) {
                        systemdManager.disableService();
                    } else {
                        systemdManager.enableService();
                    }
                }
            }

            Label {
                width: parent.width
                text: qsTr("Start/Stop the Amazfish Background Service")
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
            }

            Row {
                id: serviceButtonRow

                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                spacing: 10

                Button {
                    id: start
                    text: qsTr("Start")
                    enabled: serviceActiveState ? false : true
                    onClicked: {
                        systemdServiceIface.call("Start", ["replace"])
                    }
                }

                Button {
                    id: stop
                    text: qsTr("Stop")
                    enabled: serviceActiveState ? true : false
                    onClicked: {
                        systemdServiceIface.call("Stop")
                    }
                }
            }

            Separator {
                width: parent.width
                horizontalAlignment: Qt.AlignHCenter
                color: Theme.highlightColor
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Save Settings")
                onClicked: {
                    saveSettings();
                }
            }
        }
    }
    Component.onCompleted: {
        chkNotifyConnect.checked = appNotifyConnect.value;
        sldWeatherRefresh.value = appRefreshWeather.value;
        sldCalendarRefresh.value = appRefreshCalendar.value;
        chkAutoSyncData.checked = appAutoSyncData.value;
        chkNotifyLowBattery.checked = appNotifyLowBattery.value;
    }

    function saveSettings() {
        appNotifyConnect.value = chkNotifyConnect.checked;
        appRefreshWeather.value = sldWeatherRefresh.value;
        appRefreshCalendar.value = sldCalendarRefresh.value;
        appAutoSyncData.value = chkAutoSyncData.checked;
        appNotifyLowBattery.value = chkNotifyLowBattery.checked;

        if (supportsFeature(DaemonInterface.FEATURE_WEATHER)) {
            weather.refresh();
        }
    }

}
