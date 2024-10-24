import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Heartrate")

    property var day: new Date()
    property var relaxed: 0
    property var light: 0
    property var intensive: 0
    property var aerobic: 0
    property var anerobic: 0
    property var vo2max: 0
    property var total: relaxed + light + intensive + aerobic + anerobic + vo2max
    property var minhr: 0
    property var maxhr: 0

    pageMenu: PageMenuPL {
        DownloadDataMenuItem{}
    }

    Column {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        LabelPL {
            id: lblCurrentHeartrate
            font.pixelSize: styler.themeFontSizeExtraLarge * 3
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("%1 bpm").arg(_InfoHeartrate)
            horizontalAlignment: Text.AlignHCenter
        }

        Row { //Min and Max HR
            //height: childrenRect.height
            anchors.horizontalCenter: parent.horizontalCenter
            IconPL { iconName: styler.iconDown; iconHeight: styler.themeIconSizeSmall; iconWidth: styler.themeIconSizeSmall }
            LabelPL { text: minhr; anchors.verticalCenter: parent.verticalCenter
            }
            IconPL { iconName: styler.iconUp; iconHeight: styler.themeIconSizeSmall; iconWidth: styler.themeIconSizeSmall }
            LabelPL { text: maxhr; anchors.verticalCenter: parent.verticalCenter
            }
        }

        DateNavigation {
            text: day.toDateString();
            onBackward: {
                day.setDate(day.getDate() - 1);
                text = day.toDateString();
                updateGraphs();
            }
            onForward: {
                day.setDate(day.getDate() + 1);
                text = day.toDateString();
                updateGraphs();
            }
        }

        Graph {
            id: graphHR
            graphTitle: qsTr("BPM")
            graphHeight: 300

            axisY.units: "BPM"
            type: DataSource.Heartrate
            graphType: 2

            minY: 0
            maxY: 200
            valueConverter: function(value) {
                return value.toFixed(0);
            }
            onClicked: {
                updateGraph(day);
            }
        }

        //Type summary
        Grid {
            columns: 3
            spacing: styler.themePaddingMedium
            width: parent.width - (styler.themePaddingMedium * 2)
            LabelPL {text: qsTr("Relaxed")}
            Item { 
                width: parent.width * 0.5
                height: 50
                Rectangle { color: "grey"; width: parent.width * (relaxed  / total) ; height: parent.height }
                LabelPL { text: Math.round((relaxed / total) * 100) + "%"; anchors.centerIn: parent}
            }
            LabelPL {text: qsTr("<= %1 BPM".arg(Math.round(maxHR()*0.5)))}

            LabelPL {text: qsTr("Light")}
            Item {
                width: parent.width * 0.5
                height: 50
                Rectangle { color: "lightblue"; width: parent.width * (light  / total) ; height: parent.height }
                LabelPL { text: Math.round((light / total) * 100) + "%"; anchors.centerIn: parent}
            }
            LabelPL {text: qsTr("<= %1 BPM".arg(Math.round(maxHR()*0.6)))}

            LabelPL {text: qsTr("Intensive")}
            Item {
                width: parent.width * 0.5
                height: 50
                Rectangle { color: "green"; width: parent.width * (intensive  / total) ; height: parent.height }
                LabelPL { text: Math.round((intensive / total) * 100) + "%"; anchors.centerIn: parent}
            }
            LabelPL {text: qsTr("<= %1 BPM".arg(Math.round(maxHR()*0.7)))}

            LabelPL {text: qsTr("Aerobic")}
            Item {
                width: parent.width * 0.5
                height: 50
                Rectangle { color: "yellow"; width: parent.width * (aerobic  / total) ; height: parent.height }
                LabelPL { text: Math.round((aerobic / total) * 100) + "%"; anchors.centerIn: parent}
            }
            LabelPL {text: qsTr("<= %1 BPM".arg(Math.round(maxHR()*0.8)))}

            LabelPL {text: qsTr("Anerobic")}
            Item {
                width: parent.width * 0.5
                height: 50
                Rectangle { color: "orange"; width: parent.width * (anerobic  / total) ; height: parent.height }
                LabelPL { text: Math.round((anerobic / total) * 100) + "%"; anchors.centerIn: parent}
            }
            LabelPL {text: qsTr("<= %1 BPM".arg(Math.round(maxHR()*0.9)))}

            LabelPL {text: qsTr("VO2 Max")}
            Item {
                width: parent.width * 0.5
                height: 50
                Rectangle { color: "red"; width: parent.width * (vo2max  / total) ; height: parent.height }
                LabelPL { text: Math.round((vo2max / total) * 100) + "%"; anchors.centerIn: parent}
            }
            LabelPL {text: qsTr("<= %1 BPM".arg(Math.round(maxHR())))}
        }
    }

    function updateGraphs() {
        graphHR.updateGraph(day);
        calculateZones();
    }

    function calculateZones() {
        var points = graphHR.points;
        var end = points.length;
        var maxHRforAge = maxHR();

        relaxed = 0;
        light = 0;
        intensive = 0;
        aerobic = 0;
        anerobic = 0;
        vo2max = 0;

        minhr = 0;
        maxhr = 0;
        for (var i = 0; i < end; i++) {
            if (points[i].y >= (maxHRforAge * 0.9)) {
                vo2max++;
            } else if (points[i].y >= (maxHRforAge * 0.8)) {
                anerobic++;
            } else if (points[i].y >= (maxHRforAge * 0.7)) {
                aerobic++
            } else if (points[i].y >= (maxHRforAge * 0.6)) {
                intensive++;
            } else if (points[i].y >= (maxHRforAge * 0.5)) {
                light++;
            } else {
                relaxed++;
            }
            if (points[i].y > maxhr) {
                maxhr = points[i].y;
            }
            if (minhr == 0) {
                minhr = points[i].y;
            }

            if (points[i].y > 0 && points[i].y < minhr)  {
                minhr = points[i].y;
            }
        }
        console.log("relaxed:", relaxed);
        console.log("light:", light);
        console.log("intensive:", intensive);
        console.log("aerobic:", aerobic);
        console.log("anerobic:", anerobic);
        console.log("vo2max:", vo2max);

    }

    function maxHR() {
        var dob = AmazfishConfig.profileDOB;
        var diff_ms = Date.now() - dob.getTime();
        var age_dt = new Date(diff_ms);
        var age = Math.abs(age_dt.getUTCFullYear() - 1970);
        var max_hr = 200;

        if (age >= 70) {
            max_hr = 150;
        } else if (age >= 65) {
            max_hr = 155;
        } else if (age >= 60) {
            max_hr = 160;
        } else if (age >= 55) {
            max_hr = 165;
        } else if (age >= 50) {
            max_hr = 170;
        } else if (age >= 45) {
            max_hr = 175;
        } else if (age >= 40) {
            max_hr = 180;
        } else if (age >= 35) {
            max_hr = 185;
        } else if (age >= 30) {
            max_hr = 190;
        } else if (age >= 25) {
            max_hr = 195;
        } else {
            max_hr = 200;
        }

        console.log("Age is", age, "max hr is", max_hr);

        return max_hr;
    }

    Component.onCompleted: {
        updateGraphs();
        DaemonInterfaceInstance.requestManualHeartrate();
    }

    onPageStatusActive: {
        pushAttached(Qt.resolvedUrl("SportsSummaryPage.qml"))
    }
}
