import QtQuick 2.0
import "./platform"
import "ChartColors.js" as ChartColors

// Rounded card that holds one chart, in the style of Gadgetbridge's chart cards.
Item {
    id: card

    property string title: ""
    property string info: ""        // short text shown on the right of the title
    property real padding: styler.themePaddingLarge
    default property alias content: body.data

    signal clicked

    width: parent ? parent.width : 0
    height: column.height + 2 * padding

    Rectangle {
        anchors.fill: parent
        radius: styler.themePaddingLarge
        color: ChartColors.withAlpha(styler.themeHighlightColor, 0.08)
    }

    MouseArea {
        id: cardMouse
        anchors.fill: parent
        onClicked: card.clicked()
    }

    Column {
        id: column
        x: card.padding
        y: card.padding
        width: card.width - 2 * card.padding
        spacing: styler.themePaddingMedium

        Item {
            width: parent.width
            height: Math.max(lblTitle.height, lblInfo.height)
            visible: card.title !== "" || card.info !== ""

            LabelPL {
                id: lblTitle
                anchors.left: parent.left
                anchors.right: lblInfo.left
                anchors.rightMargin: styler.themePaddingMedium
                text: card.title.toUpperCase()
                color: styler.themeHighlightColor
                opacity: 0.85
                font.pixelSize: styler.themeFontSizeExtraSmall
                font.bold: true
                font.letterSpacing: 1
                truncMode: truncModes.fade
            }
            LabelPL {
                id: lblInfo
                anchors.right: parent.right
                text: card.info
                color: styler.themeSecondaryColor
                font.pixelSize: styler.themeFontSizeExtraSmall
            }
        }

        Column {
            id: body
            width: parent.width
            spacing: styler.themePaddingMedium
        }
    }
}
