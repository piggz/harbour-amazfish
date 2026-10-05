import QtQuick 2.0
import "./platform"
import "ChartTools.js" as ChartTools

// Heart rate of a day as area and line on zone bands, with a dashed resting line.
Item {
    id: chart

    property var points: []
    property var startTime: 0
    property int durationSec: 86400
    property real minY: 40
    property real maxY: 200
    property real restingHeartrate: 0
    property var zoneLimits: []          // ascending upper limits in bpm, one per styler.chartZoneColors entry

    readonly property bool noData: !points || points.length === 0

    width: parent ? parent.width : 0
    height: styler.themeItemSizeLarge * 3

    onPointsChanged: canvas.requestPaint()
    onRestingHeartrateChanged: canvas.requestPaint()
    onZoneLimitsChanged: canvas.requestPaint()
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    ChartCanvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            var fontPx = styler.themeFontSizeExtraSmall;
            ctx.font = ChartTools.canvasFont(fontPx, styler.themeFontFamily, false);
            var axisW = fontPx * 2.4;
            var bottom = fontPx * 1.8;
            var top = fontPx * 0.6;
            var plotW = width - axisW;
            var plotH = height - bottom - top;
            var span = maxY - minY;

            function yOf(v) { return top + plotH - (Math.max(minY, Math.min(maxY, v)) - minY) / span * plotH; }
            function xOf(t) { return axisW + (t - startTime) / durationSec * plotW; }

            // zone bands
            var lower = minY;
            for (var z = 0; z < zoneLimits.length && z < styler.chartZoneColors.length; z++) {
                var upper = z === zoneLimits.length - 1 ? maxY : zoneLimits[z];
                if (upper > lower) {
                    ctx.fillStyle = ChartTools.withAlpha(styler.chartZoneColors[z], styler.chartBandOpacity);
                    ctx.fillRect(axisW, yOf(upper), plotW, yOf(lower) - yOf(upper));
                }
                lower = upper;
            }

            ctx.lineWidth = styler.chartGridLineWidth;
            ctx.strokeStyle = styler.chartGridColor;
            ctx.fillStyle = styler.themeSecondaryColor;
            ctx.textAlign = "right";
            for (var g = 0; g <= 4; g++) {
                var v = minY + span * g / 4;
                var gy = Math.round(yOf(v)) + 0.5;
                ctx.beginPath(); ctx.moveTo(axisW, gy); ctx.lineTo(width, gy); ctx.stroke();
                ctx.fillText(Math.round(v), axisW - fontPx * 0.4, gy + fontPx * 0.35);
            }
            for (var t = 0; t <= 4; t++) {
                ctx.textAlign = t === 0 ? "left" : (t === 4 ? "right" : "center");
                ctx.fillText(Qt.formatTime(new Date((startTime + durationSec * t / 4) * 1000), "hh:mm"),
                             axisW + plotW * t / 4, height - fontPx * 0.3);
            }

            if (noData) {
                ctx.textAlign = "center";
                ctx.fillText(qsTr("No data"), axisW + plotW / 2, top + plotH / 2);
                return;
            }

            // build segments, broken on gaps longer than 15 minutes
            var segments = [], seg = [];
            for (var i = 0; i < points.length; i++) {
                var p = points[i];
                if (p.y <= 0) continue;
                if (seg.length && p.x - seg[seg.length - 1].x > 15 * 60) { segments.push(seg); seg = []; }
                seg.push(p);
            }
            if (seg.length) segments.push(seg);

            var base = top + plotH;
            for (var s = 0; s < segments.length; s++) {
                var sg = segments[s];
                ctx.beginPath();
                ctx.moveTo(xOf(sg[0].x), base);
                for (i = 0; i < sg.length; i++) ctx.lineTo(xOf(sg[i].x), yOf(sg[i].y));
                ctx.lineTo(xOf(sg[sg.length - 1].x), base);
                ctx.closePath();
                ctx.fillStyle = ChartTools.withAlpha(styler.chartHeartRateColor, styler.chartFillOpacity);
                ctx.fill();

                ctx.beginPath();
                for (i = 0; i < sg.length; i++) {
                    if (i === 0) ctx.moveTo(xOf(sg[i].x), yOf(sg[i].y));
                    else ctx.lineTo(xOf(sg[i].x), yOf(sg[i].y));
                }
                ctx.strokeStyle = styler.chartHeartRateColor;
                ctx.lineWidth = styler.chartLineWidth;
                ctx.lineJoin = "round";
                ctx.stroke();
            }

            if (restingHeartrate > 0) {
                ctx.strokeStyle = styler.chartRestingHeartRateColor;
                ctx.lineWidth = styler.chartLineWidth;
                var ry = Math.round(yOf(restingHeartrate)) + 0.5;
                ChartTools.dashedLine(ctx, axisW, width, ry, fontPx * 0.4, fontPx * 0.3);
                ctx.fillStyle = styler.chartRestingHeartRateColor;
                ctx.textAlign = "right";
                ctx.fillText(qsTr("Resting %1").arg(Math.round(restingHeartrate)), width, ry - fontPx * 0.4);
            }
        }
    }
}
