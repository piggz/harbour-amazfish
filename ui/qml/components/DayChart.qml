import QtQuick 2.0
import "./platform"
import "ChartTools.js" as ChartTools

// Values of one day, either as a line with area or as bars in fixed time slots.
// points: [{ x: seconds, y: value }], sorted by time
Item {
    id: chart

    property var points: []
    property var startTime: 0                // seconds, start of the day
    property int durationSec: 86400
    property bool bars: false
    property int slotSec: 1800               // bars: each bar shows the average of its slot
    property int gapSec: 2 * 3600            // line: longer gaps break the line
    property color color: styler.chartActiveColor
    property color fillColor             // line: area below it, none if not set
    property var colorFor: null              // bars: optional function(value) giving each bar its own colour
    property var guides: []                  // dashed lines: [{ value: 40, color: ... }]
    property real minY: 0
    property real maxY: 0                    // minY and maxY 0: fitted to the values
    property var valueLabel: function(v) { return priv.step < 1 ? v.toFixed(1) : Math.round(v).toString(); }

    readonly property bool noData: priv.count === 0
    readonly property real average: priv.average
    readonly property real minimum: priv.minimum
    readonly property real maximum: priv.maximum
    readonly property real last: priv.last

    width: parent ? parent.width : 0
    height: styler.themeItemSizeLarge * 3

    onPointsChanged: { priv.prepare(); canvas.requestPaint(); }
    onMinYChanged: { priv.prepare(); canvas.requestPaint(); }
    onMaxYChanged: { priv.prepare(); canvas.requestPaint(); }
    onStartTimeChanged: { priv.prepare(); canvas.requestPaint(); }
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    QtObject {
        id: priv
        property int count: 0
        property real average: 0
        property real minimum: 0
        property real maximum: 0
        property real last: 0
        property real lowY: 0
        property real highY: 1
        property real step: 1
        property var slots: []

        function niceStep(raw) {
            var mag = Math.pow(10, Math.floor(Math.log(raw) / Math.LN10));
            var nice = [1, 2, 2.5, 5, 10];
            for (var i = 0; i < nice.length; i++) {
                if (nice[i] * mag >= raw) return nice[i] * mag;
            }
            return 10 * mag;
        }

        function prepare() {
            var list = chart.points || [];
            var c = 0, sum = 0, lo = 0, hi = 0, l = 0;
            var n = Math.max(1, Math.ceil(chart.durationSec / chart.slotSec));
            var sums = [], counts = [];
            for (var k = 0; k < n; k++) { sums.push(0); counts.push(0); }
            for (var i = 0; i < list.length; i++) {
                var p = list[i];
                if (p.x < chart.startTime || p.x >= chart.startTime + chart.durationSec) continue;
                if (c === 0 || p.y < lo) lo = p.y;
                if (c === 0 || p.y > hi) hi = p.y;
                sum += p.y; c++; l = p.y;
                k = Math.floor((p.x - chart.startTime) / chart.slotSec);
                sums[k] += p.y; counts[k]++;
            }
            var s = [];
            for (k = 0; k < n; k++) s.push(counts[k] ? sums[k] / counts[k] : -1);
            slots = s;
            count = c; average = c ? sum / c : 0; minimum = lo; maximum = hi; last = l;

            if (chart.maxY > chart.minY) {
                lowY = chart.minY; highY = chart.maxY;
                step = (highY - lowY) / 4;
                return;
            }
            // fit four round grid steps around the values
            if (c === 0) { lo = 0; hi = 4; }
            var pad = Math.max((hi - lo) * 0.15, Math.abs(hi) * 0.02, 0.5);
            step = niceStep((hi - lo + 2 * pad) / 4);
            lowY = Math.floor((lo - pad) / step) * step;
            if (lo >= 0 && lowY < 0) lowY = 0;
            highY = lowY + 4 * step;
            while (highY < hi + pad * 0.5) {
                step = niceStep(step * 1.01);
                lowY = Math.floor((lo - pad) / step) * step;
                highY = lowY + 4 * step;
            }
        }
    }

    ChartCanvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            var fontPx = styler.themeFontSizeExtraSmall;
            ctx.font = ChartTools.canvasFont(fontPx, styler.themeFontFamily, false);
            var axisW = Math.max(ctx.measureText(valueLabel(priv.highY)).width,
                                 ctx.measureText(valueLabel(priv.lowY)).width) + fontPx * 0.8;
            var bottom = fontPx * 1.8;
            var top = fontPx * 0.6;
            var plotW = width - axisW;
            var plotH = height - bottom - top;
            if (plotW <= 0 || plotH <= 0) return;
            var lowY = priv.lowY, span = priv.highY - priv.lowY;

            function yOf(v) { return top + plotH - (Math.max(lowY, Math.min(priv.highY, v)) - lowY) / span * plotH; }
            function xOf(t) { return axisW + (t - startTime) / durationSec * plotW; }

            ctx.lineWidth = styler.chartGridLineWidth;
            ctx.strokeStyle = styler.chartGridColor;
            ctx.fillStyle = styler.themeSecondaryColor;
            ctx.textAlign = "right";
            for (var g = 0; g <= 4; g++) {
                var v = lowY + span * g / 4;
                var gy = Math.round(yOf(v)) + 0.5;
                ctx.beginPath(); ctx.moveTo(axisW, gy); ctx.lineTo(width, gy); ctx.stroke();
                ctx.fillText(valueLabel(v), axisW - fontPx * 0.4, gy + fontPx * 0.35);
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

            var base = top + plotH;
            if (bars) {
                var slots = priv.slots;
                var slotW = plotW / slots.length;
                var bw = Math.max(1, slotW * 0.7);
                var radius = Math.min(bw / 3, fontPx * 0.3);
                for (var i = 0; i < slots.length; i++) {
                    if (slots[i] < 0) continue;
                    var h = Math.max(1.5, base - yOf(slots[i]));
                    var x = axisW + i * slotW + (slotW - bw) / 2;
                    ctx.fillStyle = colorFor ? colorFor(slots[i]) : chart.color;
                    ctx.beginPath();
                    ctx.roundedRect(x, base - h, bw, h, radius, radius);
                    ctx.fill();
                    if (h > radius) ctx.fillRect(x, base - radius, bw, radius);
                }
            } else {
                // segments, broken on long gaps; a lone value is drawn as a dot
                var segments = [], seg = [];
                for (i = 0; i < points.length; i++) {
                    var p = points[i];
                    if (p.x < startTime || p.x >= startTime + durationSec) continue;
                    if (seg.length && p.x - seg[seg.length - 1].x > gapSec) { segments.push(seg); seg = []; }
                    seg.push(p);
                }
                if (seg.length) segments.push(seg);

                for (var s = 0; s < segments.length; s++) {
                    var sg = segments[s];
                    if (sg.length === 1) {
                        ctx.fillStyle = chart.color;
                        ctx.beginPath();
                        ctx.arc(xOf(sg[0].x), yOf(sg[0].y), styler.chartLineWidth * 1.5, 0, 2 * Math.PI);
                        ctx.fill();
                        continue;
                    }
                    if (fillColor.a > 0) {
                        ctx.beginPath();
                        ctx.moveTo(xOf(sg[0].x), base);
                        for (i = 0; i < sg.length; i++) ctx.lineTo(xOf(sg[i].x), yOf(sg[i].y));
                        ctx.lineTo(xOf(sg[sg.length - 1].x), base);
                        ctx.closePath();
                        ctx.fillStyle = fillColor;
                        ctx.fill();
                    }

                    ctx.beginPath();
                    for (i = 0; i < sg.length; i++) {
                        if (i === 0) ctx.moveTo(xOf(sg[i].x), yOf(sg[i].y));
                        else ctx.lineTo(xOf(sg[i].x), yOf(sg[i].y));
                    }
                    ctx.strokeStyle = chart.color;
                    ctx.lineWidth = styler.chartLineWidth;
                    ctx.lineJoin = "round";
                    ctx.stroke();
                }
            }

            ctx.lineWidth = styler.chartGuideLineWidth;
            for (var gi = 0; gi < guides.length; gi++) {
                ctx.strokeStyle = guides[gi].color;
                var ty = Math.round(yOf(guides[gi].value)) + 0.5;
                ChartTools.dashedLine(ctx, axisW, width, ty, styler.chartDashLength, styler.chartDashLength);
            }
        }
    }
}
