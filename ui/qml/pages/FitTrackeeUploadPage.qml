/*
 * Copyright (C) 2017 Jussi Nieminen, Finland
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

import QtQuick 2.2
import com.pipacs.o2 1.0
import uk.co.piggz.amazfish 1.0
import "../tools/JSTools.js" as SharedResources
import "../components/"
import "../components/platform"

PagePL {
    id: fitTrackeeDialog
    title: qsTr("FitTrackee Upload")

    property bool busy: false
    property string activityType: ""
    property string activityID: ""
    property var tcx
    property var uploadData;
    property alias activityName: st_name.text
    property alias activityDescription: st_description.text

    Column {
        id: input_fields
        width: fitTrackeeDialog.width

        TextFieldPL {
            id: st_name
            width: parent.width
            placeholderText: qsTr("Activity name for FitTrackee")
            label: qsTr("Name")
        }

        LabelPL {
            text: qsTr("Description")
        }

        TextAreaPL {
            id: st_description
            width: parent.width
            height: width * 0.6
            placeholderText: qsTr("Activity description for FitTrackee")

        }
        TextFieldPL {
            id: st_activityType
            width: parent.width
            text: activityType + "(" + SharedResources.toStravaType(activityType) + ")"
            enabled: false
            label: qsTr("Type (FitTrackee)")
        }

        TextSwitchPL {
            id: chkPrivate
            text: qsTr("Private");
        }
        TextSwitchPL {
            id: chkCommute
            text: qsTr("Commute");
        }

        ButtonPL {
            text: qsTr("Upload")
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                busy = true;
                uploadTCX();
            }
        }

        TextAreaPL {
            id: lblStatus
            readOnly: true
            width: parent.width
            height: width * 0.6
            textFormat: TextEdit.RichText

            BusyIndicatorPL {
                visible: fitTrackeeDialog.busy
                running: fitTrackeeDialog.busy
            }
        }

    }

    /*
        Uploads TCX to FitTrackee as first stage of activity upload process
    */
    function uploadTCX(){
        if (!o2fittrackee.linked){
            console.log("Not linked to FitTrackee");
            return;
        }

        var isGPX = false;
        if (tcx.indexOf("<gpx") > 0) {
            isGPX = true;
        }

        console.log("Upload TCX...");
        statusMessage(qsTr("Uploading data..."));

        var xmlhttp = new XMLHttpRequest();
        var boundary = "--------------" + (new Date).getTime();

        xmlhttp.open("POST", AmazfishConfig.fittrackeeBaseURL + "/api/workouts");
        xmlhttp = setDefaultRequestHeaders(xmlhttp)
        xmlhttp.setRequestHeader('Content-Type', 'multipart/form-data; boundary=' + boundary);

        xmlhttp.onreadystatechange=function(){
            console.log("Ready state changed:", xmlhttp.readyState, xmlhttp.responseType, xmlhttp.responseText, xmlhttp.status, xmlhttp.statusText);
            if (xmlhttp.readyState==4 && xmlhttp.status==201){
                console.log("Post Response:", xmlhttp.responseText);
                uploadData = JSON.parse(xmlhttp.responseText);
                if (uploadData["status"] === "created") {
                    statusMessage(qsTr("Activity upload complete"));
                    busy = false;
                } else {
                    requestError(xmlhttp.responseText)
                }
            } else if (xmlhttp.readyState==4 && xmlhttp.status!=201){
                requestError(xmlhttp.responseText);
            }
        };

        var dataStruct = {
            "sport_id": activityKindToFittrackee(activityType),
            "notes": "",
            "title": st_name.text,
            "description": st_description.text,
            "analysis_visibility": chkPrivate ? "private" : "public", // private, followers_only or public
            "map_visibility": chkPrivate ? "private" : "public", // private, followers_only or public
            "workout_visibility": chkPrivate ? "private" : "public", // private, followers_only or public
            "equipment_ids": []
        }

        //Create a multipart form the manual way!
        var  part ="";
        part += '\r\n--' + boundary + '\r\n';
        part += 'Content-Disposition: form-data; name="data"\r\n\r\n' + JSON.stringify(dataStruct) + '\r\n--' + boundary + '\r\n';

        part += 'Content-Disposition: form-data; name="file"; filename="' + activityID + (isGPX ? ".gpx" : ".tcx") + '"\r\n';
        part += "Content-Type: text/plain";
        part += "\r\n\r\n";
        part += tcx;
        part += "--" + boundary + "--" + "\r\n";

        console.log("Sending to fittrackee...");

        xmlhttp.send(part);
    }

    function activityKindToFittrackee(kind) {
        switch (kind) {
            case "Cycling":               return 1;  // Cycling (Sport)
            case "OutdoorCycling":        return 1;
            case "BikeCommute":           return 2;  // Cycling (Transport)
            case "Hiking":                return 3;  // Hiking
            case "MountainHike":          return 3;
            case "Cycling":               return 4;  // Mountain Biking
            case "Running":               return 5;  // Running
            case "OutdoorRunning":        return 5;
            case "Walking":               return 6;  // Walking
            case "OutdoorWalking":        return 6;
            case "EBike":                 return 7;  // Mountain Biking (Electric)
            case "TrailRun":              return 8;  // Trail
            case "Skiing":                return 9;  // Skiing (Alpine)
            case "XcClassicSki":          return 10; // Skiing (Cross Country)
            case "CrossCountrySkiing":    return 10;
            case "Rowing":                return 11; // Rowing
            case "Snowshoe":              return 12; // Snowshoes
            case "IndoorCycling":         return 13; // Cycling (Virtual)
            case "DynamicCycle":          return 13;
            case "Spinning":              return 13;
            case "Mountaineering":        return 14; // Mountaineering
            case "Paragliding":           return 15; // Paragliding
            case "OpenSwimming":          return 16; // Open Water Swimming
            case "Trekking":              return 17; // Cycling (Trekking)
            case "Multisport":            return 18; // Swimrun
            case "Kayaking":              return 19; // Kayaking
            case "Canoeing":              return 20; // Canoeing
            case "Handcycling":           return 21; // Halfbike
            case "Windsurfing":           return 22; // Windsurfing
            case "StandUpPaddleboarding": return 23; // Standup Paddleboarding
            case "Tennis":                return 24; // Tennis (Outdoor)
            case "Padel":                 return 25; // Padel (Outdoor)
            case "Rafting":               return 26; // Canoeing (Whitewater)
            case "Paddling":              return 27; // Kayaking (Whitewater)
            default:                      return 1;
        }
    }


    function setDefaultRequestHeaders(xmlhttp) {
        xmlhttp.setRequestHeader('Accept-Encoding', 'text');
        xmlhttp.setRequestHeader('Connection', 'keep-alive');
        xmlhttp.setRequestHeader('Pragma', 'no-cache');
        xmlhttp.setRequestHeader('Cache-Control', 'no-cache');
        xmlhttp.setRequestHeader('Authorization', "Bearer " + o2fittrackee.token);

        return xmlhttp;
    }

    function statusMessage(msg) {
        lblStatus.text = msg;
    }


    function requestError(strerr) {
        console.error(strerr);
        var errStatus = JSON.parse(strerr);
        if (errStatus.error_description !== null){
            statusMessage(errStatus["error_description"]);
        } else {
            statusMessage(qsTr("An unknown error occurred"));
        }
        busy = false;
    }
}
