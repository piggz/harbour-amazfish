import QtQuick 2.0
import "./platform"
import "ChartTools.js" as ChartTools

// Percentage over several days in fixed time slots; each bar shows the last value of its slot.
// Slots without a value stay empty.
Item {
    id: chart

    property var points: []
    property var startTime: 0            // seconds, first slot starts here
    property var endTime: 0              // seconds, last slot ends here
    property int slotSec: 6 * 3600
    property var colorFor: function(percent) {
        return percent >= 50 ? styler.chartBatteryHighColor
                             : percent >= 20 ? styler.chartBatteryMediumColor : styler.chartBatteryLowColor;
    }
    // faint guide lines where the colour changes
    property var guides: [
        { value: 20, color: styler.chartBatteryMediumGuideColor },
        { value: 50, color: styler.chartBatteryHighGuideColor }
    ]

    readonly property bool noData: priv.filled === 0
    readonly property real lowest: priv.lowest
    readonly property var lowestTime: priv.lowestTime
    readonly property var lastChargeTime: priv.lastChargeTime

    width: parent ? parent.width : 0
    height: styler.themeItemSizeLarge * 2.6

    onPointsChanged: { priv.bucket(); canvas.requestPaint(); }
    onStartTimeChanged: { priv.bucket(); canvas.requestPaint(); }
    onEndTimeChanged: { priv.bucket(); canvas.requestPaint(); }
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    QtObject {
        id: priv
        property var slots: []
        property int filled: 0
        property real lowest: -1
        property var lowestTime: 0
        property var lastChargeTime: 0

        function bucket() {
            var n = Math.max(1, Math.ceil((chart.endTime - chart.startTime) / chart.slotSec));
            var s = [];
            for (var i = 0; i < n; i++) s.push(-1);
            var list = chart.points || [];
            var lo = -1, loT = 0, chargeT = 0, prev = -1, f = 0;
            for (i = 0; i < list.length; i++) {
                var p = list[i];
                var v = Math.max(0, Math.min(100, p.y));
                // a rise of 5 points or more counts as charging
                if (prev >= 0 && v >= prev + 5) chargeT = p.x;
                prev = v;
                var k = Math.floor((p.x - chart.startTime) / chart.slotSec);
                if (k < 0 || k >= n) continue;
                if (s[k] < 0) f++;
                s[k] = v;                              // samples are sorted: last one wins
                if (lo < 0 || v < lo) { lo = v; loT = p.x; }
            }
            slots = s; filled = f; lowest = lo; lowestTime = loT; lastChargeTime = chargeT;
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
            // wide enough for the longest axis label ("100 %"), whatever the font
            var axisW = ctx.measureText("100 %").width + fontPx * 0.8;
            var bottom = fontPx * 1.8;
            var top = fontPx * 0.6;
            var plotW = width - axisW;
            var plotH = height - bottom - top;
            if (plotW <= 0 || plotH <= 0) return;

            function yOf(v) { return top + plotH - v / 100 * plotH; }

            // grid and axis
            ctx.lineWidth = styler.chartGridLineWidth;
            ctx.strokeStyle = styler.chartGridColor;
            ctx.fillStyle = styler.themeSecondaryColor;
            ctx.textAlign = "right";
            for (var g = 0; g <= 4; g++) {
                var gy = Math.round(yOf(g * 25)) + 0.5;
                ctx.beginPath(); ctx.moveTo(axisW, gy); ctx.lineTo(width, gy); ctx.stroke();
                ctx.fillText((g * 25) + " %", axisW - fontPx * 0.4, gy + fontPx * 0.35);
            }

            var slots = priv.slots;
            var n = slots.length;
            var slotW = plotW / n;
            var slotsPerDay = Math.round(86400 / slotSec);

            // day boundaries and weekday labels (every second day if space is tight)
            var days = Math.round(n / slotsPerDay);
            var labelEvery = plotW / days < fontPx * 2.6 ? 2 : 1;
            ctx.textAlign = "center";
            for (var d = 0; d < days; d++) {
                var dx = axisW + d * slotsPerDay * slotW;
                if (d > 0) {
                    ctx.strokeStyle = styler.chartGridFaintColor;
                    ctx.beginPath();
                    ctx.moveTo(Math.round(dx) + 0.5, top);
                    ctx.lineTo(Math.round(dx) + 0.5, top + plotH);
                    ctx.stroke();
                }
                var last = d === days - 1;
                if ((days - 1 - d) % labelEvery === 0) {
                    var dayDate = new Date((startTime + d * 86400 + 43200) * 1000);
                    ctx.fillStyle = last ? styler.themeHighlightColor : styler.themeSecondaryColor;
                    ctx.font = ChartTools.canvasFont(fontPx, styler.themeFontFamily, last);
                    ctx.fillText(Qt.formatDate(dayDate, "ddd"), dx + slotsPerDay * slotW / 2, height - fontPx * 0.3);
                }
            }
            ctx.font = ChartTools.canvasFont(fontPx, styler.themeFontFamily, false);

            if (noData) {
                ctx.fillStyle = styler.themeSecondaryColor;
                ctx.textAlign = "center";
                ctx.fillText(qsTr("No data"), axisW + plotW / 2, top + plotH / 2);
                return;
            }

            // bars
            var bw = Math.max(1, slotW * 0.7);
            var radius = Math.min(bw / 3, fontPx * 0.3);
            for (var i = 0; i < n; i++) {
                if (slots[i] < 0) continue;
                var h = Math.max(1.5, slots[i] / 100 * plotH);
                var x = axisW + i * slotW + (slotW - bw) / 2;
                var base = top + plotH;
                ctx.fillStyle = colorFor(slots[i]);
                ctx.beginPath();
                ctx.roundedRect(x, base - h, bw, h, radius, radius);
                ctx.fill();
                if (h > radius) ctx.fillRect(x, base - radius, bw, radius);
            }

            // thresholds where the colour changes
            ctx.lineWidth = styler.chartGuideLineWidth;
            for (var t = 0; t < guides.length; t++) {
                ctx.strokeStyle = guides[t].color;
                var ty = Math.round(yOf(guides[t].value)) + 0.5;
                ChartTools.dashedLine(ctx, axisW, width, ty, styler.chartDashLength, styler.chartDashLength);
            }
        }
    }
}
