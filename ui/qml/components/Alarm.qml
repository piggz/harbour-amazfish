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


    ConfigurationValue {
        id: alarmEnabled
        key: "/uk/co/piggz/amazfish/alarms/" + alarmId + "/enabled"
        defaultValue: false
    }

    ConfigurationValue {
        id: alarmHour
        key: "/uk/co/piggz/amazfish/alarms/" + alarmId + "/hour"
        defaultValue: 0
    }

    ConfigurationValue {
        id: alarmMinute
        key: "/uk/co/piggz/amazfish/alarms/" + alarmId + "/minute"
        defaultValue: 0
    }

    ConfigurationValue {
        id: alarmRepeat
        key: "/uk/co/piggz/amazfish/alarms/" + alarmId + "/repeat"
        defaultValue: 0
    }


    Column {
        width: parent.width
        height: childrenRect.height

        spacing: Theme.paddingLarge
        SectionHeader {
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
                horizontalAlignment: Text.AlignHCenter

            }
            Label {
                text: qsTr("Tue")
                width: (parent.width - Theme.paddingMedium) / 8
                horizontalAlignment: Text.AlignHCenter

            }
            Label {
                text: qsTr("Wed")
                width: (parent.width - Theme.paddingMedium) / 8
                horizontalAlignment: Text.AlignHCenter

            }
            Label {
                text: qsTr("Thu")
                width: (parent.width - Theme.paddingMedium) / 7
                horizontalAlignment: Text.AlignHCenter

            }
            Label {
                text: qsTr("Fri")
                width: (parent.width - Theme.paddingMedium) / 7
                horizontalAlignment: Text.AlignHCenter

            }
            Label {
                text: qsTr("Sat")
                width: (parent.width - Theme.paddingMedium) / 7
                horizontalAlignment: Text.AlignHCenter

            }
            Label {
                text: qsTr("Sun")
                width: (parent.width - Theme.paddingMedium) / 7
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    function init() {
        chkEnabled.checked = alarmEnabled.value;
        hour = alarmHour.value;
        minute = alarmMinute.value;

        var repeat = alarmRepeat.value;

        chkMo.checked = repeat & 1;
        chkTu.checked = repeat & 2;
        chkWe.checked = repeat & 4;
        chkTh.checked = repeat & 8;
        chkFr.checked = repeat & 16;
        chkSa.checked = repeat & 32;
        chkSu.checked = repeat & 64;

    }

    function save() {
        alarmEnabled.value = chkEnabled.checked;
        alarmHour.value = hour;
        alarmMinute.value = minute;

        var repeat = 0;
        repeat |= chkMo.checked ? 1 : 0;
        repeat |= chkTu.checked ? 2 : 0;
        repeat |= chkWe.checked ? 4 : 0;
        repeat |= chkTh.checked ? 8 : 0;
        repeat |= chkFr.checked ? 16 : 0;
        repeat |= chkSa.checked ? 32 : 0;
        repeat |= chkSu.checked ? 64 : 0;

        alarmRepeat.value = repeat;
    }


    function pad(n, width, z) {
        z = z || '0';
        n = n + '';
        return n.length >= width ? n : new Array(width - n.length + 1).join(z) + n;
    }
}
