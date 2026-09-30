import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import "./platform"
import "GlassStyle.js" as Glass

// Glass tile of the first page: icon top left, optional value and the label at the
// bottom, optional action button top right. Tiles with custom content (steps, PAI)
// set contentItem instead of iconSource.
Item {
    id: itm

    property string text: ""
    property string value: ""
    property string iconSource: ""
    property color iconColor: styler.themeHighlightColor
    property alias contentItem: content.children
    property alias actionItem: action.children
    property int size: pageGrid.prefWidth(this)

    signal clicked()

    Layout.preferredHeight: size
    Layout.preferredWidth: size

    GlassPanel {
        id: glass
        anchors.fill: parent
        anchors.margins: styler.themePaddingSmall
        pressed: mouse.pressed

        MouseArea {
            id: mouse
            anchors.fill: parent
            onClicked: itm.clicked()
        }

        Item {
            id: iconBox
            visible: itm.iconSource !== ""
            x: styler.themePaddingMedium
            y: styler.themePaddingMedium
            width: styler.themeIconSizeMedium * 0.8
            height: width

            Image {
                id: icon
                anchors.fill: parent
                source: itm.iconSource
                sourceSize.width: width
                sourceSize.height: height
                fillMode: Image.PreserveAspectFit
                visible: false
            }
            // The page icons are white at 50 % alpha (made for coloured tiles); two
            // tinted copies give ~75 % coverage so they read on dark glass.
            Repeater {
                model: 2
                ColorOverlay {
                    anchors.fill: icon
                    source: icon
                    color: itm.iconColor
                    cached: true
                }
            }
        }

        Item {
            id: content
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: labels.top
            anchors.margins: styler.themePaddingMedium
        }

        Column {
            id: labels
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: styler.themePaddingMedium

            LabelPL {
                width: parent.width
                visible: itm.value !== ""
                text: itm.value
                color: styler.themePrimaryColor
                font.pixelSize: styler.themeFontSizeLarge
                font.bold: true
                truncMode: truncModes.fade
            }
            LabelPL {
                id: lbl
                width: parent.width
                text: itm.text
                color: styler.themeSecondaryColor
                font.pixelSize: styler.themeFontSizeSmall
                truncMode: truncModes.elide
            }
        }

        // action (e.g. measure heart rate) in a small glass circle, top right.
        // The circle is a sibling because assigning actionItem replaces action's children.
        Rectangle {
            anchors.fill: action
            radius: width / 2
            color: Glass.chip
            visible: action.children.length > 0
        }
        Item {
            id: action
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: styler.themePaddingSmall
            width: children.length ? styler.themeIconSizeMedium : 0
            height: styler.themeIconSizeMedium
            z: 10
        }
    }
}
