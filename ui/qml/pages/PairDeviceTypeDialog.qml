import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

DialogPL {
    id: dialog;
    canAccept: false
    signal selected(variant deviceType);

    ListViewPL {
        id: listView
        anchors.fill: parent
        model: DeviceListModel {
        }
        delegate: ListItemPL {
            id: item
            contentHeight: styler.themeItemSizeLarge


            onClicked: {
                console.log(index)
                selected({
                             deviceType: model.deviceType,
                             icon: model.icon,
                             auth: model.auth,
                             pattern: model.pattern
                         })
                dialog.accepted()
                // selected(model.auth, model.deviceType);
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
    }

}
