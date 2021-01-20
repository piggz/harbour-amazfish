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

    property bool stravaLinked: false
    property bool downloadingGPX: false
    property variant athlete
    property string username: ""
    property string country: ""

    pageMenu: PageMenuPL {
        PageMenuItemPL
        {
            id: btnAuth
            text: stravaLinked ? qsTr("Logout") : qsTr("Login")
            onClicked: {
                enabled = false
                if (o2strava.linked) {
                    o2strava.unlink();
                } else {
                    o2strava.link();
                }
            }
        }
    }

    onStravaLinkedChanged: {
        if (stravaLinked) {
            var tokens = o2strava.extraTokens;
            athlete = tokens["athlete"];
            username = (athlete["username"] !== undefined) ? athlete["username"] : athlete["firstname"] + " " + athlete["lastname"];
            country = athlete["country"];
        } else {
            username = "not logged in";
            country = "";
        }
    }

    onPageStatusActivating:
    {
        stravaLinked = o2strava.linked
    }

    O2 {
        id: o2strava
        clientId: STRAVA_CLIENT_ID
        clientSecret: STRAVA_CLIENT_SECRET
        scope: "activity:write,activity:read_all"
        requestUrl: "https://www.strava.com/oauth/authorize"
        tokenUrl: "https://www.strava.com/oauth/token"
        refreshTokenUrl: "https://www.strava.com/api/v3/oauth/token"

        onOpenBrowser: {
            Qt.openUrlExternally(url);
        }

        onCloseBrowser: {
        }

        onLinkedChanged: {
            btnAuth.enabled = true;
            stravaLinked = linked
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
