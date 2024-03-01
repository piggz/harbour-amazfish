import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQml.Models 2.1

import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Huami Display Items")

    ListModel {
        id: displayItems
    }

    Column
    {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        ListView {
            id: view
            width: parent.width
            height: page.height * 0.6
            model: displayItems
            spacing: 4
            cacheBuffer: 50
            clip: true
            delegate: DraggableItem {
                Item {
                    height: textSwitch.height * 1.5
                    width: view.width

                    TextSwitchPL {
                        id: textSwitch
                        text: itemText
                        checked: itemVisible
                        width: parent.width / 2
                        y: (parent.height - height) / 2
                        onCheckedChanged: {
                            itemVisible = checked;
                        }
                    }
                }
                draggedItemParent: page
                onMoveItemRequested: {
                    displayItems.move(from, to, 1);
                }
            }
        }

        ButtonPL {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Save Settings")
            onClicked: {
                saveDisplayItems();
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
                DaemonInterfaceInstance.applyDeviceSetting(Amazfish.SETTING_DEVICE_DISPLAY_ITEMS);
                app.pages.pop();
            }
        }
    }


    Component.onCompleted: {
        buildDisplayItemsModel();
    }

    function saveSettings() {
        tmrSetDelay.start();
    }

    function displayName(name) {
        if (name === "status") return qsTr("Status");
        if (name === "hr") return qsTr("Heartrate");
        if (name === "workout") return qsTr("Workout");
        if (name === "weather") return qsTr("Weather");
        if (name === "notifications") return qsTr("Notifications");
        if (name === "more") return qsTr("More");
        if (name === "dnd") return qsTr("Do Not Disturb");
        if (name === "alarm") return qsTr("Alarms");
        if (name === "music") return qsTr("Music");
        if (name === "time") return qsTr("Time");
        if (name === "mutephone") return qsTr("Mute Phone");
        if (name === "settings") return qsTr("Settings");
        if (name === "activity") return qsTr("Activity");
        if (name === "eventreminder") return qsTr("Event Reminder");
        if (name === "pai") return qsTr("PAI");
        if (name === "worldclock") return qsTr("World Clock");
        if (name === "stress") return qsTr("Stress");
        if (name === "period") return qsTr("Period");
        if (name === "spo2") return qsTr("SPO2");
        if (name === "alexa") return qsTr("Alexa");
        if (name === "compass") return qsTr("Compass");
        if (name === "alipay") return qsTr("AliPay");
        if (name === "timer") return qsTr("Timer");

        //If all else fails, use original name
        return name;
    }

    function buildDisplayItemsModel() {
        var saveditems = AmazfishConfig.deviceDisplayItems.split(",");

        for (var i = 0; i < saveditems.length; i++) {
            if (saveditems[i] !== "") {
                displayItems.append({"itemId": saveditems[i], "itemText": displayName(saveditems[i]), "itemVisible": true});
            }
        }

        var items = DaemonInterfaceInstance.supportedDisplayItems();
        for (i = 0; i < items.length; i++) {
            if (saveditems.indexOf(items[i]) < 0) {
                displayItems.append({"itemId": items[i], "itemText": displayName(items[i]), "itemVisible": false});
            }
        }
    }

    function saveDisplayItems() {
        var items = "";
        for(var i = 0; i < displayItems.count; ++i) {
            console.log(displayItems.get(i).itemText, displayItems.get(i).itemVisible );
            if (displayItems.get(i).itemVisible) {
                items = items + displayItems.get(i).itemId + ",";
            }
        }
        console.log(items);
        AmazfishConfig.deviceDisplayItems = items;
    }
}


