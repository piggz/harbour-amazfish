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
    property string iconDot: "find-location-symbolic"
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

    property string customIconPrefix: ""
    property string customIconSuffix: ""

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
}
