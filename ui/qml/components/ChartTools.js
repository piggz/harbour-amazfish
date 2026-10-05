.pragma library
// Helpers for the charts. Colours, opacities and line widths come from StylerPL.

// Must match DataSource::Phase
var PHASE_AWAKE = 0;
var PHASE_LIGHT = 1;
var PHASE_DEEP = 2;

function withAlpha(c, a) {
    return Qt.rgba(c.r, c.g, c.b, a);
}

// Context2D on Qt 5.6 has no setLineDash()
function dashedLine(ctx, x0, x1, y, dash, gap) {
    ctx.beginPath();
    for (var x = x0; x < x1; x += dash + gap) {
        ctx.moveTo(x, y);
        ctx.lineTo(Math.min(x + dash, x1), y);
    }
    ctx.stroke();
}

// Font for a Canvas. Kirigami gives a full font description as family, so only the name before the first comma is used.
function canvasFont(px, family, bold) {
    return (bold ? "bold " : "") + px + "px \"" + String(family).split(",")[0] + "\"";
}

function formatDuration(minutes) {
    var m = Math.round(minutes);
    var h = Math.floor(m / 60);
    var r = m % 60;
    return qsTr("%1 h %2 min").arg(h).arg((r < 10 ? "0" : "") + r);
}
