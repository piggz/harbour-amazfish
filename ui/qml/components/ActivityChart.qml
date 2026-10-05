import QtQuick 2.0
import "./platform"
import "ChartTools.js" as ChartTools

// Day chart: bars coloured by activity and sleep phase, heart rate as a line.
Item {
    id: chart

    property var samples: []
    property var startTime: 0           // seconds since epoch of the first plotted instant
    property int durationSec: 86400
    property bool showHeartrate: true
    property real maxHeartrate: 200
    property real minHeartrate: 40
    property int activeIntensity: 15    // intensity % from which a minute counts as active if it has no steps

    // Summary values, updated whenever samples change
    readonly property int activeMinutes: priv.activeMinutes
    readonly property int lightMinutes: priv.lightMinutes
    readonly property int deepMinutes: priv.deepMinutes
    readonly property int averageHeartrate: priv.averageHeartrate
    readonly property bool noData: !samples || samples.length === 0

    width: parent ? parent.width : 0
    height: styler.themeItemSizeLarge * 3.2

    onSamplesChanged: { priv.summarise(); canvas.requestPaint(); }
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    QtObject {
        id: priv
        property int activeMinutes: 0
        property int lightMinutes: 0
        property int deepMinutes: 0
        property int averageHeartrate: 0

        function isActive(p) {
            return p.s > 0 || p.i >= chart.activeIntensity;
        }

        function summarise() {
            var a = 0, l = 0, d = 0, hrSum = 0, hrCount = 0;
            var list = chart.samples || [];
            for (var n = 0; n < list.length; n++) {
                var p = list[n];
                if (p.k === ChartTools.PHASE_DEEP) d++;
                else if (p.k === ChartTools.PHASE_LIGHT) l++;
                else if (isActive(p)) a++;
                if (p.h > 0) { hrSum += p.h; hrCount++; }
            }
            activeMinutes = a; lightMinutes = l; deepMinutes = d;
            averageHeartrate = hrCount ? Math.round(hrSum / hrCount) : 0;
        }
    }

    ChartCanvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            var fontPx = styler.themeFontSizeExtraSmall;
            var axisW = showHeartrate ? fontPx * 2.2 : 0;
            var bottom = fontPx * 1.6;
            var plotW = width - axisW;
            var plotH = height - bottom - fontPx * 0.5;
            var top = fontPx * 0.5;

            ctx.font = ChartTools.canvasFont(fontPx, styler.themeFontFamily, false);

            // grid + heart rate axis
            ctx.lineWidth = styler.chartGridLineWidth;
            ctx.strokeStyle = styler.chartGridColor;
            ctx.fillStyle = styler.chartHeartRateColor;
            ctx.textAlign = "left";
            for (var g = 0; g <= 4; g++) {
                var gy = Math.round(top + plotH * g / 4) + 0.5;
                ctx.beginPath(); ctx.moveTo(0, gy); ctx.lineTo(plotW, gy); ctx.stroke();
                if (showHeartrate) {
                    var v = maxHeartrate - (maxHeartrate - minHeartrate) * g / 4;
                    ctx.fillText(Math.round(v), plotW + fontPx * 0.3, gy + fontPx * 0.35);
                }
            }

            // time axis
            ctx.fillStyle = styler.themeSecondaryColor;
            for (var t = 0; t <= 4; t++) {
                var tx = plotW * t / 4;
                var d = new Date((startTime + durationSec * t / 4) * 1000);
                ctx.textAlign = t === 0 ? "left" : (t === 4 ? "right" : "center");
                ctx.fillText(Qt.formatTime(d, "hh:mm"), tx, height - fontPx * 0.3);
            }

            if (noData) {
                ctx.textAlign = "center";
                ctx.fillText(qsTr("No data"), plotW / 2, top + plotH / 2);
                return;
            }

            // one bucket per ~2 px, dominant kind wins (deep > light > active > idle)
            var buckets = Math.max(24, Math.min(1440, Math.floor(plotW / 2)));
            var bucketSec = durationSec / buckets;
            var bw = plotW / buckets;
            var rank = [], inten = [], hrSum = [], hrCnt = [];
            for (var b = 0; b < buckets; b++) { rank.push(-1); inten.push(0); hrSum.push(0); hrCnt.push(0); }

            for (var n = 0; n < samples.length; n++) {
                var p = samples[n];
                var bi = Math.floor((p.x - startTime) / bucketSec);
                if (bi < 0 || bi >= buckets) continue;
                var r = p.k === ChartTools.PHASE_DEEP ? 3
                      : p.k === ChartTools.PHASE_LIGHT ? 2
                      : priv.isActive(p) ? 1 : 0;
                if (r > rank[bi]) rank[bi] = r;
                if (p.i > inten[bi]) inten[bi] = p.i;
                if (p.h > 0) { hrSum[bi] += p.h; hrCnt[bi]++; }
            }

            var idle = styler.chartInactiveColor;
            for (b = 0; b < buckets; b++) {
                if (rank[b] < 0) continue;
                var frac, col;
                if (rank[b] === 3) { frac = 0.45; col = styler.chartDeepSleepColor; }
                else if (rank[b] === 2) { frac = 0.30; col = styler.chartLightSleepColor; }
                else if (rank[b] === 1) { frac = Math.max(0.06, inten[b] / 100); col = styler.chartActiveColor; }
                else { frac = Math.max(0.03, inten[b] / 100); col = idle; }
                var bh = Math.max(1, plotH * frac);
                ctx.fillStyle = col;
                ctx.fillRect(b * bw, top + plotH - bh, Math.max(1, bw * 0.8), bh);
            }

            if (!showHeartrate) return;

            // heart rate line, broken on gaps
            ctx.strokeStyle = styler.chartHeartRateColor;
            ctx.lineWidth = styler.chartLineWidth;
            ctx.lineJoin = "round";
            ctx.beginPath();
            var drawing = false, gap = 0;
            var span = maxHeartrate - minHeartrate;
            for (b = 0; b < buckets; b++) {
                if (!hrCnt[b]) {
                    if (++gap * bucketSec > 15 * 60) drawing = false;
                    continue;
                }
                gap = 0;
                var hv = Math.max(minHeartrate, Math.min(maxHeartrate, hrSum[b] / hrCnt[b]));
                var hx = b * bw + bw / 2;
                var hy = top + plotH - (hv - minHeartrate) / span * plotH;
                if (drawing) ctx.lineTo(hx, hy); else ctx.moveTo(hx, hy);
                drawing = true;
            }
            ctx.stroke();
        }
    }
}
