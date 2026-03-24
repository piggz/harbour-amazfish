import QtQuick 2.0
import QtQuick.Layouts 1.1
import "./platform"

Item {
    id: itm

    property string text: ""
    property alias contentItem: content.children
    property alias actionItem: action.children
    property int size

    signal clicked()

    Layout.preferredHeight: size
    Layout.preferredWidth: size

    Rectangle {
        color: styler.themeSecondaryHighlightColor
        anchors.fill: parent
        anchors.margins: styler.themePaddingSmall

        Item {
            id: content
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: row.top
            anchors.margins: styler.themePaddingSmall
        }

        RowLayout {
            id: row
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: styler.themePaddingSmall
            height: styler.themeIconSizeMedium

            LabelPL {
                id: lbl
                text: itm.text
                color: styler.blockBg
                Layout.fillWidth: true
                font.pixelSize: styler.themeFontSizeLarge
                Layout.alignment: Qt.AlignBottom
                truncMode: truncModes.elide
            }
            Item {
                id: action
                width: styler.themeIconSizeMedium
                height: styler.themeIconSizeMedium
                z: 10
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                itm.clicked();
            }
        }
    }
}


