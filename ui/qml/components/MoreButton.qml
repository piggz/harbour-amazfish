import QtQuick 2.0
import Sailfish.Silica 1.0

BackgroundItem {
    property alias text: label.text
    property alias textAlignment: label.horizontalAlignment
    property int depth: 0
    readonly property color _color: enabled ? highlighted ? Theme.highlightColor : Theme.primaryColor : Theme.secondaryColor

    height: Theme.itemSizeSmall

    Label {
        id: label
        anchors {
            left: parent.left
            right: image.left
            verticalCenter: parent.verticalCenter
            leftMargin: Theme.horizontalPageMargin + depth * Theme.paddingLarge
            rightMargin: Theme.paddingMedium
        }
        horizontalAlignment: Text.AlignRight
        truncationMode: TruncationMode.Fade
        color: _color
    }

    Image {
        id: image
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: Theme.horizontalPageMargin
        }
        source: "image://theme/icon-m-right?" + _color
    }
}
