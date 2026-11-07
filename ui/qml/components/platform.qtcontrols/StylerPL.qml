/* -*- coding: utf-8-unix -*-
 *
 * Copyright (C) 2018 Rinigus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import QtQuick.Window 2.2

QtObject {
    // font sizes and family
    property string themeFontFamily: Qt.application.font.family
    property string themeFontFamilyHeading: Qt.application.font.family
    property int  themeFontSizeHuge: Math.round(themeFontSizeMedium*3.0)
    property int  themeFontSizeExtraLarge: Math.round(themeFontSizeMedium*2.0)
    property int  themeFontSizeLarge: Math.round(themeFontSizeMedium*1.5)
    property int  themeFontSizeMedium: Math.round(Qt.application.font.pixelSize*1.0)
    property int  themeFontSizeSmall: Math.round(themeFontSizeMedium*0.9)
    property int  themeFontSizeExtraSmall: Math.round(themeFontSizeMedium*0.7)
    property real themeFontSizeOnMap: themeFontSizeSmall

    // colors
    // block background (navigation, poi panel, bubble)
    property color blockBg: palette.window
    // variant of navigation icons
    property string navigationIconsVariant: darkTheme ? "white" : "black"
    // descriptive items
    property color themeHighlightColor: palette.windowText
    // navigation items (to be clicked)
    property color themePrimaryColor: palette.text
    // navigation items, secondary
    property color themeSecondaryColor: inactivePalette.text
    // descriptive items, secondary
    property color themeSecondaryHighlightColor: inactivePalette.text

    // button sizes
    property real themeButtonWidthLarge: 256
    property real themeButtonWidthMedium: 180

    // icon sizes
    property real themeIconSizeLarge: 2.5*themeFontSizeLarge
    property real themeIconSizeMedium: 2*themeFontSizeLarge
    property real themeIconSizeSmall: 1.5*themeFontSizeLarge
    // used icons
    property string iconAbout: "help-about-symbolic"
    property string iconBack: "go-previous-symbolic"
    property string iconClear: "edit-clear-all-symbolic"
    property string iconClose: "window-close-symbolic"
    property string iconDelete: "edit-delete-symbolic"
    property string iconDown: "go-down-symbolic"
    property string iconEdit: "document-edit-symbolic"
    property string iconEditClear: "edit-clear-symbolic"
    property string iconFavorite: "bookmark-new-symbolic"
    property string iconFavoriteSelected: "user-bookmarks-symbolic"
    property string iconForward: "go-next-symbolic"
    property string iconMenu: "open-menu-symbolic"
    property string iconPause: "media-playback-pause-symbolic"
    property string iconPhone: "call-start-symbolic"
    property string iconPreferences: "preferences-system-symbolic"
    property string iconRefresh: "view-refresh-symbolic"
    property string iconSave: "document-save-symbolic"
    property string iconSearch: "edit-find-symbolic"
    property string iconShare: "emblem-shared-symbolic"
    property string iconStart: "media-playback-start-symbolic"
    property string iconStop: "media-playback-stop-symbolic"
    property string iconWebLink: "web-browser-symbolic"

    // item sizes
    property real themeItemSizeLarge: themeFontSizeLarge * 3
    property real themeItemSizeSmall: themeFontSizeMedium * 3
    property real themeItemSizeExtraSmall: themeFontSizeSmall * 3

    // paddings and page margins
    property real themeHorizontalPageMargin: 1.25*themeFontSizeExtraLarge
    property real themePaddingLarge: 0.75*themeFontSizeExtraLarge
    property real themePaddingMedium: 0.5*themeFontSizeLarge
    property real themePaddingSmall: 0.25*themeFontSizeSmall

    property real themePixelRatio: Screen.devicePixelRatio

    property bool darkTheme: (blockBg.r + blockBg.g + blockBg.b) <
                             (themePrimaryColor.r + themePrimaryColor.g +
                              themePrimaryColor.b)

    property list<QtObject> children: [
        SystemPalette {
            id: palette
            colorGroup: SystemPalette.Active
        },

        SystemPalette {
            id: disabledPalette
            colorGroup: SystemPalette.Disabled
        },

        SystemPalette {
            id: inactivePalette
            colorGroup: SystemPalette.Inactive
        }
    ]

    // returns the colour c (also a colour name) with the opacity a
    function alpha(c, a) {
        var q = Qt.lighter(c, 1.0);
        return Qt.rgba(q.r, q.g, q.b, a);
    }

    // surfaces of tiles, cards and grouped lists
    property color surfaceColor: alpha(themePrimaryColor, 0.06)
    property color surfaceBorderColor: alpha(themePrimaryColor, 0.1)
    property color surfaceEdgeColor: alpha(themePrimaryColor, 0.08)
    property color surfacePressedColor: alpha(themePrimaryColor, 0.08)
    property color surfaceChipColor: alpha(themePrimaryColor, 0.08)
    property color surfaceDividerColor: alpha(themePrimaryColor, 0.08)
    property color surfaceTrackColor: alpha(themePrimaryColor, 0.12)
    property real surfaceRadius: Math.round(themePaddingLarge * 1.25)
    property real surfaceBorderWidth: 1
    property real surfacePressedOpacity: 0.6

    // reordering items by drag and drop
    property color dragIndicatorColor: themeHighlightColor
    property real dragIndicatorOpacity: 0.8
    property real draggedItemOpacity: 0.9

    // structure of the charts
    property color chartGridColor: alpha(themeSecondaryColor, 0.25)
    property color chartGridFaintColor: alpha(themeSecondaryColor, 0.12)
    property color chartEmptyColor: alpha(themeSecondaryColor, 0.1)
    property color chartInactiveColor: alpha(themeSecondaryColor, 0.35)
    property color chartAverageColor: alpha(themePrimaryColor, 0.9)
    property color chartLabelBackgroundColor: alpha(blockBg, 0.75)

    // colours with a fixed meaning in the charts, close to Gadgetbridge
    property color chartDeepSleepColor: "#1a5fb4"
    property color chartLightSleepColor: "#46acea"
    property color chartAwakeColor: "#f5a623"
    property color chartActiveColor: "#5ad24a"
    property color chartActiveDimColor: "#3f7f38"
    property color chartGoalColor: "#f5a623"
    property color chartBelowGoalColor: "#f2d541"
    property color chartHeartRateColor: "#ff6b6b"
    property color chartRestingHeartRateColor: "#46acea"
    property color chartPaceColor: "#46acea"
    property color chartElevationColor: "#5ad24a"
    property var chartZoneColors: ["#8a96a8", "#46acea", "#5ad24a", "#e8d44d", "#f5a623", "#ff6b6b"]
    property color chartBatteryHighColor: "#5ad24a"
    property color chartBatteryMediumColor: "#f2d541"
    property color chartBatteryLowColor: "#ff6b6b"
    property color chartPaiLowColor: "#f5a623"
    property color chartPaiMediumColor: "#46acea"
    property color chartPaiHighColor: "#5ad24a"
    property color chartRouteColor: "#f5a623"
    property color chartRouteStartColor: "#5ad24a"
    property color chartRouteFinishColor: "#ff6b6b"
    property color chartStressRelaxedColor: "#46acea"
    property color chartStressMildColor: "#5ad24a"
    property color chartStressModerateColor: "#f5a623"
    property color chartStressHighColor: "#ff6b6b"
    property color chartSpo2Color: "#46acea"
    property color chartSpo2SleepColor: "#1a5fb4"
    property color chartHrvColor: "#b27fe0"
    property color chartTemperatureColor: "#f5a623"

    // translucent variants of the colours above
    property color chartGoalBandColor: alpha(chartGoalColor, 0.12)
    property color chartGoalGuideColor: alpha(chartGoalColor, 0.7)
    property color chartHeartRateFillColor: alpha(chartHeartRateColor, 0.15)
    property color chartPaceFillColor: alpha(chartPaceColor, 0.15)
    property color chartElevationFillColor: alpha(chartElevationColor, 0.15)
    property color chartBatteryHighGuideColor: alpha(chartBatteryHighColor, 0.7)
    property color chartBatteryMediumGuideColor: alpha(chartBatteryMediumColor, 0.7)
    property color chartStressMildGuideColor: alpha(chartStressMildColor, 0.7)
    property color chartStressModerateGuideColor: alpha(chartStressModerateColor, 0.7)
    property color chartStressHighGuideColor: alpha(chartStressHighColor, 0.7)
    property color chartHrvFillColor: alpha(chartHrvColor, 0.15)
    property color chartTemperatureFillColor: alpha(chartTemperatureColor, 0.15)
    property var chartZoneBandColors: chartZoneColors.map(function(c) { return alpha(c, 0.12); })

    // chart lines
    property real chartGridLineWidth: 1
    property real chartLineWidth: Math.max(1.5, themeFontSizeExtraSmall / 12)
    property real chartGuideLineWidth: Math.max(1, themeFontSizeExtraSmall / 16)
    property real chartRouteLineWidth: Math.max(3, themePaddingSmall)
    property real chartDashLength: themeFontSizeExtraSmall * 0.3
    property real chartRingWidthRatio: 0.08
    property real chartGaugeWidthRatio: 0.09
}
