/*
 * Copyright (C) 2017 Adam Pigg <adam@piggz.co.uk>
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
import com.pipacs.o2 1.0
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Strava settings")

    property variant athlete
    property string username: ""
    property string country: ""

    pageMenu: PageMenuPL {
        PageMenuItemPL
        {
            id: btnAuth
            iconSource: styler.iconStravaLogin !== undefined ? styler.iconStravaLogin : ""
            text: o2strava.linked ? qsTr("Logout") : qsTr("Login")
            onClicked: {
                if (o2strava.linked) {
                    o2strava.unlink();
                } else {
                    o2strava.link();
                }
            }
        }
    }

    Connections {
        target: o2strava
        onLinkedChanged: {
            if (linked) {
                var tokens = o2strava.extraTokens;
                athlete = tokens["athlete"];
                username = (athlete["username"] !== undefined) ? athlete["username"] : athlete["firstname"] + " " + athlete["lastname"];
                country = athlete["country"];
            } else {
                username = "not logged in";
                country = "";
            }
        }
    }

    onPageStatusActivating:
    {
        if (o2strava.linked) {
            var tokens = o2strava.extraTokens;
            athlete = tokens["athlete"];
            username = (athlete["username"] !== undefined) ? athlete["username"] : athlete["firstname"] + " " + athlete["lastname"];
            country = athlete["country"];
        } else {
            username = "not logged in";
            country = "";
        }
    }

    Column
    {
        id: column
        width: page.width
        spacing: styler.themePaddingLarge

        Column {
            width: parent.width
            spacing: styler.themePaddingLarge

            LabelPL{
                id: lblUser
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
                text: qsTr("User Name: ") + username
            }
            LabelPL{
                id: lblCountry
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
                text: qsTr("Country: ") + country
            }
        }
    }
}
