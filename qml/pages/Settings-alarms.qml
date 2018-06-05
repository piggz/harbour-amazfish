import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import "../components/"

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    ConfigurationValue {
        id: appNotifyConnect
        key: "/uk/co/piggz/amazfish/app/notifyconnect"
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
                title: qsTr("Alarms")
            }

            Alarm {
                alarmName: qsTr("Alarm 1")
                alarmId: "alarm1"
            }

            Alarm {
                alarmName: qsTr("Alarm 2")
                alarmId: "alarm2"


            }

            Alarm {
                alarmName: qsTr("Alarm 3")
                alarmId: "alarm3"

            }

            Alarm {
                alarmName: qsTr("Alarm 4")
                alarmId: "alarm4"

            }

            Alarm {
                alarmName: qsTr("Alarm 5")
                alarmId: "alarm5"

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

    }
    function saveSettings() {
    }

}
