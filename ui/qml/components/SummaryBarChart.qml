import QtQuick 2.0
import "./platform"
import "ChartColors.js" as ChartColors

// One bar per day with an optional dashed goal line and average line,
// like Gadgetbridge's weekly steps / sleep charts.
// points: [{ x: seconds, y: value, z: optional value stacked *below* y }]
Item {
    id: chart

    property var points: []
    property real goal: 0
    property color colorY: ChartColors.active
    property color colorBelowGoal: ChartColors.activeDim
    property color colorZ: ChartColors.deepSleep
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
    readonly property int count: priv.count
    readonly property real lastValue: priv.last

    width: parent ? parent.width : 0
    height: styler.themeItemSizeLarge * 3

    onPointsChanged: { priv.summarise(); canvas.requestPaint(); }
    onGoalChanged: { priv.summarise(); canvas.requestPaint(); }
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    QtObject {
        id: priv
        property real total: 0
        property real average: 0
        property real best: 0
        property var bestTime: 0
        property int goalDays: 0
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
            var t = 0, b = 0, bt = 0, gd = 0, c = 0, m = 0, l = 0;
            var list = chart.points || [];
            for (var i = 0; i < list.length; i++) {
                var v = pointValue(list[i]);
                l = v;
                if (v <= 0) continue;
                t += v; c++;
                if (v > b) { b = v; bt = list[i].x; }
                if (chart.goal > 0 && v >= chart.goal) gd++;
                if (v > m) m = v;
            }
            total = t; best = b; bestTime = bt; goalDays = gd; count = c; last = l;
            average = c ? t / c : 0;
            maxY = niceMax(Math.max(m, chart.goal * 1.15) * 1.05);
        }
    }

    ChartCanvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            var fontPx = styler.themeFontSizeExtraSmall;
            ctx.font = fontPx + "px \"" + styler.themeFontFamily + "\"";
            var axisW = fontPx * 3;
            var bottom = fontPx * 1.8;
            var top = fontPx * 0.6;
            var plotW = width - axisW;
            var plotH = height - bottom - top;
            var maxY = priv.maxY;

            function yOf(v) { return top + plotH - v / maxY * plotH; }

            ctx.lineWidth = 1;
            ctx.strokeStyle = ChartColors.withAlpha(styler.themeSecondaryColor, 0.25);
            ctx.fillStyle = styler.themeSecondaryColor;
            ctx.textAlign = "right";
            for (var g = 0; g <= 4; g++) {
                var val = maxY * g / 4;
                var gy = Math.round(yOf(val)) + 0.5;
                ctx.beginPath(); ctx.moveTo(axisW, gy); ctx.lineTo(width, gy); ctx.stroke();
                ctx.fillText(valueLabel(val), axisW - fontPx * 0.4, gy + fontPx * 0.35);
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
                ctx.font = (last ? "bold " : "") + fontPx + "px \"" + styler.themeFontFamily + "\"";
                ctx.textAlign = "center";
                ctx.fillText(Qt.formatDate(new Date(p.x * 1000), labelMask), x + bw / 2, height - fontPx * 0.3);
            }
            ctx.font = fontPx + "px \"" + styler.themeFontFamily + "\"";

            // average: scaled like the goal line (fixed 1-2 px dashes vanish on high-density
            // screens), dotted rhythm to tell it apart, labelled at the right end
            if (showAverage && priv.average > 0) {
                var ay = Math.round(yOf(priv.average)) + 0.5;
                ctx.strokeStyle = ChartColors.average;
                ctx.lineWidth = Math.max(1.5, fontPx / 12);
                ChartColors.dashedLine(ctx, axisW, width, ay, fontPx * 0.15, fontPx * 0.25);

                var avgText = "Ø " + averageLabel(priv.average);
                var goalY = goal > 0 ? yOf(goal) : -1000;
                // put the label below the line if it would collide with the goal line
                var labelY = (Math.abs(ay - goalY) < fontPx * 1.2 && goalY < ay) || ay - fontPx * 0.5 < top + fontPx
                        ? ay + fontPx * 1.1 : ay - fontPx * 0.4;
                ctx.font = "bold " + fontPx + "px \"" + styler.themeFontFamily + "\"";
                ctx.textAlign = "right";
                var tw = ctx.measureText(avgText).width;
                ctx.fillStyle = ChartColors.withAlpha("#0e121c", 0.75);
                ctx.fillRect(width - tw - fontPx * 0.5, labelY - fontPx, tw + fontPx * 0.5, fontPx * 1.3);
                ctx.fillStyle = ChartColors.average;
                ctx.fillText(avgText, width - fontPx * 0.25, labelY);
                ctx.font = fontPx + "px \"" + styler.themeFontFamily + "\"";
            }
            if (goal > 0) {
                ctx.strokeStyle = ChartColors.goal;
                ctx.lineWidth = Math.max(1.5, fontPx / 12);
                ChartColors.dashedLine(ctx, axisW, width, Math.round(yOf(goal)) + 0.5, fontPx * 0.4, fontPx * 0.3);
            }
        }
    }
}
