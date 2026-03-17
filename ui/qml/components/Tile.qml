import QtQuick 2.0
import QtQuick.Layouts 1.1
import "./platform"

Rectangle {
    property string text: ""
    property alias contentItem: content.children

    signal clicked()

    color: styler.themeSecondaryHighlightColor
    height: width

    Item {
        id: content
        anchors.fill: parent
        anchors.margins: 8
    }

    LabelPL {
        text: parent.text
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        color: styler.blockBg
        font.pixelSize: styler.themeFontSizeLarge
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            parent.clicked();
        }
    }
}
