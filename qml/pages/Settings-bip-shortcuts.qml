import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import uk.co.piggz.amazfish 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    ConfigurationValue {
        id: deviceDisplayStatus
        key: "/uk/co/piggz/amazfish/device/displaystatus"
        defaultValue: true
    }


    ConfigurationValue {
        id: deviceDisplayActivity
        key: "/uk/co/piggz/amazfish/device/displayactivity"
        defaultValue: true
    }


    ConfigurationValue {
        id: deviceDisplayWeather
        key: "/uk/co/piggz/amazfish/device/displayweather"
        defaultValue: true
    }


    ConfigurationValue {
        id: deviceDisplayAlarm
        key: "/uk/co/piggz/amazfish/device/displayalarm"
        defaultValue: true
    }

    ConfigurationValue {
        id: deviceDisplayTimer
        key: "/uk/co/piggz/amazfish/device/displaytimer"
        defaultValue: true
    }


    ConfigurationValue {
        id: deviceDisplayCompass
        key: "/uk/co/piggz/amazfish/device/displaycompass"
        defaultValue: true
    }

    ConfigurationValue {
        id: deviceDisplaySettings
        key: "/uk/co/piggz/amazfish/device/displaysettings"
        defaultValue: true
    }


    ConfigurationValue {
        id: deviceDisplayAliPay
        key: "/uk/co/piggz/amazfish/device/displayalipay"
        defaultValue: true
    }

    ConfigurationValue {
        id: deviceDisplayWeathershortcut
        key: "/uk/co/piggz/amazfish/device/displayweathershortcut"
        defaultValue: true
    }


    ConfigurationValue {
        id: deviceDisplayAliPayShortcut
        key: "/uk/co/piggz/amazfish/device/displayalipayshortcut"
        defaultValue: true
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
                title: qsTr("BIP Display Items")
            }

            TextSwitch {
                id: chkDisplayStatus
                width: parent.width
                text: qsTr("Status")
            }
            TextSwitch {
                id: chkDisplayActivity
                width: parent.width
                text: qsTr("Activity")
            }
            TextSwitch {
                id: chkDisplayWeather
                width: parent.width
                text: qsTr("Weather")
            }
            TextSwitch {
                id: chkDisplayAlarm
                width: parent.width
                text: qsTr("Alarm")
            }
            TextSwitch {
                id: chkDisplayTimer
                width: parent.width
                text: qsTr("Timer")
            }
            TextSwitch {
                id: chkDisplayCompass
                width: parent.width
                text: qsTr("Compass")
            }
            TextSwitch {
                id: chkDisplaySettings
                width: parent.width
                text: qsTr("Settings")
            }
            TextSwitch {
                id: chkDisplayAliPay
                width: parent.width
                text: qsTr("AliPay")
            }

            TextSwitch {
                id: chkDisplayWeatherShortcut
                width: parent.width
                text: qsTr("Weather Shortcut")
            }
            TextSwitch {
                id: chkDisplayAliPayShortcut
                width: parent.width
                text: qsTr("AliPay Shortcut")
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
        chkDisplayStatus.checked = deviceDisplayStatus.value;
        chkDisplayActivity.checked = deviceDisplayActivity.value;
        chkDisplayWeather.checked = deviceDisplayWeather.value;
        chkDisplayAlarm.checked = deviceDisplayAlarm.value;
        chkDisplayTimer.checked = deviceDisplayTimer.value;
        chkDisplayCompass.checked = deviceDisplayCompass.value;
        chkDisplaySettings.checked = deviceDisplaySettings.value;
        chkDisplayAliPay.checked = deviceDisplayAliPay.value;
        chkDisplayWeatherShortcut.checked = deviceDisplayWeathershortcut.value;
        chkDisplayAliPayShortcut.checked = deviceDisplayAliPayShortcut.value;
    }

    function saveSettings() {
        deviceDisplayStatus.value = chkDisplayStatus.checked;
        deviceDisplayActivity.value = chkDisplayActivity.checked;
        deviceDisplayWeather.value = chkDisplayWeather.checked;
        deviceDisplayAlarm.value = chkDisplayAlarm.checked;
        deviceDisplayTimer.value = chkDisplayTimer.checked;
        deviceDisplayCompass.value = chkDisplayCompass.checked;
        deviceDisplaySettings.value = chkDisplaySettings.checked;
        deviceDisplayAliPay.value = chkDisplayAliPay.checked;
        deviceDisplayWeathershortcut.value = chkDisplayWeatherShortcut.checked;
        deviceDisplayAliPayShortcut.value = chkDisplayAliPayShortcut.checked;

        tmrSetDelay.start();
    }

    Timer {
        //Allow data to sync
        id: tmrSetDelay
        repeat: false
        interval: 500
        running: false
        onTriggered: {
            DeviceInterfaceInstance.applyDeviceSetting(DeviceInterface.SETTING_DEVICE_DISPLAY_ITEMS);
        }
    }

}
