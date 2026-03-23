import QtQuick 2.0
import QtQuick.Layouts 1.1
import "./platform"

Item {
    id: itm

    property string text: ""
    property alias contentItem: content.children
    property alias actionItem: action.children
    property int actionSize: styler.themeFontSizeLarge

    signal clicked()

    height: width

    Rectangle {
        color: styler.themeSecondaryHighlightColor
        anchors.fill: parent
        anchors.margins: 4

        Item {
            id: content
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: lbl.top
            anchors.margins: 8
        }

        Item {
            id: action
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 2
            width: actionSize
            height: actionSize
            z: 10
        }

        LabelPL {
            id: lbl
            text: itm.text
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            color: styler.blockBg
            font.pixelSize: styler.themeFontSizeLarge
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                itm.clicked();
            }
        }
    }
}


