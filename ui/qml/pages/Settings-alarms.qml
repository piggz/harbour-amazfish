import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Alarms")

    Column {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        Alarm {
            id: alarm1
            alarmName: qsTr("Alarm 1")
            alarmId: 1
        }

        Alarm {
            id: alarm2
            alarmName: qsTr("Alarm 2")
            alarmId: 2


        }

        Alarm {
            id: alarm3
            alarmName: qsTr("Alarm 3")
            alarmId: 3

        }

        Alarm {
            id: alarm4
            alarmName: qsTr("Alarm 4")
            alarmId: 4

        }

        Alarm {
            id: alarm5
            alarmName: qsTr("Alarm 5")
            alarmId: 5

        }

        SectionHeaderPL {
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
                DaemonInterfaceInstance.applyDeviceSetting(Amazfish.SETTING_ALARMS);
                app.pages.pop();
            }
        }
    }

    Component.onCompleted: {
        alarm1.init();
        alarm2.init();
        alarm3.init();
        alarm4.init();
        alarm5.init();

    }

    function saveSettings() {
        alarm1.save();
        alarm2.save();
        alarm3.save();
        alarm4.save();
        alarm5.save();
        tmrSetDelay.start();
    }
}
