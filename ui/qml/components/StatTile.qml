import QtQuick 2.0
import "./platform"

// Value with unit and a label below; value and unit shrink together to fit the width.
Column {
    id: tile
    property string value: "-"
    property string unit: ""
    property string label: ""

    readonly property real valueSize: styler.themeFontSizeLarge
    readonly property real unitSize: styler.themeFontSizeSmall
    readonly property real minimumScale: 0.55

    // width at full size, measured by hidden texts so the scaling has no binding loop
    readonly property real naturalWidth: measureValue.implicitWidth
                                         + (unit !== "" ? styler.themePaddingSmall + measureUnit.implicitWidth : 0)
    readonly property real fitScale: naturalWidth > 0 && width > 0
                                  ? Math.max(minimumScale, Math.min(1, width / naturalWidth)) : 1
    // set by a surrounding grid so that all values of a group share one size
    property real sharedScale: -1
    readonly property real appliedScale: sharedScale > 0 ? sharedScale : fitScale

    spacing: 0

    Text {
        id: measureValue
        visible: false
        text: tile.value
        font.pixelSize: tile.valueSize
        font.family: styler.themeFontFamily
    }
    Text {
        id: measureUnit
        visible: false
        text: tile.unit
        font.pixelSize: tile.unitSize
        font.family: styler.themeFontFamily
    }

    Row {
        width: tile.width
        spacing: styler.themePaddingSmall * tile.appliedScale

        LabelPL {
            id: lblValue
            text: tile.value
            color: styler.themeHighlightColor
            font.pixelSize: tile.valueSize * tile.appliedScale
        }
        LabelPL {
            anchors.baseline: lblValue.baseline
            text: tile.unit
            visible: tile.unit !== ""
            color: styler.themeSecondaryHighlightColor
            font.pixelSize: tile.unitSize * tile.appliedScale
        }
    }
    LabelPL {
        width: tile.width
        text: tile.label
        color: styler.themeSecondaryColor
        font.pixelSize: styler.themeFontSizeExtraSmall
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        elide: Text.ElideRight
    }
}
