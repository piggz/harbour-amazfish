import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

PageListPL {
    id: page

    title: qsTr("Settings Menu")


    model: settingsPages

    delegate: ListItemPL {

        visible: checkFeature()
        contentHeight: visible ? styler.themeItemSizeLarge : 0

        IconPL {
            id: settingsIcon
            anchors.verticalCenter: parent.verticalCenter
            iconName: icon
            height: styler.themeItemSizeLarge
            width: height
        }
        LabelPL {
            id: settingsName
            anchors.verticalCenter: settingsIcon.verticalCenter
            anchors.left: settingsIcon.right
            anchors.leftMargin: 20
            text: name
        }

        onClicked: {
            if(name === qsTr("Weather")) {
                var dlg = pageStack.push(Qt.resolvedUrl(url))
                dlg.cityManager = cityManager;
            }
            else if (name === qsTr("Donate")) {
                Qt.openUrlExternally("https://paypal.me/piggz")
            }
            else {
                pageStack.push(Qt.resolvedUrl(url))
            }
        }

        function checkFeature() {
            if(name === qsTr("Alarms")) {
                return supportsFeature(DaemonInterface.FEATURE_WEATHER)
            }
            else if (name === qsTr("Weather")) {
                return supportsFeature(DaemonInterface.FEATURE_ALARMS)
            }
            else {
                return true
            }
        }
    }


    ListModel {
        id: settingsPages

        property bool completed: false
        Component.onCompleted: {
            append({"icon": styler.iconContact, "name": qsTr("User Settings"), "url": "Settings-profile.qml"});
            append({"icon": styler.iconWatch, "name": qsTr("Device Settings"), "url": "Settings-device.qml"});
            append({"icon": styler.iconLevels, "name": qsTr("Application Settings"), "url": "Settings-app.qml"});
            append({"icon": styler.iconAlarm, "name": qsTr("Alarms"), "url": "Settings-alarms.qml"});
            append({"icon": styler.iconWeather, "name": qsTr("Weather"), "url": "AddCityDialog.qml"});
            append({"icon": styler.iconDiagnostic, "name": qsTr("Debug Info"), "url": "DebugInfo.qml"});
            append({"icon": styler.iconFavoriteSelected, "name": qsTr("Donate"), "url": "https://paypal.me/piggz"});
            completed = true;
        }
    }
}
