import QtQuick 2.0
import "./platform"
import "ChartTools.js" as ChartTools

// Hypnogram with one lane per sleep phase.
Item {
    id: chart

    function phaseColor(k) {
        if (k === ChartTools.PHASE_DEEP) return styler.chartDeepSleepColor;
        if (k === ChartTools.PHASE_LIGHT) return styler.chartLightSleepColor;
        return styler.chartAwakeColor;
    }

    property var samples: []

    readonly property int lightMinutes: priv.light
    readonly property int deepMinutes: priv.deep
    readonly property int awakeMinutes: priv.awake
    readonly property int totalMinutes: priv.light + priv.deep
    readonly property var sleepStart: priv.start
    readonly property var sleepEnd: priv.end
    readonly property bool noData: priv.first < 0

    width: parent ? parent.width : 0
    height: styler.themeItemSizeLarge * 2.6

    onSamplesChanged: { priv.analyse(); canvas.requestPaint(); }
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    QtObject {
        id: priv
        property int first: -1
        property int last: -1
        property int light: 0
        property int deep: 0
        property int awake: 0
        property var start: 0
        property var end: 0

        // Trims the 24 h window to the part between the first and the last sleep sample.
        function analyse() {
            var list = chart.samples || [];
            var f = -1, l = -1;
            for (var i = 0; i < list.length; i++) {
                if (list[i].k !== ChartTools.PHASE_AWAKE) {
                    if (f < 0) f = i;
                    l = i;
                }
            }
            var li = 0, de = 0, aw = 0;
            for (i = Math.max(0, f); f >= 0 && i < l; i++) {
                var dur = (list[i + 1].x - list[i].x) / 60;
                if (dur > 10) dur = 1;     // gap in the data, count a single minute
                if (list[i].k === ChartTools.PHASE_DEEP) de += dur;
                else if (list[i].k === ChartTools.PHASE_LIGHT) li += dur;
                else aw += dur;
            }
            first = f; last = l;
            light = Math.round(li); deep = Math.round(de); awake = Math.round(aw);
            start = f >= 0 ? new Date(list[f].x * 1000) : 0;
            end = f >= 0 ? new Date(list[l].x * 1000) : 0;
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
            var labelW = fontPx * 4.5;
            var bottom = fontPx * 1.8;
            var plotW = width - labelW;
            var plotH = height - bottom;

            var lanes = [
                { label: qsTr("Awake"), k: ChartTools.PHASE_AWAKE },
                { label: qsTr("Light"), k: ChartTools.PHASE_LIGHT },
                { label: qsTr("Deep"),  k: ChartTools.PHASE_DEEP }
            ];
            var laneH = plotH / lanes.length;
            var laneOf = {};

            for (var j = 0; j < lanes.length; j++) {
                laneOf[lanes[j].k] = j;
                ctx.fillStyle = styler.chartEmptyColor;
                ctx.fillRect(labelW, j * laneH + 1, plotW, laneH - 2);
                ctx.fillStyle = styler.themeSecondaryColor;
                ctx.textAlign = "left";
                ctx.fillText(lanes[j].label, 0, j * laneH + laneH / 2 + fontPx * 0.35);
            }

            if (noData) {
                ctx.textAlign = "center";
                ctx.fillText(qsTr("No data"), labelW + plotW / 2, plotH / 2);
                return;
            }

            var list = samples;
            var t0 = list[priv.first].x;
            var t1 = list[priv.last].x + 60;
            var scale = plotW / (t1 - t0);

            // merge consecutive samples of the same phase into one block
            var i = priv.first;
            while (i <= priv.last) {
                var k = list[i].k;
                var jEnd = i;
                while (jEnd + 1 <= priv.last && list[jEnd + 1].k === k) jEnd++;
                var xs = labelW + (list[i].x - t0) * scale;
                var xe = labelW + (Math.min(list[jEnd].x + 60, t1) - t0) * scale;
                var lane = laneOf[k];
                ctx.fillStyle = chart.phaseColor(k);
                ctx.beginPath();
                ctx.roundedRect(xs, lane * laneH + laneH * 0.15, Math.max(1.5, xe - xs), laneH * 0.7, 2, 2);
                ctx.fill();
                i = jEnd + 1;
            }

            ctx.fillStyle = styler.themeSecondaryColor;
            for (var t = 0; t <= 3; t++) {
                var tt = t0 + (t1 - t0) * t / 3;
                ctx.textAlign = t === 0 ? "left" : (t === 3 ? "right" : "center");
                var tick = Qt.formatTime(new Date(tt * 1000), "hh:mm");
                ctx.fillText(tick, labelW + plotW * t / 3, height - fontPx * 0.3);
            }
        }
    }
}
