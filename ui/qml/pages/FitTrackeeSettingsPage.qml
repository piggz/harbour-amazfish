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
import uk.co.piggz.amazfish 1.0
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("FitTrackee settings")

    pageMenu: PageMenuPL {
        PageMenuItemPL
        {
            id: btnAuth
            iconSource: styler.iconStravaLogin !== undefined ? styler.iconStravaLogin : ""
            text: o2fittrackee.linked ? qsTr("Logout") : qsTr("Login")
            onClicked: {
                if (o2fittrackee.linked) {
                    o2fittrackee.unlink();
                } else {
                    saveSettings();
                }
            }
        }
    }

    // Connections {
    //     target: o2fittrackee
    //     onLinkedChanged: {
    //         if (linked) {
    //             var tokens = o2fittrackee.extraTokens;
    //             console.log(JSON.stringify(tokens))
    //         }
    //     }
    // }

    // onPageStatusActivating:
    // {
    //     if (o2fittrackee.linked) {
    //         var tokens = o2fittrackee.extraTokens;
    //         console.log(JSON.stringify(tokens))
    //     }
    // }

    Column
    {
        id: column
        width: page.width
        spacing: styler.themePaddingLarge

            TextFieldPL {
                id: fldBaseURL
                label: qsTr("Base URL:")
                placeholderText: "http://localhost:5000"
                width: parent.width
                enabled: !o2fittrackee.linked

            }

            TextFieldPL {
                id: fldClientID
                label: qsTr("Client ID:")
                placeholderText: "d1mAK......FI"
                width: parent.width
                enabled: !o2fittrackee.linked
            }

            TextFieldPL {
                id: fldClientSecret
                label: qsTr("Client Secret:")
                placeholderText: "5CQV7......b8"
                width: parent.width
                enabled: !o2fittrackee.linked
            }

            LabelPL {
                text: qsTr("Account linked")
                visible: o2fittrackee.linked
                horizontalAlignment: Text.AlignHCenter
                width: parent.width
            }


    }

    function saveSettings() {
        o2fittrackee.clientId = fldClientID.text
        o2fittrackee.clientSecret = fldClientSecret.text
        o2fittrackee.baseUrl = fldBaseURL.text

        AmazfishConfig.fittrackeeBaseURL = fldBaseURL.text
        AmazfishConfig.fittrackeeClientID = fldClientID.text
        AmazfishConfig.fittrackeeClientSecret = fldClientSecret.text
        o2fittrackee.link();

    }
    Component.onCompleted: {
        fldBaseURL.text = AmazfishConfig.fittrackeeBaseURL
        fldClientID.text = AmazfishConfig.fittrackeeClientID
        fldClientSecret.text = AmazfishConfig.fittrackeeClientSecret
    }
}
