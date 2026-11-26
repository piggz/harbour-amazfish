import QtQuick 2.0
import "./platform"

// Label and value row like the Silica DetailItem; both sides wrap instead of being cut off.
Item {
    id: row
    property string label
    property string value

    width: parent ? parent.width : 0
    height: Math.max(lbl.height, val.height)

    // labels tend to be longer than values: give them a bit more than half
    Item {
        id: splitter
        x: row.width * 0.55
        width: 0
        height: 0
    }

    LabelPL {
        id: lbl
        anchors.left: parent.left
        anchors.right: splitter.left
        anchors.rightMargin: styler.themePaddingMedium
        horizontalAlignment: Text.AlignRight
        text: row.label
        color: styler.themeSecondaryHighlightColor
        font.pixelSize: styler.themeFontSizeSmall
        // A single word longer than the column breaks inside the word.
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        maximumLineCount: 3
        elide: Text.ElideRight
    }
    LabelPL {
        id: val
        anchors.left: splitter.right
        anchors.leftMargin: styler.themePaddingMedium
        anchors.right: parent.right
        text: row.value
        color: styler.themeHighlightColor
        font.pixelSize: styler.themeFontSizeSmall
        wrapMode: Text.WordWrap
        maximumLineCount: 3
        elide: Text.ElideRight
    }
}
