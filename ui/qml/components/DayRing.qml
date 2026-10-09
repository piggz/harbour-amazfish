import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "ChartTools.js" as ChartTools

// 24 h ring coloured by sleep phase and activity, midnight at the top.
Item {
    id: ring

    property var samples: []
    property var startTime: 0
    property int segments: 144                  // 10 minute segments
    property real lineWidth: width * styler.chartRingWidthRatio
    property int activeIntensity: 15
    property color idleColor: styler.chartInactiveColor
    property color emptyColor: styler.chartGridFaintColor
    property bool showHourMarks: false          // ticks and labels at 0, 6, 12 and 18 h
    // mark labels scale with the ring so they always fit inside it
    readonly property real markFontSize: Math.max(8, width * 0.075)
    readonly property real markSpace: showHourMarks ? markFontSize * 1.9 : 0
    // radius left free inside the ring and its hour marks, e.g. for a goal gauge
    readonly property real innerRadius: Math.max(0, Math.min(width, height) / 2 - lineWidth - markSpace)
    default property alias content: centre.data

    implicitWidth: styler.themeItemSizeLarge * 3
    implicitHeight: implicitWidth

    onSamplesChanged: canvas.requestPaint()
    onIdleColorChanged: canvas.requestPaint()
    onShowHourMarksChanged: canvas.requestPaint()

    ChartCanvas {
        id: canvas
        anchors.fill: parent
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            var n = ring.segments;
            var rank = [];
            for (var i = 0; i < n; i++) rank.push(-1);

            var list = ring.samples || [];
            var segSec = 86400 / n;
            for (i = 0; i < list.length; i++) {
                var p = list[i];
                var s = Math.floor((p.x - ring.startTime) / segSec);
                if (s < 0 || s >= n) continue;
                var r = p.k === DataSource.PhaseDeepSleep ? 3
                      : p.k === DataSource.PhaseLightSleep ? 2
                      : (p.s > 0 || p.i >= ring.activeIntensity) ? 1 : 0;
                if (r > rank[s]) rank[s] = r;
            }

            var cx = width / 2, cy = height / 2;
            var rad = Math.min(width, height) / 2 - ring.lineWidth / 2;
            // see GaugeArc: skip until the ring has a real size, it repaints on resize
            if (rad <= 0) {
                return;
            }
            var step = 2 * Math.PI / n;
            ctx.lineWidth = ring.lineWidth;
            ctx.lineCap = "butt";

            for (i = 0; i < n; i++) {
                var a = -Math.PI / 2 + i * step;
                ctx.strokeStyle = rank[i] === 3 ? styler.chartDeepSleepColor
                                : rank[i] === 2 ? styler.chartLightSleepColor
                                : rank[i] === 1 ? styler.chartActiveColor
                                : rank[i] === 0 ? ring.idleColor
                                : ring.emptyColor;
                ctx.beginPath();
                ctx.arc(cx, cy, rad, a, a + step * 1.02, false);   // slight overlap avoids hairline gaps
                ctx.stroke();
            }

            if (!ring.showHourMarks) {
                return;
            }
            // hour marks just inside the ring, midnight at the top
            var fontPx = ring.markFontSize;
            var inner = rad - ring.lineWidth / 2;
            ctx.lineWidth = styler.chartGuideLineWidth;
            ctx.strokeStyle = styler.themeSecondaryColor;
            ctx.fillStyle = styler.themeSecondaryColor;
            ctx.font = ChartTools.canvasFont(fontPx, styler.themeFontFamily, false);
            ctx.textAlign = "center";
            var hours = [0, 6, 12, 18];
            for (var h = 0; h < hours.length; h++) {
                var ang = -Math.PI / 2 + hours[h] / 24 * 2 * Math.PI;
                var cos = Math.cos(ang), sin = Math.sin(ang);
                ctx.beginPath();
                ctx.moveTo(cx + (inner - 1) * cos, cy + (inner - 1) * sin);
                ctx.lineTo(cx + (inner - fontPx * 0.45) * cos, cy + (inner - fontPx * 0.45) * sin);
                ctx.stroke();
                var lr = inner - fontPx * 1.15;
                ctx.fillText(hours[h], cx + lr * cos, cy + lr * sin + fontPx * 0.35);
            }
        }
    }

    Item {
        id: centre
        anchors.fill: parent
    }
}
