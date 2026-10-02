import QtQuick 2.0
import "ChartColors.js" as ChartColors

// Draws only the shape of a route (no map), used when the watch sent no start
// position and the absolute location of the track is unknown.
// coordinates: list of QGeoCoordinate
Item {
    id: shape

    property var coordinates: []
    // real latitude of the route, if known; east-west distances shrink with its cosine
    property real referenceLatitude: 0
    onReferenceLatitudeChanged: canvas.requestPaint()

    width: parent ? parent.width : 0
    height: width * 0.6

    onCoordinatesChanged: canvas.requestPaint()
    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    ChartCanvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            var pts = shape.coordinates || [];
            if (pts.length < 2) return;

            // equirectangular projection is exact enough for the size of a workout
            var minX = Number.MAX_VALUE, maxX = -Number.MAX_VALUE, minY = Number.MAX_VALUE, maxY = -Number.MAX_VALUE;
            var scaleLon = Math.cos(shape.referenceLatitude * Math.PI / 180);
            var xy = [];
            for (var i = 0; i < pts.length; i++) {
                var x = pts[i].longitude * scaleLon, y = pts[i].latitude;
                xy.push([x, y]);
                if (x < minX) minX = x; if (x > maxX) maxX = x;
                if (y < minY) minY = y; if (y > maxY) maxY = y;
            }
            var margin = styler.themePaddingLarge * 1.5;
            var spanX = Math.max(maxX - minX, 1e-9), spanY = Math.max(maxY - minY, 1e-9);
            var s = Math.min((width - 2 * margin) / spanX, (height - 2 * margin) / spanY);
            var offX = (width - spanX * s) / 2, offY = (height - spanY * s) / 2;
            function px(p) { return [offX + (p[0] - minX) * s, height - offY - (p[1] - minY) * s]; }

            ctx.strokeStyle = ChartColors.awake;
            ctx.lineWidth = Math.max(3, styler.themePaddingSmall);
            ctx.lineJoin = "round";
            ctx.lineCap = "round";
            ctx.beginPath();
            for (i = 0; i < xy.length; i++) {
                var p = px(xy[i]);
                if (i === 0) ctx.moveTo(p[0], p[1]); else ctx.lineTo(p[0], p[1]);
            }
            ctx.stroke();

            var r = Math.max(5, styler.themePaddingSmall * 1.4);
            var ends = [[xy[0], ChartColors.active], [xy[xy.length - 1], ChartColors.heartrate]];
            for (var e = 0; e < ends.length; e++) {
                var q = px(ends[e][0]);
                ctx.beginPath();
                ctx.arc(q[0], q[1], r, 0, 2 * Math.PI, false);
                ctx.fillStyle = ends[e][1];
                ctx.fill();
                ctx.lineWidth = 2;
                ctx.strokeStyle = "white";
                ctx.stroke();
            }
        }
    }
}
