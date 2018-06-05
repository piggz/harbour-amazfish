import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Item {
    id: itmAlarm
    property alias enabled: chkEnabled.checked
    property int hour: 0
    property int minute: 0
    property string alarmName: "Alarm 1"
    property string alarmId: "alarm1"

    height: childrenRect.height + Theme.paddingLarge
    width: parent.width


    Column {
        width: parent.width
        height: childrenRect.height

        spacing: Theme.paddingLarge
        Label {
            text: alarmName
        }

        Row {
            spacing: Theme.paddingMedium
            width: parent.width
            height: childrenRect.height

            TextSwitch {
                id: chkEnabled
                text: qsTr("Enabled")
                width: parent.width / 2
            }
            Button {
                id: button
                text: "Time: " + pad(hour, 2) + ":" + pad(minute, 2)

                onClicked: {
                    var dialog = pageStack.push("Sailfish.Silica.TimePickerDialog", {
                                                    hour: itmAlarm.hour,
                                                    minute:itmAlarm.minute,
                                                    hourMode: DateTime.TwentyFourHours
                                                })
                    dialog.accepted.connect(function() {
                        itmAlarm.hour = dialog.hour;
                        itmAlarm.minute = dialog.minute;
                    })
                }
            }
        }
        Grid {
            columns: 7
            spacing: Theme.paddingSmall
            width: parent.width
            height: childrenRect.height

            TextSwitch {
                id: chkMo
                width: (parent.width - Theme.paddingMedium) / 8
            }
            TextSwitch {
                id: chkTu
                width: (parent.width - Theme.paddingMedium) / 8
            }
            TextSwitch {
                id: chkWe
                width: (parent.width - Theme.paddingMedium) / 8
            }
            TextSwitch {
                id: chkTh
                width: (parent.width - Theme.paddingMedium) / 8
            }
            TextSwitch {
                id: chkFr
                width: (parent.width - Theme.paddingMedium) / 8
            }
            TextSwitch {
                id: chkSa
                width: (parent.width - Theme.paddingMedium) / 8
            }
            TextSwitch {
                id: chkSu
                width: (parent.width - Theme.paddingMedium) / 8
            }
            Label {
                text: qsTr("Mon")
                width: (parent.width - Theme.paddingMedium) / 8

            }
            Label {
                text: qsTr("Tue")
                width: (parent.width - Theme.paddingMedium) / 8

            }
            Label {
                text: qsTr("Wed")
                width: (parent.width - Theme.paddingMedium) / 8

            }
            Label {
                text: qsTr("Thu")
                width: (parent.width - Theme.paddingMedium) / 7

            }
            Label {
                text: qsTr("Fri")
                width: (parent.width - Theme.paddingMedium) / 7

            }
            Label {
                text: qsTr("Sat")
                width: (parent.width - Theme.paddingMedium) / 7

            }
            Label {
                text: qsTr("Sun")
                width: (parent.width - Theme.paddingMedium) / 7

            }
        }
    }



    function pad(n, width, z) {
        z = z || '0';
        n = n + '';
        return n.length >= width ? n : new Array(width - n.length + 1).join(z) + n;
    }
}
