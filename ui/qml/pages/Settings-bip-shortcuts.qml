import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("BIP Display Items")


    Column {
        id: column
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        TextSwitchPL {
            id: chkDisplayStatus
            text: qsTr("Status")
        }
        TextSwitchPL {
            id: chkDisplayActivity
            text: qsTr("Activity")
        }
        TextSwitchPL {
            id: chkDisplayWeather
            text: qsTr("Weather")
        }
        TextSwitchPL {
            id: chkDisplayAlarm
            text: qsTr("Alarm")
        }
        TextSwitchPL {
            id: chkDisplayTimer
            text: qsTr("Timer")
        }
        TextSwitchPL {
            id: chkDisplayCompass
            text: qsTr("Compass")
        }
        TextSwitchPL {
            id: chkDisplaySettings
            text: qsTr("Settings")
        }
        TextSwitchPL {
            id: chkDisplayAliPay
            text: qsTr("AliPay")
        }

        TextSwitchPL {
            id: chkDisplayWeatherShortcut
            text: qsTr("Weather Shortcut")
        }
        TextSwitchPL {
            id: chkDisplayAliPayShortcut
            text: qsTr("AliPay Shortcut")
        }

        ButtonPL {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Save Settings")
            onClicked: {
                saveSettings();
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
        chkDisplayWeatherShortcut.checked = AmazfishConfig.deviceDisplayWeatherShortcut;
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
