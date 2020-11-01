import QtQuick 2.0
import "../components"
import "../components/platform"
import uk.co.piggz.amazfish 1.0

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
            source: icon
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

        ListElement {
            icon: "image://theme/icon-m-contact"
            name: qsTr("User Settings")
            url: "Settings-profile.qml"
        }
        ListElement {
            icon: "image://theme/icon-m-watch"
            name: qsTr("Device Settings")
            url: "Settings-device.qml"
        }
        ListElement {
            icon: "image://theme/icon-m-levels"
            name: qsTr("Application Settings")
            url: "Settings-app.qml"
        }
        ListElement {
            icon: "image://theme/icon-m-alarm"
            name: qsTr("Alarms")
            url: "Settings-alarms.qml"
        }
        ListElement {
            icon: "image://theme/icon-m-weather-d212-light"
            name: qsTr("Weather")
            url: "AddCityDialog.qml"
        }
        ListElement {
            icon: "image://theme/icon-m-diagnostic"
            name: qsTr("Debug Info")
            url: "DebugInfo.qml"
        }
        ListElement {
            icon: "image://theme/icon-m-favorite-selected"
            name: qsTr("Donate")
            url: "https://paypal.me/piggz"
        }
    }
}
