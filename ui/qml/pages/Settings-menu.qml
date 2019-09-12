import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import "../components/"

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
                title: qsTr("Settings Menu")
            }

            SettingsPageButton {
                text: qsTr("User Settings")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("Settings-profile.qml"))
                }
            }
            SettingsPageButton {
                text: qsTr("Device Settings")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("Settings-device.qml"))
                }
            }

            SettingsPageButton {
                text: qsTr("Application Settings")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("Settings-app.qml"))
                }
            }

            SettingsPageButton {
                text: qsTr("Alarms")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("Settings-alarms.qml"))
                }
            }
            
            SettingsPageButton {
                text: qsTr("Weather")
                onClicked: {
                    var dlg = pageStack.push(Qt.resolvedUrl("AddCityDialog.qml"))
                    dlg.cityManager = cityManager;
                }
            }

            SettingsPageButton {
                text: serviceActiveState == false ? qsTr("Start Service") : qsTr("Stop Service")

                onClicked: {
                    systemdServiceIface.call(serviceActiveState ? "Stop" : "Start", ["replace"])                }
            }

            SettingsPageButton {
                text: serviceEnabledState == false ? qsTr("Enable Service") : qsTr("Disable Service")

                onClicked: {
                    if (serviceEnabledState) {
                        systemdManager.disableService();
                    } else {
                        systemdManager.enableService();
                    }
                }
            }

            SettingsPageButton {
                text: qsTr("Debug Info")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("DebugInfo.qml"))
                }
            }

            SettingsPageButton {
                text: qsTr("Donate")
                onClicked: {
                    Qt.openUrlExternally("https://paypal.me/piggz");
                }
            }
        }
    }

}
