import QtQuick 2.0
import "../components"
import "../components/platform"

ListItemPL {
    id: item
    contentHeight: styler.themeItemSizeLarge

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
            leftMargin: styler.themeHorizontalPageMargin
        }
        height: parent.height
        width: parent.height
        fillMode: Image.PreserveAspectFit
        source: model.icon
    }

    Text {
        id: text
        anchors {
            verticalCenter: parent.verticalCenter
            left: icon.right
            leftMargin: styler.themePaddingMedium
            right: parent.right
            rightMargin: styler.themeHorizontalPageMargin
        }
        font.pixelSize: styler.themeFontSizeLarge
        color: item.down ? styler.themeHighlightColor : styler.themePrimaryColor
        text: model.deviceType
    }
}
