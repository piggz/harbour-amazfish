import QtQuick 2.0
import "./platform"

// Legend row: items = [{ color: "#...", label: "...", line: false }]
Flow {
    id: legend
    property var items: []

    width: parent ? parent.width : 0
    spacing: styler.themePaddingLarge

    Repeater {
        model: legend.items
        delegate: Row {
            spacing: styler.themePaddingSmall
            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: modelData.line ? styler.themeFontSizeExtraSmall * 1.4 : styler.themeFontSizeExtraSmall * 0.8
                height: modelData.line ? Math.max(2, styler.themeFontSizeExtraSmall / 6) : width
                radius: modelData.line ? height / 2 : width / 4
                color: modelData.color
            }
            LabelPL {
                text: modelData.label
                color: styler.themeSecondaryColor
                font.pixelSize: styler.themeFontSizeExtraSmall
            }
        }
    }
}
