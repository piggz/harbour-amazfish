/* -*- coding: utf-8-unix -*-
 *
 * Copyright (C) 2014 Osmo Salomaa, 2018 Rinigus
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

// Cover is Sailfish OS specific and, as a result,
// is implemented here.

CoverBackground {
    id: cover

    property bool active: status === Cover.Active

    Image {
        // Background icon
        anchors.centerIn: parent
        height: width/sourceSize.width * sourceSize.height
        opacity: 0.1
        smooth: true
        source: "image://theme/icon-m-watch"
        visible: !cover.showNarrative
        width: 1.5 * parent.width
    }

    /*
     * Default cover
     */

    Label {
        // Title
        anchors.centerIn: parent
        color: Theme.primaryColor
        font.pixelSize: Theme.fontSizeLarge
        horizontalAlignment: Text.AlignHCenter
        lineHeight: 1.25
        text: "Amazfish"
        visible: !cover.showNarrative
    }

//    CoverActionList {
//        enabled: app.initialized && app.conf.showNarrative && app.navigator.hasRoute
//    }
}
