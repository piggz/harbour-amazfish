import QtQuick 2.0
import QtQuick.Layouts 1.1
import "./platform"

RowLayout {
    id: dateNavigation
    spacing: styler.themePaddingLarge
    width: parent.width
    property string text: ""
    signal backward
    signal forward

    IconButtonPL {
        id: btnPrev
        iconName: "icon-m-back"
        onClicked: {
            backward();
        }
    }
    LabelPL {
        id: lblDay
        Layout.fillWidth: true
        text: dateNavigation.text
        height: btnPrev.height
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    IconButtonPL {
        id: btnNext
        iconName: "icon-m-forward"
        onClicked: {
            forward();
        }
    }
}
