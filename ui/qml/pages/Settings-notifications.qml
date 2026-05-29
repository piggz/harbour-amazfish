import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

DialogListPL {

    title: qsTr("Notifications settings")
    acceptText: qsTr("Save Settings")
    onAccepted: {
        saveSettings();
    }

    model: ListModel {
        id: settingsListModel
    }
    delegate: ListItemPL {
        width: parent.width
        height: textSwitch.height

        TextSwitchPL {
            id: textSwitch
            text: name
            checked: value
            anchors.verticalCenter: parent.verticalCenter
            onCheckedChanged: {
                settingsListModel.setProperty(index, "value", checked)
            }

        }
        menu: ContextMenuPL {
            id: contextMenu

            ContextMenuItemPL {
                iconName: styler.iconDelete
                text: qsTr("Remove")
                onClicked: {
                    settingsListModel.remove(index)
                }
            }

        }

    }

    Component.onCompleted: {
        var settings = AmazfishConfig.notificationSettings;
        var keys = Object.keys(settings);

        settingsListModel.clear();

        for (var i = 0; i < keys.length; i++) {
            var key = keys[i];
            settingsListModel.append({"name": key, "value": settings[key]});
        }
    }

    function saveSettings() {
            var newSettings = {};

            for (var i = 0; i < settingsListModel.count; i++) {
                var item = settingsListModel.get(i);
                newSettings[item.name] = item.value;
            }
            AmazfishConfig.notificationSettings = newSettings;
        }
}
