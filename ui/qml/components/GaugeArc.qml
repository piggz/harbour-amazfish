import QtQuick 2.0
import "ChartColors.js" as ChartColors

// 270° progress gauge as used on Gadgetbridge's dashboard. Children are centred.
Item {
    id: gauge

    property real value: 0              // 0 .. 1 (values above 1 are clamped)
    property color color: ChartColors.active
    property color trackColor: ChartColors.withAlpha(styler.themeSecondaryColor, 0.25)
    property real lineWidth: width * 0.09
    default property alias content: centre.data

    implicitWidth: styler.themeItemSizeLarge * 1.6
    implicitHeight: implicitWidth

    onValueChanged: canvas.requestPaint()
    onColorChanged: canvas.requestPaint()
    onTrackColorChanged: canvas.requestPaint()

    ChartCanvas {
        id: canvas
        anchors.fill: parent
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            var r = Math.min(width, height) / 2 - gauge.lineWidth / 2;
            // no size yet (e.g. while the page was hidden): arc() would throw on a
            // negative radius and abort painting; the size change repaints later
            if (r <= 0) {
                return;
            }
            var cx = width / 2, cy = height / 2;
            var a0 = Math.PI * 0.75, sweep = Math.PI * 1.5;

            ctx.lineCap = "round";
            ctx.lineWidth = gauge.lineWidth;

            ctx.strokeStyle = gauge.trackColor;
            ctx.beginPath();
            ctx.arc(cx, cy, r, a0, a0 + sweep, false);
            ctx.stroke();

            var v = Math.max(0, Math.min(1, gauge.value));
            if (v > 0) {
                ctx.strokeStyle = gauge.color;
                ctx.beginPath();
                ctx.arc(cx, cy, r, a0, a0 + sweep * v, false);
                ctx.stroke();
            }
        }
    }

    Item {
        id: centre
        anchors.fill: parent
    }
}
