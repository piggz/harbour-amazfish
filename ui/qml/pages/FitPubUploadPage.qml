import QtQuick 2.2
import uk.co.piggz.amazfish 1.0
import "../tools/JSTools.js" as SharedResources
import "../components/"
import "../components/platform"

PagePL {
    id: fitpubDialog
    title: qsTr("FitPub Upload")

    property bool busy: false
    property string activityType: ""
    property string activityID: ""
    property var tcx
    property var uploadData;
    property alias activityName: st_name.text
    property alias activityDescription: st_description.text

    Column {
        id: input_fields
        width: fitpubDialog.width

        TextFieldPL {
            id: st_name
            width: parent.width
            placeholderText: qsTr("Activity name for FitPub")
            label: qsTr("Name")
        }

        LabelPL {
            text: qsTr("Description")
        }

        TextAreaPL {
            id: st_description
            width: parent.width
            height: width * 0.6
            placeholderText: qsTr("Activity description for FitPub")
        }

        TextFieldPL {
            id: st_activityType
            width: parent.width
            text: activityType
            enabled: false
            label: qsTr("Type (FitPub)")
        }

        TextSwitchPL {
            id: chkPrivate
            text: qsTr("Private");
            checked: true
        }

        ButtonPL {
            text: qsTr("Upload")
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                busy = true;
                loginAndUpload();
            }
        }

        TextAreaPL {
            id: lblStatus
            readOnly: true
            width: parent.width
            height: width * 0.6

            BusyIndicatorPL {
                visible: fitpubDialog.busy
                running: fitpubDialog.busy
            }
        }
    }

    function statusMessage(msg) {
        lblStatus.text = msg;
    }

    function requestError(strerr) {
        console.error(strerr);
        statusMessage(strerr);
        busy = false;
    }

    function loginAndUpload() {
        if (!app.fitpubLinked) {
            statusMessage(qsTr("Not linked to FitPub"));
            busy = false;
            return;
        }

        statusMessage(qsTr("Authenticating..."));
        var loginHttp = new XMLHttpRequest();
        loginHttp.open("POST", AmazfishConfig.fitpubBaseURL + "/api/auth/login");
        loginHttp.setRequestHeader("Content-Type", "application/json; charset=utf-8");
        loginHttp.withCredentials = true;

        loginHttp.onreadystatechange = function() {
            if (loginHttp.readyState == 4) {
                if (loginHttp.status == 200) {
                    uploadTCX();
                } else {
                    requestError(qsTr("Authentication failed: ") + loginHttp.status);
                }
            }
        }
        var authData = {
            "usernameOrEmail": AmazfishConfig.fitpubUsername,
            "password": AmazfishConfig.fitpubPassword
        };
        loginHttp.send(JSON.stringify(authData));
    }

    function uploadTCX() {
        var isGPX = false;
        if (tcx.indexOf("<gpx") > 0) {
            isGPX = true;
        }

        statusMessage(qsTr("Uploading data..."));

        var xmlhttp = new XMLHttpRequest();
        var boundary = "----fitpubupload" + (new Date).getTime();

        xmlhttp.open("POST", AmazfishConfig.fitpubBaseURL + "/api/activities/upload");
        xmlhttp.withCredentials = true;
        xmlhttp.setRequestHeader('Content-Type', 'multipart/form-data; boundary=' + boundary);

        xmlhttp.onreadystatechange = function() {
            if (xmlhttp.readyState == 4) {
                if (xmlhttp.status >= 200 && xmlhttp.status < 300) {
                    var responseData = JSON.parse(xmlhttp.responseText);
                    updateProperties(responseData);
                } else {
                    requestError(qsTr("Upload failed: ") + xmlhttp.status + "\n" + xmlhttp.responseText);
                }
            }
        };

        var part = "";
        part += "--" + boundary + "\r\n";
        part += 'Content-Disposition: form-data; name="file"; filename="' + activityID + (isGPX ? ".gpx" : ".tcx") + '"\r\n';
        part += "Content-Type: " + (isGPX ? "application/gpx+xml" : "application/vnd.garmin.tcx+xml") + "\r\n\r\n";
        part += tcx;
        part += "\r\n--" + boundary + "--\r\n";

        xmlhttp.send(part);
    }

    function updateProperties(activityData) {
        statusMessage(qsTr("Updating activity metadata..."));

        var xmlhttp = new XMLHttpRequest();
        xmlhttp.open("PUT", AmazfishConfig.fitpubBaseURL + "/api/activities/" + activityData["id"]);
        xmlhttp.withCredentials = true;
        xmlhttp.setRequestHeader("Content-Type", "application/json; charset=utf-8");

        xmlhttp.onreadystatechange = function() {
            if (xmlhttp.readyState == 4) {
                if (xmlhttp.status >= 200 && xmlhttp.status < 300) {
                    statusMessage(qsTr("Activity upload complete"));
                    busy = false;
                } else {
                    requestError(qsTr("Update failed: ") + xmlhttp.status + "\n" + xmlhttp.responseText);
                }
            }
        }

        var visibility = chkPrivate.checked ? "PRIVATE" : "PUBLIC";
        var updateData = {
            "activityType": activityData["activityType"] ? activityData["activityType"].toUpperCase() : "RUNNING",
            "description": st_description.text,
            "race": activityData["race"] !== undefined ? activityData["race"] : false,
            "title": st_name.text !== "" ? st_name.text : (activityData["title"] || "Activity"),
            "visibility": visibility
        };

        xmlhttp.send(JSON.stringify(updateData));
    }
}
