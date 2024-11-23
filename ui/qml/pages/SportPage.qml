import QtQuick 2.0
import QtPositioning 5.3
import MapboxMap 1.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"
import "../tools/JSTools.js" as JSTools

PagePL {
    id: page

    property string date: ""
    property string duration: ""
    property string location: ""
    property string startdate: ""
    property string kindstring: ""
    property string activitytitle: ""
    property string tcx: ""

    property bool bMapMaximized: false
    property alias loader: trackLoader

    title: activitytitle

    pageMenu: PageMenuPL {
        PageMenuItemPL
        {
            iconSource: styler.iconUploadToStrava !== undefined ? styler.iconUploadToStrava : ""
            text: qsTr("Send to Strava")
            visible: o2strava.linked
            onClicked: {
                var dialog = app.pages.push(Qt.resolvedUrl("StravaUploadPage.qml"));
                dialog.activityID = activitytitle.replace(/\s/g, '');
                dialog.tcx = tcx;
                dialog.activityName = activitytitle;
                dialog.activityDescription = trackLoader.description;
                dialog.activityType = kindstring
            }
        }
    }

    Item {
        id: pageItem
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
        height: app.height

        GridLayout {
            id: grid
            columns: 2
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: styler.themePaddingLarge
            Layout.preferredHeight: page.height * 0.66

            IconPL
            {
                id: workoutImage
                Layout.rowSpan: 3
                Layout.preferredWidth: styler.themeItemSizeLarge
                Layout.preferredHeight: styler.themeItemSizeLarge
                Layout.alignment: Qt.AlignLeft
                iconName: styler.customIconPrefix + "icon-m-" + getKindString(kindstring) + styler.customIconSuffix
            }

            LabelPL
            {
                id: dateLabel
                text: startdate
            }

            LabelPL
            {
                id: durationLabel
                text: duration
            }

            LabelPL
            {
                id: locationLabel
                text: location
            }

            LabelPL {
                id: dataLabel
                Layout.rowSpan: 2
                text: qsTr("Data:")
            }
        }

        ListViewPL {
            id: listView
            width: parent.width
            height: app.height / 3
            clip: true
            anchors.top: grid.bottom
            anchors.margins: styler.themePaddingLarge

            model: SportsMeta

            delegate: ListItemPL {
                id: listItem
                height: keyLabel.height + styler.themePaddingMedium

                LabelPL
                {
                    id: keyLabel
                    width: page.width / 2
                    anchors.topMargin: styler.themePaddingMedium
                    anchors.left: parent.left
                    anchors.leftMargin: styler.themePaddingMedium
                    text: translateSportKey(key)
                }

                LabelPL
                {
                    id: valueLabel
                    width: page.width / 3
                    anchors.top: parent.top
                    anchors.left: keyLabel.right
                    anchors.leftMargin: styler.themePaddingMedium
                    text: value + " " + translateSportUnit(unit)
                }
            }
        }


        MapboxMap
        {
            id: map

            width: pageItem.width
            anchors.top: bMapMaximized ? pageItem.top : listView.bottom
            anchors.left: pageItem.left
            anchors.right: pageItem.right
            anchors.bottom: pageItem.bottom

            //height: bMapMaximized ? page.height : (page.height - (listView.y + listView.height))

            center: QtPositioning.coordinate(51.9854, 9.2743)
            zoomLevel: 8.0
            minimumZoomLevel: 0
            maximumZoomLevel: 20
            pixelRatio: 3.0

            accessToken: "pk.eyJ1IjoiamRyZXNjaGVyIiwiYSI6ImNqYmVta256YTJsdjUzMm1yOXU0cmxibGoifQ.JiMiONJkWdr0mVIjajIFZQ"
            cacheDatabaseDefaultPath: true
            styleUrl: "mapbox://styles/mapbox/outdoors-v11"

            visible: true

            Behavior on height {
                NumberAnimation { duration: 150 }
            }

            Item
            {
                id: centerButton
                anchors.left: parent.left
                anchors.leftMargin: styler.themePaddingSmall
                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingSmall
                width: parent.width / 10
                height: parent.width / 10
                visible: true
                z: 200

                MouseArea
                {
                    anchors.fill: parent
                    onReleased:
                    {
                        console.log("centerButton pressed");

                        var trackPointsTemporary = [];

                        //Go through array with track data points
                        for (var i=0; i<JSTools.trackPointsAt.length; i++)
                        {
                            //add this track point to temporary array. This will be used for drawing the track line
                            trackPointsTemporary.push(JSTools.trackPointsAt[i]);
                        }
                        map.fitView(trackPointsTemporary);
                    }
                }
                Image
                {
                    anchors.fill: parent
                    source: "../pics/map_btn_center.png"
                }
            }
            Item
            {
                id: minmaxButton
                anchors.right: parent.right
                anchors.rightMargin: styler.themePaddingSmall
                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingSmall
                width: parent.width / 10
                height: parent.width / 10
                visible: true
                z: 200

                MouseArea
                {
                    anchors.fill: parent
                    onReleased:
                    {
                        console.log("minmaxButton pressed");
                        bMapMaximized = !bMapMaximized;
                    }
                }
                Image
                {
                    anchors.fill: parent
                    source: (map.height === page.height) ? "../pics/map_btn_min.png" : "../pics/map_btn_max.png"
                }
            }

            MapboxMapGestureArea
            {
                id: mouseArea
                map: map
                activeClickedGeo: true
                activeDoubleClickedGeo: true
                activePressAndHoldGeo: false

                onDoubleClicked:
                {
                    //console.log("onDoubleClicked: " + mouse)
                    map.setZoomLevel(map.zoomLevel + 1, Qt.point(mouse.x, mouse.y) );
                }
                onDoubleClickedGeo:
                {
                    //console.log("onDoubleClickedGeo: " + geocoordinate);
                    map.center = geocoordinate;
                }
            }
        }

        TrackLoader
        {
            id: trackLoader
            onTrackChanged:
            {
                var trackLength = trackLoader.trackPointCount();
                var pauseLength = trackLoader.pausePositionsCount();
                var iLastProperHeartRate = 0;

                JSTools.arrayDataPoints = [];
                JSTools.trackPointsAt = [];
                JSTools.trackPausePointsTemporary = [];

                for(var i=0; i<trackLength; i++)
                {
                    var iHeartrate = trackLoader.heartRateAt(i);

                    //Problem is there are often HR points with value 0. This will be solved.
                    if (iHeartrate > 0)
                    {
                        iLastProperHeartRate = iHeartrate;
                    }
                    else
                    {
                        iHeartrate = iLastProperHeartRate;
                    }

                    //heartrate,elevation,distance,time,unixtime,speed,pace,pacevalue,paceimp,duration
                    JSTools.fncAddDataPoint(iHeartrate, trackLoader.elevationAt(i), trackLoader.distanceAt(i), trackLoader.timeAt(i), trackLoader.unixTimeAt(i), trackLoader.speedAt(i), trackLoader.paceStrAt(i), trackLoader.paceAt(i), trackLoader.paceImperialStrAt(i), trackLoader.durationAt(i));
                    JSTools.trackPointsAt.push(trackLoader.trackPointAt(i));
                }

                //Go through array with pause data points
                for (i=0; i<pauseLength; i++)
                {
                    //add this track point to temporary array in JS.
                    JSTools.trackPausePointsTemporary.push(trackLoader.pausePositionAt(i));
                }

                //console.log("JSTools.arrayDataPoints.length: " + JSTools.arrayDataPoints.length.toString());

                //bHeartrateSupported = trackLoader.hasHeartRateData();
                //bPaceRelevantForWorkoutType = trackLoader.paceRelevantForWorkoutType();
                //iPausePositionsCount = trackLoader.pausePositionsCount();
                addActivityToMap();
            }
        }
    }

    function addActivityToMap()
    {

        var trackPointsTemporary = [];

        //Go through array with track data points
        for (var i=0; i<JSTools.trackPointsAt.length; i++)
        {
            //add this track point to temporary array. This will be used for drawing the track line
            trackPointsTemporary.push(JSTools.trackPointsAt[i]);
        }
        //This is the actialy activity route
        //vTrackLinePoints = decode(activity.map.polyline);
        map.addSourceLine("linesrc", trackPointsTemporary, "line")

        map.addLayer("line", { "type": "line", "source": "linesrc" })
        map.setLayoutProperty("line", "line-join", "round");
        map.setLayoutProperty("line", "line-cap", "round");
        map.setPaintProperty("line", "line-color", "red");
        map.setPaintProperty("line", "line-width", 2.0);

        map.fitView(trackPointsTemporary);
    }

    function decode(encoded){

        // array that holds the points

        var points=[ ]
        var index = 0, len = encoded.length;
        var lat = 0, lng = 0;
        while (index < len) {
            var b, shift = 0, result = 0;
            do {

                b = encoded.charAt(index++).charCodeAt(0) - 63;//finds ascii                                                                                    //and substract it by 63
                result |= (b & 0x1f) << shift;
                shift += 5;
            } while (b >= 0x20);


            var dlat = ((result & 1) != 0 ? ~(result >> 1) : (result >> 1));
            lat += dlat;
            shift = 0;
            result = 0;
            do {
                b = encoded.charAt(index++).charCodeAt(0) - 63;
                result |= (b & 0x1f) << shift;
                shift += 5;
            } while (b >= 0x20);
            var dlng = ((result & 1) != 0 ? ~(result >> 1) : (result >> 1));
            lng += dlng;

            points.push(QtPositioning.coordinate(( lat / 1E5), ( lng / 1E5)));
        }
        return points
    }

    function getKindString(kind) {
        if (kind == "") {
            return "unknown";
        }
        return kind.toLowerCase();
    }

    function translateSportKey(key) {
        if (key === "steps") return qsTr("Steps");
        if (key === "activeSeconds") return qsTr("Active Seconds");
        if (key === "caloriesBurnt") return qsTr("Calories");
        if (key === "distanceMeters") return qsTr("Distance");
        if (key === "ascentMeters") return qsTr("Ascent");
        if (key === "descentMeters") return qsTr("Descent");
        if (key === "maxAltitude") return qsTr("Max Altitude");
        if (key === "minAltitude") return qsTr("Min Altitude");
        if (key === "minPace") return qsTr("Min Pace");
        if (key === "maxPace") return qsTr("Max Pace");
        if (key === "averageHR") return qsTr("Average HR");
        if (key === "averageKMPaceSeconds") return qsTr("Average Pace");
        if (key === "averageStride") return qsTr("Average Stride");
        if (key === "maxHR") return qsTr("Max HR");
        if (key === "ascentSeconds") return qsTr("Time Ascending");
        if (key === "descentSeconds") return qsTr("Time Descending");
        if (key === "flatSeconds") return qsTr("Time Flat");
        if (key === "averageStrokeDistance") return qsTr("Average Stroke");
        if (key === "averageStrokesPerSecond") return qsTr("Average Stroke/s");
        if (key === "averageLapPace") return qsTr("Average Lap Pace");
        if (key === "strokes") return qsTr("Strokes");
        if (key === "swolfIndex") return qsTr("SWOLF Index");
        if (key === "swimStyle") return qsTr("Swim Style");
        if (key === "laps") return qsTr("Laps");
        if (key === "maxLatitude") return qsTr("Max Latitude");
        if (key === "minLatitude") return qsTr("Min Latitude");
        if (key === "maxLongitude") return qsTr("Max Longitude");
        if (key === "minLongitude") return qsTr("Min Longitude");
        if (key === "maxSpeed") return qsTr("Max Speed");
        if (key === "totalStride") return qsTr("Total Stride");

        return key;
    }

    function translateSportUnit(unit) {
        if (unit === "steps_unit") return qsTr("steps");
        if (unit === "seconds") return qsTr("seconds");
        if (unit === "calories_unit") return qsTr("kcal");
        if (unit === "meters") return qsTr("meters");
        if (unit === "seconds_m") return qsTr("seconds/m");
        if (unit === "bpm") return qsTr("bpm");
        if (unit === "seconds_km") return qsTr("seconds/km");
        if (unit === "cm") return qsTr("cm");
        if (unit === "strokes_second") return qsTr("strokes/s");
        if (unit === "strokes") return qsTr("strokes");
        if (unit === "swolf_index") return qsTr("swolf");
        if (unit === "style") return "";
        if (unit === "laps") return "";
        if (unit === "swolf_index") return qsTr("swolf");

        return unit;
    }

    function update() {
        loader.loadString(tcx);
    }
}
