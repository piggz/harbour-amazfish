import QtQuick 2.0
import "./platform"
import "ChartTools.js" as ChartTools

// One bar per day, with an optional goal line, recommended range band and average line.
// points: [{ x: seconds, y: value, z: optional value stacked below y }]
Item {
    id: chart

    property var points: []
    property real goal: 0
    property real bandLow: 0                 // recommended range, e.g. 7-9 h of sleep
    property real bandHigh: 0
    property color bandColor: styler.chartGoalColor
    readonly property bool hasBand: bandHigh > bandLow && bandLow > 0
    property color colorY: styler.chartActiveColor
    property color colorBelowGoal: styler.chartActiveDimColor
    property color colorZ: styler.chartDeepSleepColor
    property bool showAverage: true
    property string labelMask: "ddd"
    property var valueLabel: function(v) { return Math.round(v).toString(); }
    property var averageLabel: valueLabel   // text next to the average line

    readonly property bool stacked: points.length > 0 && typeof points[0].z !== "undefined"
    readonly property bool noData: priv.count === 0

    // summary values
    readonly property real total: priv.total
    readonly property real average: priv.average
    readonly property real best: priv.best
    readonly property var bestTime: priv.bestTime
    readonly property int goalDays: priv.goalDays
    readonly property int bandDays: priv.bandDays       // days inside the recommended range
    readonly property int count: priv.count
    readonly property real lastValue: priv.last

    width: parent ? parent.width : 0
    height: styler.themeItemSizeLarge * 3

    onPointsChanged: { priv.summarise(); canvas.requestPaint(); }
    onGoalChanged: { priv.summarise(); canvas.requestPaint(); }
    onBandLowChanged: { priv.summarise(); canvas.requestPaint(); }
    onBandHighChanged: { priv.summarise(); canvas.requestPaint(); }
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    QtObject {
        id: priv
        property real total: 0
        property real average: 0
        property real best: 0
        property var bestTime: 0
        property int goalDays: 0
        property int bandDays: 0
        property int count: 0
        property real last: 0
        property real maxY: 1

        function pointValue(p) {
            return p.y + (typeof p.z !== "undefined" ? p.z : 0);
        }

        // Rounds v up so that the four grid steps land on round numbers.
        function niceMax(v) {
            if (v <= 0) return 4;
            var step = v / 4;
            var mag = Math.pow(10, Math.floor(Math.log(step) / Math.LN10));
            var nice = [1, 2, 2.5, 3, 4, 5, 6, 8, 10];
            for (var i = 0; i < nice.length; i++) {
                if (nice[i] * mag >= step) return nice[i] * mag * 4;
            }
            return 40 * mag;
        }

        function summarise() {
            var t = 0, b = 0, bt = 0, gd = 0, bd = 0, c = 0, m = 0, l = 0;
            var list = chart.points || [];
            for (var i = 0; i < list.length; i++) {
                var v = pointValue(list[i]);
                l = v;
                if (v <= 0) continue;
                t += v; c++;
                if (v > b) { b = v; bt = list[i].x; }
                if (chart.goal > 0 && v >= chart.goal) gd++;
                if (chart.hasBand && v >= chart.bandLow && v <= chart.bandHigh) bd++;
                if (v > m) m = v;
            }
            total = t; best = b; bestTime = bt; goalDays = gd; bandDays = bd; count = c; last = l;
            average = c ? t / c : 0;
            maxY = niceMax(Math.max(m, chart.goal * 1.15, chart.hasBand ? chart.bandHigh * 1.1 : 0) * 1.05);
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
            var axisW = fontPx * 3;
            var bottom = fontPx * 1.8;
            var top = fontPx * 0.6;
            var plotW = width - axisW;
            var plotH = height - bottom - top;
            var maxY = priv.maxY;

            function yOf(v) { return top + plotH - v / maxY * plotH; }

            ctx.lineWidth = styler.chartGridLineWidth;
            ctx.strokeStyle = styler.chartGridColor;
            ctx.fillStyle = styler.themeSecondaryColor;
            ctx.textAlign = "right";
            for (var g = 0; g <= 4; g++) {
                var val = maxY * g / 4;
                var gy = Math.round(yOf(val)) + 0.5;
                ctx.beginPath(); ctx.moveTo(axisW, gy); ctx.lineTo(width, gy); ctx.stroke();
                ctx.fillText(valueLabel(val), axisW - fontPx * 0.4, gy + fontPx * 0.35);
            }

            // recommended range behind the bars, with dashed edges
            if (hasBand) {
                var bandTop = Math.round(yOf(bandHigh)) + 0.5;
                var bandBottom = Math.round(yOf(bandLow)) + 0.5;
                ctx.fillStyle = ChartTools.withAlpha(bandColor, styler.chartBandOpacity);
                ctx.fillRect(axisW, bandTop, width - axisW, bandBottom - bandTop);
                ctx.strokeStyle = ChartTools.withAlpha(bandColor, styler.chartGuideOpacity);
                ctx.lineWidth = styler.chartGuideLineWidth;
                ChartTools.dashedLine(ctx, axisW, width, bandTop, styler.chartDashLength, styler.chartDashLength);
                ChartTools.dashedLine(ctx, axisW, width, bandBottom, styler.chartDashLength, styler.chartDashLength);
            }

            var list = points || [];
            if (list.length === 0) {
                ctx.textAlign = "center";
                ctx.fillText(qsTr("No data"), axisW + plotW / 2, top + plotH / 2);
                return;
            }

            var slot = plotW / list.length;
            var bw = Math.min(slot * 0.6, fontPx * 3);
            var radius = Math.min(bw / 3, fontPx * 0.4);

            for (var i = 0; i < list.length; i++) {
                var p = list[i];
                var x = axisW + i * slot + (slot - bw) / 2;
                var base = top + plotH;

                if (stacked) {
                    var hz = p.z / maxY * plotH;
                    var hy = p.y / maxY * plotH;
                    if (hz > 0) { ctx.fillStyle = colorZ; ctx.fillRect(x, base - hz, bw, hz); }
                    if (hy > 0) {
                        ctx.fillStyle = colorY;
                        ctx.beginPath();
                        ctx.roundedRect(x, base - hz - hy, bw, hy, radius, radius);
                        ctx.fill();
                        if (hy > radius) ctx.fillRect(x, base - hz - radius, bw, radius);
                    }
                } else {
                    var h = p.y / maxY * plotH;
                    if (h > 0) {
                        ctx.fillStyle = (goal > 0 && p.y < goal) ? colorBelowGoal : colorY;
                        ctx.beginPath();
                        ctx.roundedRect(x, base - h, bw, h, radius, radius);
                        ctx.fill();
                        if (h > radius) ctx.fillRect(x, base - radius, bw, radius);
                    }
                }

                var last = i === list.length - 1;
                ctx.fillStyle = last ? styler.themeHighlightColor : styler.themeSecondaryColor;
                ctx.font = ChartTools.canvasFont(fontPx, styler.themeFontFamily, last);
                ctx.textAlign = "center";
                ctx.fillText(Qt.formatDate(new Date(p.x * 1000), labelMask), x + bw / 2, height - fontPx * 0.3);
            }
            ctx.font = ChartTools.canvasFont(fontPx, styler.themeFontFamily, false);

            // Average line, dotted to tell it apart from the goal line, with its value at the right end.
            if (showAverage && priv.average > 0) {
                var ay = Math.round(yOf(priv.average)) + 0.5;
                ctx.strokeStyle = styler.chartAverageColor;
                ctx.lineWidth = styler.chartLineWidth;
                ChartTools.dashedLine(ctx, axisW, width, ay, fontPx * 0.15, fontPx * 0.25);

                var avgText = qsTr("Avg. %1").arg(averageLabel(priv.average));
                var goalY = goal > 0 ? yOf(goal) : -1000;
                // put the label below the line if it would collide with the goal line
                var labelY = (Math.abs(ay - goalY) < fontPx * 1.2 && goalY < ay) || ay - fontPx * 0.5 < top + fontPx
                        ? ay + fontPx * 1.1 : ay - fontPx * 0.4;
                ctx.font = ChartTools.canvasFont(fontPx, styler.themeFontFamily, true);
                ctx.textAlign = "right";
                var tw = ctx.measureText(avgText).width;
                ctx.fillStyle = styler.chartLabelBackgroundColor;
                ctx.fillRect(width - tw - fontPx * 0.5, labelY - fontPx, tw + fontPx * 0.5, fontPx * 1.3);
                ctx.fillStyle = styler.chartAverageColor;
                ctx.fillText(avgText, width - fontPx * 0.25, labelY);
                ctx.font = ChartTools.canvasFont(fontPx, styler.themeFontFamily, false);
            }
            if (goal > 0) {
                ctx.strokeStyle = styler.chartGoalColor;
                ctx.lineWidth = styler.chartLineWidth;
                ChartTools.dashedLine(ctx, axisW, width, Math.round(yOf(goal)) + 0.5, fontPx * 0.4, fontPx * 0.3);
            }
        }
    }
}
