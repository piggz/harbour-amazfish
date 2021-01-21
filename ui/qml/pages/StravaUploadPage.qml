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
import "../tools/JSTools.js" as SharedResources
import "../components/"
import "../components/platform"

PagePL {
    id: stravaDialog
    title: qsTr("Strava Upload")

    property bool busy: false
    property string activityType: ""
    property string activityID: ""
    property var tcx
    property var uploadData;
    property alias activityName: st_name.text
    property alias activityDescription: st_description.text

    Column {
        id: input_fields
        width: stravaDialog.width

        TextFieldPL {
            id: st_name
            width: parent.width
            placeholderText: qsTr("Activity name for Strava")
            label: qsTr("Name")
        }

        LabelPL {
            text: qsTr("Description")
        }

        TextAreaPL {
            id: st_description
            width: parent.width
            height: width * 0.6
            placeholderText: qsTr("Activity description for Strava")

        }
        TextFieldPL {
            id: st_activityType
            width: parent.width
            text: activityType + "(" + SharedResources.toStravaType(activityType) + ")"
            enabled: false
            label: qsTr("Type (Strava)")
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

            BusyIndicatorPL {
                visible: stravaDialog.busy
                running: stravaDialog.busy
            }
        }

        Timer {
            id: tmrStatusCheck
            running: false
            repeat: true
            interval: 2000
            onTriggered: {
                checkUploadStatus();
            }
        }

        Timer {
            id: tmrUpdateActivity
            running: false
            repeat: false
            interval: 2000
            onTriggered: {
                updateActivity();
            }
        }

    }

    /*
        Uploads TCX to Strava as first stage of activity upload process
    */
    function uploadTCX(){
        if (!o2strava.linked){
            console.log("Not linked to Strava");
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

        xmlhttp.open("POST", "https://www.strava.com/api/v3/uploads");
        xmlhttp = setDefaultRequestHeaders(xmlhttp)
        xmlhttp.setRequestHeader('Content-Type', 'multipart/form-data; boundary=' + boundary);

        xmlhttp.onreadystatechange=function(){
            console.log("Ready state changed:", xmlhttp.readyState, xmlhttp.responseType, xmlhttp.responseText, xmlhttp.status, xmlhttp.statusText);
            if (xmlhttp.readyState==4 && xmlhttp.status==201){
                console.log("Post Response:", xmlhttp.responseText);
                uploadData = JSON.parse(xmlhttp.responseText);
                if (uploadData["error"] === null){
                    console.log("Upload ID:", uploadData["id"]);
                    tmrStatusCheck.start();
                    statusMessage(qsTr("Checking upload..."));
                }
                else{
                    console.log(xmlhttp.responseText);
                    console.log("TCX Import error, cannot save exercise");
                    statusMessage(uploadData["error"]);
                    busy = false;
                }
            }
            else if (xmlhttp.readyState==4 && xmlhttp.status!=201){
                var strerr = xmlhttp.responseText;
                requestError(strerr);

            }
        };

        //Create a multipart form the manual way!
        var  part ="";
        part += '\r\n--' + boundary + '\r\n';
        part += 'Content-Disposition: form-data; name="name"\r\n\r\n' + st_name.text + '\r\n--' + boundary + '\r\n';
        part += 'Content-Disposition: form-data; name="type"\r\n\r\n' + SharedResources.toStravaType(activityType) + '\r\n--' + boundary + '\r\n';
        part += 'Content-Disposition: form-data; name="description"\r\n\r\n' + st_description.text + '\r\n--' + boundary + '\r\n';
        part += 'Content-Disposition: form-data; name="private"\r\n\r\n' + (chkPrivate.checked ? "1" : "0") + '\r\n--' + boundary + '\r\n';
        part += 'Content-Disposition: form-data; name="commute""\r\n\r\n' + (chkCommute.checked ? "1" : "0") + '\r\n--' + boundary + '\r\n';
        part += 'Content-Disposition: form-data; name="data_type"\r\n\r\n' + (isGPX ? "gpx": "tcx") + '\r\n--' + boundary + '\r\n';
        part += 'Content-Disposition: form-data; name="external_id"\r\n\r\n' + activityID + '\r\n--' + boundary + '\r\n';
        part += 'Content-Disposition: form-data; name="file"; filename="' + activityID + '"\r\n';
        part += "Content-Type: text/plain";
        part += "\r\n\r\n";
        part += tcx;
        part += "--" + boundary + "--" + "\r\n";

        console.log("Sending to strava...");

        xmlhttp.send(part);
    }

    function checkUploadStatus() {
        var xmlhttp = new XMLHttpRequest();

        if (!isNumeric(uploadData["id"])) {
            console.log("No upload id")
            busy = false;
            tmrStatusCheck.stop();
            return;
        }

        xmlhttp.open("GET", "https://www.strava.com/api/v3/uploads/" + uploadData.id);
        xmlhttp = setDefaultRequestHeaders(xmlhttp)
        xmlhttp.setRequestHeader('Accept', 'application/json, text/plain, */*');
        xmlhttp.setRequestHeader('Content-Type', 'application/json');

        xmlhttp.onreadystatechange=function(){
            console.log("Ready state changed:", xmlhttp.readyState, xmlhttp.responseType, xmlhttp.responseText, xmlhttp.status, xmlhttp.statusText);
            if (xmlhttp.readyState==4 && xmlhttp.status==200){
                console.log("Post Response:", xmlhttp.responseText);
                uploadData = JSON.parse(xmlhttp.responseText);
                if (uploadData["error"] === null){
                    console.log("Activity ID:", uploadData.activity_id);
                    if (isNumeric(uploadData.activity_id)) { //Upload is complete
                        tmrStatusCheck.stop();
                        tmrUpdateActivity.start();
                        console.log("TCX uploaded...")
                        statusMessage(qsTr("TCX uploaded..."));
                    }
                }
                else{
                    console.log(xmlhttp.responseText);
                    tmrStatusCheck.stop();
                    busy = false;
                    statusMessage(uploadData["error"]);
                }
            }
            else if (xmlhttp.readyState==4 && xmlhttp.status!=200){
                var strerr = xmlhttp.responseText;
                requestError(strerr);
            }
        };

        xmlhttp.send();
    }

    function updateActivity() {
        var xmlhttp = new XMLHttpRequest();

        xmlhttp.open("PUT", "https://www.strava.com/api/v3/activities/" + uploadData.activity_id);
        xmlhttp = setDefaultRequestHeaders(xmlhttp)
        xmlhttp.setRequestHeader('Content-Type', 'application/json');
        xmlhttp.setRequestHeader('Accept', 'application/json, text/plain, */*');

        xmlhttp.onreadystatechange=function(){
            if (xmlhttp.readyState==4) {
                if (xmlhttp.status==200) {
                    uploadData = JSON.parse(xmlhttp.responseText);
                    tmrUpdateActivity.stop();
                    busy = false;
                    if (uploadData.error === undefined) {
                        console.log("Activity upload complete")
                        statusMessage(qsTr("Activity upload complete"));
                    } else {
                        console.log(xmlhttp.responseText);
                        statusMessage(uploadData["error"]);
                    }
                } else {
                    var strerr = xmlhttp.responseText;
                    requestError(strerr);
                }
            }
        };

        var JsonString = '{
            "commute":'+(chkCommute.checked ? "1" : "0")+',
            "type":"'+SharedResources.toStravaType(activityType)+'"
        }';

        xmlhttp.send(JsonString);
    }

    function setDefaultRequestHeaders(xmlhttp) {
        xmlhttp.setRequestHeader('Accept-Encoding', 'text');
        xmlhttp.setRequestHeader('Connection', 'keep-alive');
        xmlhttp.setRequestHeader('Pragma', 'no-cache');
        xmlhttp.setRequestHeader('Cache-Control', 'no-cache');
        xmlhttp.setRequestHeader('Authorization', "Bearer " + o2strava.token);

        return xmlhttp;
    }

    function statusMessage(msg) {
        lblStatus.text = msg;
    }

    function isNumeric(n) {
        return !isNaN(parseFloat(n)) && isFinite(n);
    }

    function requestError(strerr) {
        console.log(strerr);
        console.log("Some kind of error happened");
        var errStatus = JSON.parse(xmlhttp.responseText);
        console.log(errStatus);
        if (errStatus.message !== null){
            statusMessage(errStatus["message"]);
        } else {
            statusMessage(qsTr("An unknown error occurred"));
        }
        tmrUpdateActivity.stop();
        tmrStatusCheck.stop();
        busy = false;
    }
}
