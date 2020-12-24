import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQml.Models 2.1

import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("BIP Display Items")


    ListModel {
        id: itemMode
        ListElement {
            itemText: "Status"
        }
        ListElement {
            itemText: "Ativity"
        }
    }

    Column
    {
        id: column
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        /*
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
*/

        Component {
            id: dragDelegate

            MouseArea {
                id: dragArea

                property bool held: false

                anchors { left: parent.left; right: parent.right }
                height: content.height

                drag.target: held ? content : undefined
                drag.axis: Drag.YAxis

                onPressAndHold: held = true
                onReleased: held = false

                Rectangle {
                    id: content
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        verticalCenter: parent.verticalCenter
                    }
                    width: dragArea.width; height: column.implicitHeight + 4

                    border.width: 1
                    border.color: "lightsteelblue"

                    color: dragArea.held ? "lightsteelblue" : "white"
                    Behavior on color { ColorAnimation { duration: 100 } }

                    radius: 2
                    Drag.active: dragArea.held
                    Drag.source: dragArea
                    Drag.hotSpot.x: width / 2
                    Drag.hotSpot.y: height / 2
                    states: State {
                        when: dragArea.held

                        ParentChange { target: content; parent: root }
                        AnchorChanges {
                            target: content
                            anchors { horizontalCenter: undefined; verticalCenter: undefined }
                        }
                    }

                    Column {
                        id: column
                        anchors { fill: parent; margins: 2 }

                        Text { text: 'Name: ' + itemText }
                    }
                }
                DropArea {
                    anchors { fill: parent; margins: 10 }

                    onEntered: {
                        visualModel.items.move(
                                    drag.source.DelegateModel.itemsIndex,
                                    dragArea.DelegateModel.itemsIndex)
                    }
                }
            }
        }
        DelegateModel {
            id: visualModel

            model: itemMode
            delegate: dragDelegate
        }

        ListView {
            id: view

            anchors { fill: parent; margins: 2 }

            model: visualModel

            spacing: 4
            cacheBuffer: 50
        }


        ButtonPL {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Save Settings")
            onClicked: {
                saveSettings();
            }
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
}


