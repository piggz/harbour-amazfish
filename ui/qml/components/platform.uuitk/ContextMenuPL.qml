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
import Lomiri.Components 1.3

ListItemActions {
    id: root

    // Forward children explicitly
    default property alias actionsList: root.actions

    // skipping context menu in UUITK for now
    // TODO: fix
    // otherwise was getting all kind of errors on absence of
    // "enabled" property on UUITK setup and complains regarding FINAL property on
    // desktop
    function open() {
        console.log("Not implemented")
    }
}
