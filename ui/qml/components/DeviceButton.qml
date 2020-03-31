import QtQuick 2.0
import Sailfish.Silica 1.0

ListItem {
    id: item
    width: parent.width
    contentHeight: Theme.itemSizeHuge

    onClicked: {
        var props = {deviceType: model.deviceType}

        if (model.auth) {
            pageStack.push(Qt.resolvedUrl("../pages/AuthKeyDialog.qml"), {
                               acceptDestination: Qt.resolvedUrl("../pages/PairPage.qml"),
                               acceptDestinationProperties: props
                           })
        } else {
            pageStack.push(Qt.resolvedUrl("../pages/PairPage.qml"), props)
        }
    }

    Image {
        id: icon
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: Theme.horizontalPageMargin
        }
        height: Theme.iconSizeExtraLarge
        width: Theme.iconSizeExtraLarge
        fillMode: Image.PreserveAspectFit
        source: model.icon
    }

    Text {
        id: text
        anchors {
            verticalCenter: parent.verticalCenter
            left: icon.right
            leftMargin: Theme.paddingMedium
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
        }
        font.pixelSize: Theme.fontSizeLarge
        color: item.down ? Theme.highlightColor : Theme.primaryColor
        text: model.deviceType
    }
}
