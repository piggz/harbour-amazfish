import QtQuick 2.0
import QtQuick.Layouts 1.1
import "./platform"

RowLayout {
    id: dateNavigation
    spacing: styler.themePaddingLarge
    width: parent.width
    property date day
    signal backward
    signal forward

    GlassPanel {
        round: true
        Layout.preferredWidth: styler.themeItemSizeSmall
        Layout.preferredHeight: styler.themeItemSizeSmall

        IconButtonPL {
            id: btnPrev
            anchors.centerIn: parent
            iconName: styler.iconBackward
            iconHeight: styler.themeIconSizeSmall
            iconWidth: styler.themeIconSizeSmall
            onClicked: {
                backward();
            }
        }
    }

    ButtonPL {
        id: lblDay
        Layout.fillWidth: true

        // width: parent.width - lblDay.width - styler.themePaddingLarge
        height: styler.themeItemSizeSmall

        text: day.toLocaleDateString();

        onClicked: {
            var datearg = new Date(day);
            var dialog = app.pages.push(pickerComponent, {
                                            date: !isNaN(datearg) ? datearg : new Date()
                                        })
            dialog.accepted.connect(function() {
                dateNavigation.day = dialog.date;
                // console.log(dateNavigation.day)
            })
        }

        Component {
            id: pickerComponent
            DatePickerDialogPL {}
        }
    }

    GlassPanel {
        round: true
        Layout.preferredWidth: styler.themeItemSizeSmall
        Layout.preferredHeight: styler.themeItemSizeSmall

        IconButtonPL {
            id: btnNext
            anchors.centerIn: parent
            iconName: styler.iconForward
            iconHeight: styler.themeIconSizeSmall
            iconWidth: styler.themeIconSizeSmall
            onClicked: {
                forward();
            }
        }
    }
}
