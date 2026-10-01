.pragma library
// Colour palette shared by all charts, modelled on Gadgetbridge's chart colours.
// Plain ES5 only: Sailfish OS still ships an older QML JS engine.

var deepSleep  = "#1a5fb4";
var lightSleep = "#46acea";
var awake      = "#f5a623";
var active     = "#5ad24a";
var activeDim  = "#3f7f38";
var belowGoal  = "#f2d541";   // yellow, clearly lighter than the orange goal line
var average    = "#e6ffffff";

// Battery levels: green / yellow / red, also clearly different in lightness
var batteryHigh   = "#5ad24a";   // >= 50 %
var batteryMedium = "#f2d541";   // 20 - 49 %
var batteryLow    = "#ff6b6b";   // < 20 %

function batteryColor(percent) {
    if (percent >= 50) return batteryHigh;
    if (percent >= 20) return batteryMedium;
    return batteryLow;
}
var heartrate  = "#ff6b6b";
var goal       = "#f5a623";
var distance   = "#4fc3c9";

// Heart rate zone colours, from relaxed to VO2 max
var zones = ["#8a96a8", "#46acea", "#5ad24a", "#e8d44d", "#f5a623", "#ff6b6b"];

// Phase constants must match DataSource::Phase
var PHASE_AWAKE = 0;
var PHASE_LIGHT = 1;
var PHASE_DEEP  = 2;

function withAlpha(c, a) {
    if (typeof c === "string") c = Qt.darker(c, 1.0);   // "#rrggbb" -> color
    return Qt.rgba(c.r, c.g, c.b, a);
}

function phaseColor(k) {
    if (k === PHASE_DEEP) return deepSleep;
    if (k === PHASE_LIGHT) return lightSleep;
    return awake;
}

// Draws a dashed horizontal line; Context2D on Qt 5.6 has no setLineDash().
function dashedLine(ctx, x0, x1, y, dash, gap) {
    ctx.beginPath();
    for (var x = x0; x < x1; x += dash + gap) {
        ctx.moveTo(x, y);
        ctx.lineTo(Math.min(x + dash, x1), y);
    }
    ctx.stroke();
}

function formatDuration(minutes) {
    var m = Math.round(minutes);
    var h = Math.floor(m / 60);
    var r = m % 60;
    return qsTr("%1 h %2 min").arg(h).arg((r < 10 ? "0" : "") + r);
}
