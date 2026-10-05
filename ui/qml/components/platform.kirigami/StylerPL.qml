/* -*- coding: utf-8-unix -*-
 *
 * Copyright (C) 2018-2019 Rinigus, 2019 Purism SPC
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
import org.kde.kirigami 2.5 as Kirigami

QtObject {
    // font sizes and family
    property string themeFontFamily: Kirigami.Theme.defaultFont
    property string themeFontFamilyHeading: Kirigami.Theme.defaultFont
    property int  themeFontSizeHuge: Math.round(themeFontSizeMedium*3.0)
    property int  themeFontSizeExtraLarge: Math.round(themeFontSizeMedium*2.0)
    property int  themeFontSizeLarge: Math.round(themeFontSizeMedium*1.5)
    property int  themeFontSizeMedium: Math.round(Qt.application.font.pixelSize*1.0)
    property int  themeFontSizeSmall: Math.round(themeFontSizeMedium*0.9)
    property int  themeFontSizeExtraSmall: Math.round(themeFontSizeMedium*0.7)
    property real themeFontSizeOnMap: themeFontSizeSmall

    // colors
    // block background (navigation, poi panel, bubble)
    property color blockBg: Kirigami.Theme.backgroundColor
    // variant of navigation icons
    property string navigationIconsVariant: darkTheme ? "white" : "black"
    // descriptive items
    property color themeHighlightColor: Kirigami.Theme.textColor
    // navigation items, primary
    property color themePrimaryColor: Kirigami.Theme.textColor
    // navigation items, secondary
    property color themeSecondaryColor: Kirigami.Theme.textColor
    // descriptive items, secondary
    property color themeSecondaryHighlightColor: Kirigami.Theme.disabledTextColor

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
    property string iconEdit: "document-edit-symbolic"
    property string iconEditClear: "edit-clear-symbolic"
    property string iconFavorite: "bookmark-new-symbolic"
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

    property string iconForward: "icon-m-forward"
    property string iconBackward: "icon-m-back"
    property string iconContact: "icon-m-contact"
    property string iconWatch: "icon-m-watch"
    property string iconLevels: "icon-m-levels"
    property string iconAlarm: "icon-m-alarm"
    property string iconNotifications: "icon-m-notifications"
    property string iconWeather: "icon-m-weather-d212-light"
    property string iconDiagnostic: "icon-m-diagnostic"
    property string iconFavoriteSelected: "icon-m-favorite-selected"
    property string iconBattery: "icon-m-battery"
    property string iconBluetooth: "icon-m-bluetooth-device"
    property string iconSteps: "icon-m-steps"
    property string iconHeartrate: "icon-m-heartrate"
    property string iconUp: "icon-m-up"
    property string iconDown: "icon-m-down"
    property string iconUpDown: "icon-m-up-down"
    property string iconClock: "icon-m-clock"
    property string iconLocation: "icon-m-location"
    property string iconStrava: "icon-strava"

    property string activityIconPrefix: "qrc:///qml/activity-icons/"
    property string customIconPrefix: "qrc:///qml/custom-icons/"
    property string customIconSuffix: ".png"

    // item sizes
    property real themeItemSizeLarge: themeItemSizeSmall * 2
    property real themeItemSizeSmall: Kirigami.Units.gridUnit * 2.5
    property real themeItemSizeExtraSmall: themeItemSizeSmall * 0.75

    // paddings and page margins
    property real themeHorizontalPageMargin: Kirigami.Units.largeSpacing * 2
    property real themePaddingLarge: Kirigami.Units.largeSpacing * 2
    property real themePaddingMedium: Kirigami.Units.largeSpacing * 1
    property real themePaddingSmall: Kirigami.Units.smallSpacing

    property real themePixelRatio: 1 //Screen.devicePixelRatio

    property bool darkTheme: (Kirigami.Theme.backgroundColor.r + Kirigami.Theme.backgroundColor.g +
                              Kirigami.Theme.backgroundColor.b) <
                             (Kirigami.Theme.textColor.r + Kirigami.Theme.textColor.g +
                              Kirigami.Theme.textColor.b)

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

    // returns the colour c with the opacity a
    function alpha(c, a) {
        return Qt.rgba(c.r, c.g, c.b, a);
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

    // opacities applied to the colours above
    property real chartFillOpacity: 0.15
    property real chartBandOpacity: 0.12
    property real chartGuideOpacity: 0.7

    // chart lines
    property real chartGridLineWidth: 1
    property real chartLineWidth: Math.max(1.5, themeFontSizeExtraSmall / 12)
    property real chartGuideLineWidth: Math.max(1, themeFontSizeExtraSmall / 16)
    property real chartRouteLineWidth: Math.max(3, themePaddingSmall)
    property real chartDashLength: themeFontSizeExtraSmall * 0.3
    property real chartRingWidthRatio: 0.08
    property real chartGaugeWidthRatio: 0.09
}
