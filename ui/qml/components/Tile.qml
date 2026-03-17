import QtQuick 2.0
import QtQuick.Layouts 1.1
import "./platform"

Item {
    id: itm

    property string text: ""
    property alias contentItem: content.children

    signal clicked()

    height: width

    Rectangle {
        color: styler.themeSecondaryHighlightColor
        anchors.fill: parent
        anchors.margins: 4

        Item {
            id: content
            anchors.fill: parent
            anchors.margins: 8
        }

        LabelPL {
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


