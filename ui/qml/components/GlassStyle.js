.pragma library
// Shared "glass" surface tokens: every tile, card, bar and list group uses these,
// so corner radii and backgrounds stay consistent across the app.
// Colours are #AARRGGBB.

var fill           = "#800e121c";   // dark tint, 50 % opaque
var border         = "#1affffff";   // hairline, 10 % white
var edge           = "#14ffffff";   // light edge along the top, 8 % white
var pressedOverlay = "#14ffffff";   // pressed feedback
var chip           = "#14ffffff";   // icon circles and small fills inside a surface
var divider        = "#14ffffff";   // separators between rows inside a surface
var track          = "#1fffffff";   // empty part of bars and gauges

// Large surfaces (tiles, cards, list groups)
function radiusLarge(styler) {
    return Math.round(styler.themePaddingLarge * 1.25);
}

// Elements inside a surface (maps, progress bars): 60 % of the large radius
function radiusMedium(styler) {
    return Math.round(radiusLarge(styler) * 0.6);
}
