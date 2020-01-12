import QtQuick 2.0
import Sailfish.Silica 1.0

Rectangle {
    id: item
    signal clicked
    property string icn: ""
    property string txt: ""
    radius: 10
    color: Theme.highlightDimmerColor
    border.color: Theme.secondaryColor
    border.width: 5
    width: parent.width
    height: width / 3

    Image {
        id: icon
        source: icn
        x: Theme.paddingMedium
        y: Theme.paddingMedium
        height: parent.height - Theme.paddingMedium * 2
        width: height
    }
    Text {
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: icon.right
        anchors.leftMargin: Theme.paddingMedium
        font.pixelSize: Theme.fontSizeLarge
        color: Theme.primaryColor
        text: txt
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            item.clicked()
        }
    }
}
