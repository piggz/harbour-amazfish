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

        transform: Rotation {
            id: transf
            origin.x: width/2 - styler.themePaddingSmall
            origin.y: height/2 - styler.themePaddingSmall
            axis { x: 0; y: 1; z: 0 }
            angle: mouse.pressed ? 180 : 0
            Behavior on angle {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Item {
            id: content
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: row.top
            anchors.margins: styler.themePaddingSmall
            visible: transf.angle < 90
        }

        RowLayout {
            id: row
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: styler.themePaddingSmall
            visible: transf.angle < 90
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
                width: children.length ? styler.themeIconSizeMedium : 0
                height: styler.themeIconSizeMedium
                z: 10
            }
        }

        MouseArea {
            id: mouse
            anchors.fill: parent
            onClicked: {
                itm.clicked();
            }
        }
    }
}


