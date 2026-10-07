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
import Sailfish.Silica 1.0

QtObject {
    readonly property bool isSilica: true
    
    // font sizes and family
    property string themeFontFamily: Theme.fontFamily
    property string themeFontFamilyHeading: Theme.fontFamilyHeading
    property int  themeFontSizeHuge: Theme.fontSizeHuge
    property int  themeFontSizeExtraLarge: Theme.fontSizeExtraLarge
    property int  themeFontSizeLarge: Theme.fontSizeLarge
    property int  themeFontSizeMedium: Theme.fontSizeMedium
    property int  themeFontSizeSmall: Theme.fontSizeSmall
    property int  themeFontSizeExtraSmall: Theme.fontSizeExtraSmall
    property real themeFontSizeOnMap: themeFontSizeExtraSmall * 0.75

    // colors
    // block background (navigation, poi panel, bubble)
    property color blockBg: Theme.overlayBackgroundColor || "#e6000000"
    // variant of navigation icons
    property string navigationIconsVariant: Theme.colorScheme ? "black" : "white"
    // descriptive items
    property color themeHighlightColor: Theme.highlightColor
    // navigation items (to be clicked)
    property color themePrimaryColor: Theme.primaryColor
    // navigation items, secondary
    property color themeSecondaryColor: Theme.secondaryColor
    // descriptive items, secondary
    property color themeSecondaryHighlightColor: Theme.secondaryHighlightColor

    // button sizes
    property real themeButtonWidthLarge: Theme.buttonWidthLarge
    property real themeButtonWidthMedium: Theme.buttonWidthMedium

    // icon sizes
    property real themeIconSizeLarge: Theme.iconSizeLarge
    property real themeIconSizeMedium: Theme.iconSizeMedium
    property real themeIconSizeSmall: Theme.iconSizeSmallPlus

    // used icons
    property string iconAbout: "image://theme/icon-m-about"
    property string iconBack: "image://theme/icon-m-back"
    property string iconClear: "image://theme/icon-m-clear"
    property string iconClose: "image://theme/icon-m-dismiss"
    property string iconDelete: "image://theme/icon-m-delete"
    property string iconEdit: "image://theme/icon-m-edit"
    property string iconEditClear: "image://theme/icon-m-clear"
    property string iconFavorite: "image://theme/icon-m-favorite"
    property string iconMenu: "image://theme/icon-m-menu"
    property string iconPause: "image://theme/icon-m-pause"
    property string iconPhone: "image://theme/icon-m-phone"
    property string iconPreferences: "image://theme/icon-m-developer-mode"
    property string iconRefresh: "image://theme/icon-m-refresh"
    property string iconSave: "image://theme/icon-m-acknowledge"
    property string iconSearch: "image://theme/icon-m-search"
    property string iconShare: "image://theme/icon-m-share"
    property string iconStart: "image://theme/icon-m-play"
    property string iconStop: "image://theme/icon-m-clear"
    property string iconWebLink: "image://theme/icon-m-link"

    property string iconForward: "image://theme/icon-m-forward"
    property string iconBackward: "image://theme/icon-m-back"
    property string iconContact: "image://theme/icon-m-contact"
    property string iconWatch: "image://theme/icon-m-watch"
    property string iconLevels: "image://theme/icon-m-levels"
    property string iconAlarm: "image://theme/icon-m-alarm"
    property string iconNotifications: "image://theme/icon-m-notifications"
    property string iconWeather: "image://theme/icon-m-weather-d212-light"
    property string iconDiagnostic: "image://theme/icon-m-diagnostic"
    property string iconFavoriteSelected: "image://theme/icon-m-favorite-selected"
    property string iconBattery: "image://theme/icon-m-battery"
    property string iconBluetooth: "image://theme/icon-m-bluetooth-device"
    property string iconSteps: "qrc:///qml/custom-icons/icon-m-steps.png"
    property string iconHeartrate: "qrc:///qml/custom-icons/icon-m-heartrate.png"
    property string iconUp: "image://theme/icon-m-up"
    property string iconDown: "image://theme/icon-m-down"
    property string iconUpDown: "qrc:///qml/custom-icons/icon-m-up-down.png"
    property string iconClock: "image://theme/icon-m-clock"
    property string iconLocation: "image://theme/icon-m-location"
    property string iconStrava: "qrc:///qml/custom-icons/icon-strava.png"
    property string activityIconPrefix: "qrc:///qml/activity-icons/"
    property string customIconPrefix: "qrc:///qml/custom-icons/"
    property string customIconSuffix: ".png"

    // item sizes
    property real themeItemSizeLarge: Theme.itemSizeLarge
    property real themeItemSizeSmall: Theme.itemSizeSmall
    property real themeItemSizeExtraSmall: Theme.itemSizeExtraSmall

    // paddings and page margins
    property real themeHorizontalPageMargin: Theme.horizontalPageMargin
    property real themePaddingLarge: Theme.paddingLarge
    property real themePaddingMedium: Theme.paddingMedium
    property real themePaddingSmall: Theme.paddingSmall

    property real themePixelRatio: Theme.pixelRatio

    // surfaces of tiles, cards and grouped lists
    property color surfaceColor: Theme.rgba(Theme.highlightDimmerColor, 0.5)
    property color surfaceBorderColor: Theme.rgba(themePrimaryColor, 0.1)
    property color surfaceEdgeColor: Theme.rgba(themePrimaryColor, 0.08)
    property color surfacePressedColor: Theme.rgba(themePrimaryColor, 0.08)
    property color surfaceChipColor: Theme.rgba(themePrimaryColor, 0.08)
    property color surfaceDividerColor: Theme.rgba(themePrimaryColor, 0.08)
    property color surfaceTrackColor: Theme.rgba(themePrimaryColor, 0.12)
    property real surfaceRadius: Math.round(themePaddingLarge * 1.25)

    // structure of the charts
    property color chartGridColor: Theme.rgba(themeSecondaryColor, 0.25)
    property color chartGridFaintColor: Theme.rgba(themeSecondaryColor, 0.12)
    property color chartEmptyColor: Theme.rgba(themeSecondaryColor, 0.1)
    property color chartInactiveColor: Theme.rgba(themeSecondaryColor, 0.35)
    property color chartAverageColor: Theme.rgba(themePrimaryColor, 0.9)
    property color chartLabelBackgroundColor: Theme.rgba(Theme.highlightDimmerColor, 0.75)

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

    // translucent variants of the colours above
    property color chartGoalBandColor: Theme.rgba(chartGoalColor, 0.12)
    property color chartGoalGuideColor: Theme.rgba(chartGoalColor, 0.7)
    property color chartHeartRateFillColor: Theme.rgba(chartHeartRateColor, 0.15)
    property color chartPaceFillColor: Theme.rgba(chartPaceColor, 0.15)
    property color chartElevationFillColor: Theme.rgba(chartElevationColor, 0.15)
    property color chartBatteryHighGuideColor: Theme.rgba(chartBatteryHighColor, 0.7)
    property color chartBatteryMediumGuideColor: Theme.rgba(chartBatteryMediumColor, 0.7)
    property var chartZoneBandColors: chartZoneColors.map(function(c) { return Theme.rgba(c, 0.12); })

    // chart lines
    property real chartGridLineWidth: 1
    property real chartLineWidth: Math.max(1.5, themeFontSizeExtraSmall / 12)
    property real chartGuideLineWidth: Math.max(1, themeFontSizeExtraSmall / 16)
    property real chartRouteLineWidth: Math.max(3, themePaddingSmall)
    property real chartDashLength: themeFontSizeExtraSmall * 0.3
    property real chartRingWidthRatio: 0.08
    property real chartGaugeWidthRatio: 0.09
}
