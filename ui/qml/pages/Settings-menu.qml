import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    // Place our content in a Column.  The PageHeader is always placed at the top
    // of the page, followed by our content.
    SilicaListView {
        id: column

        width: page.width - 2*Theme.horizontalPageMargin
        height: parent.height

        header: PageHeader {
            title: qsTr("Settings Menu")
        }

        model: settingsPages

        delegate: ListItem {

            contentHeight: Theme.itemSizeMedium

            Icon {
                id: settingsIcon
                anchors.verticalCenter: parent.verticalCenter
                source: icon
                height: Theme.iconSizeMedium
                width: Theme.iconSizeMedium
            }
            Label {
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
            icon: "image://theme/graphic-weather-cloud-day-1"
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
