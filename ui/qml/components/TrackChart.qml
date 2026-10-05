import QtQuick 2.0
import "./platform"
import "ChartTools.js" as ChartTools

// Line and area chart over a workout, x in seconds since the start.
Item {
    id: chart

    property var points: []
    property color color: styler.chartHeartRateColor
    property bool fill: true
    property bool invertY: false         // pace: faster (smaller) values at the top
    property var zoneLimits: []          // optional ascending limits, coloured like styler.chartZoneColors
    property real referenceValue: 0      // optional dashed reference line (e.g. average)
    // one decimal when the grid steps are smaller than 1 (e.g. 117.5 m), otherwise whole numbers
    property var valueLabel: function(v) { return priv.stepSize < 1 ? v.toFixed(1) : Math.round(v).toString(); }

    readonly property bool noData: priv.count < 2

    width: parent ? parent.width : 0
    height: styler.themeItemSizeLarge * 2.4

    onPointsChanged: { priv.prepare(); canvas.requestPaint(); }
    onReferenceValueChanged: canvas.requestPaint()
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    QtObject {
        id: priv
        property int count: 0
        property real minY: 0
        property real maxY: 1
        property real maxX: 1
        property real stepSize: 1

        // smallest "nice" step that is >= raw
        function niceStep(raw) {
            if (raw <= 0) return 1;
            var mag = Math.pow(10, Math.floor(Math.log(raw) / Math.LN10));
            var nice = [1, 2, 2.5, 3, 4, 5, 6, 8, 10];
            for (var i = 0; i < nice.length; i++) {
                if (nice[i] * mag >= raw * 0.9999) return nice[i] * mag;
            }
            return 10 * mag;
        }

        function prepare() {
            var list = chart.points || [];
            var lo = Number.MAX_VALUE, hi = -Number.MAX_VALUE, mx = 0, c = 0;
            for (var i = 0; i < list.length; i++) {
                var v = list[i].y;
                if (!isFinite(v)) continue;
                if (v < lo) lo = v;
                if (v > hi) hi = v;
                if (list[i].x > mx) mx = list[i].x;
                c++;
            }
            count = c;
            if (!c) return;
            if (hi - lo < 1e-6) { lo -= 1; hi += 1; }
            // four grid steps that cover [lo, hi] with round numbers
            var step = niceStep((hi - lo) / 4);
            for (var guard = 0; guard < 20; guard++) {
                minY = Math.floor(lo / step) * step;
                maxY = minY + step * 4;
                if (maxY >= hi) break;
                step = niceStep(step * 1.01);
            }
            stepSize = step;
            maxX = Math.max(1, mx);
        }
    }

    // short workouts get m:ss labels, otherwise whole minutes would repeat ("0' 0' 1'")
    function formatElapsed(sec) {
        sec = Math.round(sec);
        var h = Math.floor(sec / 3600);
        var m = Math.floor((sec % 3600) / 60);
        var s = sec % 60;
        if (priv.maxX < 600) return m + ":" + (s < 10 ? "0" : "") + s;
        return h > 0 ? h + ":" + (m < 10 ? "0" : "") + m : m + "'";
    }

    ChartCanvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            var fontPx = styler.themeFontSizeExtraSmall;
            ctx.font = ChartTools.canvasFont(fontPx, styler.themeFontFamily, false);
            // wide enough for the longest axis label ("119.2", "35:00")
            var axisW = Math.max(ctx.measureText(valueLabel(priv.minY)).width,
                                 ctx.measureText(valueLabel(priv.maxY)).width) + fontPx * 0.8;
            var bottom = fontPx * 1.8;
            var top = fontPx * 0.6;
            var plotW = width - axisW;
            var plotH = height - bottom - top;

            if (noData) {
                ctx.fillStyle = styler.themeSecondaryColor;
                ctx.textAlign = "center";
                ctx.fillText(qsTr("No data"), width / 2, height / 2);
                return;
            }

            var span = priv.maxY - priv.minY;
            function yOf(v) {
                var f = (Math.max(priv.minY, Math.min(priv.maxY, v)) - priv.minY) / span;
                return invertY ? top + f * plotH : top + plotH - f * plotH;
            }
            function xOf(x) { return axisW + x / priv.maxX * plotW; }

            // zone bands
            var lower = priv.minY;
            for (var z = 0; z < zoneLimits.length && z < styler.chartZoneColors.length; z++) {
                var upper = Math.min(priv.maxY, zoneLimits[z]);
                if (z === zoneLimits.length - 1) upper = priv.maxY;
                if (upper > lower) {
                    ctx.fillStyle = ChartTools.withAlpha(styler.chartZoneColors[z], styler.chartBandOpacity);
                    var ya = yOf(upper), yb = yOf(lower);
                    ctx.fillRect(axisW, Math.min(ya, yb), plotW, Math.abs(yb - ya));
                }
                lower = Math.max(lower, upper);
            }

            // grid and axes
            ctx.lineWidth = styler.chartGridLineWidth;
            ctx.strokeStyle = styler.chartGridColor;
            ctx.fillStyle = styler.themeSecondaryColor;
            for (var g = 0; g <= 4; g++) {
                var val = priv.minY + span * g / 4;
                var gy = Math.round(yOf(val)) + 0.5;
                ctx.beginPath(); ctx.moveTo(axisW, gy); ctx.lineTo(width, gy); ctx.stroke();
                ctx.textAlign = "right";
                ctx.fillText(valueLabel(val), axisW - fontPx * 0.4, gy + fontPx * 0.35);
            }
            for (var t = 0; t <= 4; t++) {
                ctx.textAlign = t === 0 ? "left" : (t === 4 ? "right" : "center");
                ctx.fillText(formatElapsed(priv.maxX * t / 4), axisW + plotW * t / 4, height - fontPx * 0.3);
            }

            var list = points;
            var base = invertY ? top : top + plotH;
            if (fill) {
                ctx.beginPath();
                var started = false;
                for (var i = 0; i < list.length; i++) {
                    if (!isFinite(list[i].y)) continue;
                    if (!started) { ctx.moveTo(xOf(list[i].x), base); started = true; }
                    ctx.lineTo(xOf(list[i].x), yOf(list[i].y));
                }
                ctx.lineTo(xOf(list[list.length - 1].x), base);
                ctx.closePath();
                ctx.fillStyle = ChartTools.withAlpha(chart.color, styler.chartFillOpacity);
                ctx.fill();
            }

            ctx.beginPath();
            var first = true;
            for (i = 0; i < list.length; i++) {
                if (!isFinite(list[i].y)) continue;
                if (first) { ctx.moveTo(xOf(list[i].x), yOf(list[i].y)); first = false; }
                else ctx.lineTo(xOf(list[i].x), yOf(list[i].y));
            }
            ctx.strokeStyle = chart.color;
            ctx.lineWidth = Math.max(1.5, fontPx / 10);
            ctx.lineJoin = "round";
            ctx.stroke();

            if (referenceValue > 0) {
                ctx.strokeStyle = styler.themeSecondaryColor;
                ctx.lineWidth = styler.chartGridLineWidth;
                ChartTools.dashedLine(ctx, axisW, width, Math.round(yOf(referenceValue)) + 0.5, 3, 3);
            }
        }
    }
}
