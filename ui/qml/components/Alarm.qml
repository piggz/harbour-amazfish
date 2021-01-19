import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "."
import "./platform"

Item {
    id: itmAlarm

    property alias enabled: chkEnabled.checked
    property int hour: 0
    property int minute: 0
    property string alarmName: "Alarm 1"
    property int alarmId: 1

    height: childrenRect.height + styler.themePaddingLarge
    width: parent.width

    Column {
        width: parent.width
        height: childrenRect.height

        spacing: styler.themePaddingLarge
        SectionHeaderPL {
            text: alarmName
        }

        Row {
            spacing: styler.themePaddingMedium
            width: parent.width
            height: childrenRect.height

            TextSwitchPL {
                id: chkEnabled
                text: qsTr("Enabled")
                width: parent.width / 2
            }
            ButtonPL {
                id: button
                text: "Time: " + pad(hour, 2) + ":" + pad(minute, 2)

                onClicked: {
                    var dialog = pageStack.push(pickerComponent, {
                                                    hour: itmAlarm.hour,
                                                    minute:itmAlarm.minute,
                                                })
                    dialog.accepted.connect(function() {
                        itmAlarm.hour = dialog.hour;
                        itmAlarm.minute = dialog.minute;
                    })
                }
                Component {
                    id: pickerComponent
                    TimePickerDialogPL {}
                }
            }
        }
        Grid {
            columns: 7
            spacing: styler.themePaddingSmall
            width: parent.width
            height: childrenRect.height

            TextSwitchPL {
                id: chkMo
                width: (parent.width - styler.themePaddingMedium) / 8
            }
            TextSwitchPL {
                id: chkTu
                width: (parent.width - styler.themePaddingMedium) / 8
            }
            TextSwitchPL {
                id: chkWe
                width: (parent.width - styler.themePaddingMedium) / 8
            }
            TextSwitchPL {
                id: chkTh
                width: (parent.width - styler.themePaddingMedium) / 8
            }
            TextSwitchPL {
                id: chkFr
                width: (parent.width - styler.themePaddingMedium) / 8
            }
            TextSwitchPL {
                id: chkSa
                width: (parent.width - styler.themePaddingMedium) / 8
            }
            TextSwitchPL {
                id: chkSu
                width: (parent.width - styler.themePaddingMedium) / 8
            }
            LabelPL {
                text: qsTr("Mon")
                width: (parent.width - styler.themePaddingMedium) / 8
                horizontalAlignment: Text.AlignHCenter

            }
            LabelPL {
                text: qsTr("Tue")
                width: (parent.width - styler.themePaddingMedium) / 8
                horizontalAlignment: Text.AlignHCenter

            }
            LabelPL {
                text: qsTr("Wed")
                width: (parent.width - styler.themePaddingMedium) / 8
                horizontalAlignment: Text.AlignHCenter

            }
            LabelPL {
                text: qsTr("Thu")
                width: (parent.width - styler.themePaddingMedium) / 7
                horizontalAlignment: Text.AlignHCenter

            }
            LabelPL {
                text: qsTr("Fri")
                width: (parent.width - styler.themePaddingMedium) / 7
                horizontalAlignment: Text.AlignHCenter

            }
            LabelPL {
                text: qsTr("Sat")
                width: (parent.width - styler.themePaddingMedium) / 7
                horizontalAlignment: Text.AlignHCenter

            }
            LabelPL {
                text: qsTr("Sun")
                width: (parent.width - styler.themePaddingMedium) / 7
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    function init() {
        chkEnabled.checked = AmazfishConfig.alarmEnabled(alarmId);
        hour = AmazfishConfig.alarmHour(alarmId);
        minute = AmazfishConfig.alarmMinute(alarmId);

        var repeat = AmazfishConfig.alarmRepeatMask(alarmId);

        chkMo.checked = repeat & 1;
        chkTu.checked = repeat & 2;
        chkWe.checked = repeat & 4;
        chkTh.checked = repeat & 8;
        chkFr.checked = repeat & 16;
        chkSa.checked = repeat & 32;
        chkSu.checked = repeat & 64;

    }

    function save() {
        AmazfishConfig.setAlarmEnabled(alarmId, chkEnabled.checked)
        AmazfishConfig.setAlarmHour(alarmId, hour);
        AmazfishConfig.setAlarmMinute(alarmId, minute);

        var repeat = 0;
        repeat |= chkMo.checked ? 1 : 0;
        repeat |= chkTu.checked ? 2 : 0;
        repeat |= chkWe.checked ? 4 : 0;
        repeat |= chkTh.checked ? 8 : 0;
        repeat |= chkFr.checked ? 16 : 0;
        repeat |= chkSa.checked ? 32 : 0;
        repeat |= chkSu.checked ? 64 : 0;

        AmazfishConfig.setAlarmRepeatMask(alarmId, repeat);
    }


    function pad(n, width, z) {
        z = z || '0';
        n = n + '';
        return n.length >= width ? n : new Array(width - n.length + 1).join(z) + n;
    }
}
