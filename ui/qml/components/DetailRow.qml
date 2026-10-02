import QtQuick 2.0
import "./platform"

// Label / value row, laid out like Silica's DetailItem so it fits every platform.
// Both sides wrap onto further lines instead of being cut off, so long labels
// ("Durchschnittliche Schrittlänge") and values fit narrow cards.
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
        // wrap at words; only a single word longer than the column breaks inside
        // the word instead of running out of the card
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
