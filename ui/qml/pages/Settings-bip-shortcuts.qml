import QtQuick 2.0
import Sailfish.Silica 1.0
import uk.co.piggz.amazfish 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

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
        chkDisplayStatus.checked = AmazfishConfig.deviceDisplayStatus;
        chkDisplayActivity.checked = AmazfishConfig.deviceDisplayActivity;
        chkDisplayWeather.checked = AmazfishConfig.deviceDisplayWeather;
        chkDisplayAlarm.checked = AmazfishConfig.deviceDisplayAlarm;
        chkDisplayTimer.checked = AmazfishConfig.deviceDisplayTimer;
        chkDisplayCompass.checked = AmazfishConfig.deviceDisplayCompass;
        chkDisplaySettings.checked = AmazfishConfig.deviceDisplaySettings;
        chkDisplayAliPay.checked = AmazfishConfig.deviceDisplayAliPay;
        chkDisplayWeatherShortcut.checked = AmazfishConfig.deviceDisplayWeathershortcut;
        chkDisplayAliPayShortcut.checked = AmazfishConfig.deviceDisplayAliPayShortcut;
    }

    function saveSettings() {
        AmazfishConfig.deviceDisplayStatus = chkDisplayStatus.checked;
        AmazfishConfig.deviceDisplayActivity = chkDisplayActivity.checked;
        AmazfishConfig.deviceDisplayWeather = chkDisplayWeather.checked;
        AmazfishConfig.deviceDisplayAlarm = chkDisplayAlarm.checked;
        AmazfishConfig.deviceDisplayTimer = chkDisplayTimer.checked;
        AmazfishConfig.deviceDisplayCompass = chkDisplayCompass.checked;
        AmazfishConfig.deviceDisplaySettings = chkDisplaySettings.checked;
        AmazfishConfig.deviceDisplayAliPay = chkDisplayAliPay.checked;
        AmazfishConfig.deviceDisplayWeathershortcut = chkDisplayWeatherShortcut.checked;
        AmazfishConfig.deviceDisplayAliPayShortcut = chkDisplayAliPayShortcut.checked;

        tmrSetDelay.start();
    }

    Timer {
        //Allow data to sync
        id: tmrSetDelay
        repeat: false
        interval: 500
        running: false
        onTriggered: {
            DaemonInterfaceInstance.applyDeviceSetting(DaemonInterface.SETTING_DEVICE_DISPLAY_ITEMS);
        }
    }

}
