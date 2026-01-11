import "../components"
import "../components/platform"
import QtQuick 2.0
import uk.co.piggz.amazfish 1.0

DialogListPL {

    id: dialog

    signal selected(variant deviceType)

    canAccept: false

    model: DeviceListModel {
    }

    delegate: ListItemPL {
        id: item

        contentHeight: styler.themeItemSizeLarge
        onClicked: {
            // console.log(index);
            selected({
                "deviceType": model.deviceType,
                "icon": model.icon,
                "auth": model.auth,
                "pattern": model.pattern
            });
            dialog.accepted();
        }

        Image {
            id: icon

            height: parent.height
            width: parent.height
            fillMode: Image.PreserveAspectFit
            source: model.icon

            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                leftMargin: styler.themeHorizontalPageMargin
            }

        }

        Text {
            id: text

            font.pixelSize: styler.themeFontSizeLarge
            color: item.down ? styler.themeHighlightColor : styler.themePrimaryColor
            text: model.deviceType

            anchors {
                verticalCenter: parent.verticalCenter
                left: icon.right
                leftMargin: styler.themePaddingMedium
                right: parent.right
                rightMargin: styler.themeHorizontalPageMargin
            }

        }

    }

}
