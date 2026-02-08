import "../components/"
import "../components/platform"
import "../tools/JSTools.js" as JSTools
import MapboxMap 1.0
import QtPositioning 5.3
import QtQuick 2.0
import QtQuick.Layouts 1.1
import uk.co.piggz.amazfish 1.0

PagePL {
    // array that holds the points

    id: page

    property string date: ""
    property string duration: ""
    property var location: ""
    property string starttime: ""
    property string kindstring: ""
    property string activitytitle: ""
    property string tcx: ""
    property bool bMapMaximized: false
    property alias loader: trackLoader

    function addActivityToMap() {
        var trackPointsTemporary = [];
        //Go through array with track data points
        for (var i = 0; i < JSTools.trackPointsAt.length; i++) {
            //add this track point to temporary array. This will be used for drawing the track line
            trackPointsTemporary.push(JSTools.trackPointsAt[i]);
        }
        //This is the actialy activity route
        //vTrackLinePoints = decode(activity.map.polyline);
        map.addSourceLine("linesrc", trackPointsTemporary, "line");
        map.addLayer("line", {
            "type": "line",
            "source": "linesrc"
        });
        map.setLayoutProperty("line", "line-join", "round");
        map.setLayoutProperty("line", "line-cap", "round");
        map.setPaintProperty("line", "line-color", "red");
        map.setPaintProperty("line", "line-width", 2);
        map.fitView(trackPointsTemporary);
    }

    function decode(encoded) {
        var points = [];
        var index = 0, len = encoded.length;
        var lat = 0, lng = 0;
        while (index < len) {
            var b, shift = 0, result = 0;
            do {
                b = encoded.charAt(index++).charCodeAt(0) - 63; //finds ascii                                                                                    //and substract it by 63
                result |= (b & 31) << shift;
                shift += 5;
            } while (b >= 32)
            var dlat = ((result & 1) != 0 ? ~(result >> 1) : (result >> 1));
            lat += dlat;
            shift = 0;
            result = 0;
            do {
                b = encoded.charAt(index++).charCodeAt(0) - 63;
                result |= (b & 31) << shift;
                shift += 5;
            } while (b >= 32)
            var dlng = ((result & 1) != 0 ? ~(result >> 1) : (result >> 1));
            lng += dlng;
            points.push(QtPositioning.coordinate((lat / 100000), (lng / 100000)));
        }
        return points;
    }

    function getKindString(kind) {
        if (kind == "")
            return "unknown";

        return kind.toLowerCase();
    }

    function positionString(lat, lon, alt) {
        var positionstring;
        console.log("location: " + lat + lon + alt);
        if (lat === 0 && lon === 0)
            positionstring = "---";
        else if (alt < -1000)
            positionstring = qsTr("%1°; %2°").arg(lat.toFixed(3).toLocaleString()).arg(lon.toFixed(3).toLocaleString());
        else
            positionstring = qsTr("%1°; %2°; %3m").arg(lat.toFixed(3).toLocaleString()).arg(lon.toFixed(3).toLocaleString()).arg(alt.toLocaleString());
        return positionstring;
    }

    function translateSportKey(key) {
        const translations = {
            "activeSeconds": qsTr("Active Time"),
            "aerobicTrainingEffect": qsTr("Aerobic Training Effect"),
            "anaerobicTrainingEffect": qsTr("Anaerobic Training Effect"),
            "ascentMeters": qsTr("Ascent"),
            "ascentSeconds": qsTr("Time Ascending"),
            "averageHeartRate": qsTr("Average Heart Rate"),
            "averageHR": qsTr("Average Heart Rate"),
            "averageKMPaceSeconds": qsTr("Average Pace"),
            "averageLapPace": qsTr("Average Lap Pace"),
            "averageStride": qsTr("Average Stride"),
            "averageStrokeDistance": qsTr("Average Stroke Distance"),
            "averageStrokesPerSecond": qsTr("Average Stroke Rate"),
            "avgAltitude": qsTr("Average Altitude"),
            "avgHeartRate": qsTr("Average Heart Rate"),
            "caloriesBurnt": qsTr("Calories"),
            "calories": qsTr("Calories"),
            "currentWorkoutLoad": qsTr("Current Workout Load"),
            "descentMeters": qsTr("Descent"),
            "descentSeconds": qsTr("Time Descending"),
            "distanceMeters": qsTr("Distance"),
            "downhillTime": qsTr("Downhill Time"),
            "elevationGain": qsTr("Elevation Gain"),
            "elevationLoss": qsTr("Elevation Loss"),
            "flatSeconds": qsTr("Time on Flat"),
            "laps": qsTr("Laps"),
            "maxAltitude": qsTr("Max Altitude"),
            "maxHeartRate": qsTr("Max Heart Rate"),
            "maxHR": qsTr("Max Heart Rate"),
            "maximumOxygenUptake": qsTr("VO₂ Max"),
            "maxLatitude": qsTr("Max Latitude"),
            "maxLongitude": qsTr("Max Longitude"),
            "maxPace": qsTr("Max Pace"),
            "maxSpeed": qsTr("Max Speed"),
            "minAltitude": qsTr("Min Altitude"),
            "minHeartRate": qsTr("Min Heart Rate"),
            "minLatitude": qsTr("Min Latitude"),
            "minLongitude": qsTr("Min Longitude"),
            "minPace": qsTr("Min Pace"),
            "paceAvg": qsTr("Average Pace"),
            "paceBest": qsTr("Best Pace"),
            "pauseDuration": qsTr("Pause Duration"),
            "steps": qsTr("Steps"),
            "stepsAvgCadence": qsTr("Average Cadence"),
            "stepsAvgStride": qsTr("Average Stride"),
            "stepsMaxCadence": qsTr("Max Cadence"),
            "strokes": qsTr("Strokes"),
            "swimAvgDps": qsTr("Average Distance per Stroke"),
            "swimAvgMaxRate": qsTr("Max Stroke Rate"),
            "swimAvgStrokeRate": qsTr("Average Stroke Rate"),
            "swimLaneLength": qsTr("Lane Length"),
            "swimLaps": qsTr("Swim Laps"),
            "swimStrokes": qsTr("Swim Strokes"),
            "swimStyle": qsTr("Swim Style"),
            "swolfIndex": qsTr("SWOLF Index"),
            "totalClimbing": qsTr("Total Climbing"),
            "totalDuration": qsTr("Total Duration"),
            "totalStride": qsTr("Total Stride"),
            "uphillTime": qsTr("Uphill Time"),
            "workoutDuration": qsTr("Workout Duration")
        };
        if (translations[key] !== undefined)
            return translations[key];

        return key;
    }

    function translateSportUnit(unit) {
        const translations = {
            "bpm": qsTr("bpm"),
            "calories": qsTr("kcal"),
            "calories_unit": qsTr("kcal"),
            "cm": qsTr("cm"),
            "deg": qsTr("°"),
            "laps": qsTr("laps"),
            "meters": qsTr("m"),
            "meters_second": qsTr("m/s"),
            "min_km": qsTr("min/km"),
            "ml_kg_min": qsTr("ml/kg/min"),
            "seconds": qsTr("s"),
            "seconds_km": qsTr("s/km"),
            "seconds_m": qsTr("s/m"),
            "steps": qsTr("steps"),
            "steps_min": qsTr("steps/min"),
            "steps_unit": qsTr("steps"),
            "strokes": qsTr("strokes"),
            "strokes_second": qsTr("strokes/s"),
            "swolf_index": qsTr("SWOLF"),
        };
        if (translations[unit] !== undefined)
            return translations[unit];

        return unit;
    }

    function update() {
        loader.loadString(tcx);
    }

    title: activitytitle

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

            IconPL {
                id: workoutImage

                Layout.rowSpan: 3
                Layout.preferredWidth: styler.themeItemSizeLarge
                Layout.preferredHeight: styler.themeItemSizeLarge
                Layout.alignment: Qt.AlignLeft
                iconSource: styler.activityIconPrefix + "icon-m-" + getKindString(kindstring) + styler.customIconSuffix
            }

            LabelPL {
                id: dateLabel

                text: qsTr("Start: %1 %2").arg(date).arg(starttime)
            }

            LabelPL {
                id: durationLabel

                text: qsTr("Duration: %1").arg(duration)
            }

            LabelPL {
                id: locationLabel

                text: qsTr("Location: %1").arg(positionString(location[0], location[1], location[2]))
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

                LabelPL {
                    id: keyLabel

                    width: (page.width - (2 * styler.themePaddingLarge)) / 2
                    anchors.topMargin: styler.themePaddingMedium
                    anchors.left: parent.left
                    anchors.leftMargin: styler.themePaddingMedium
                    text: translateSportKey(key)
                }

                LabelPL {
                    id: valueLabel

                    width: (page.width - (2 * styler.themePaddingLarge)) / 2
                    anchors.top: parent.top
                    anchors.left: keyLabel.right
                    anchors.leftMargin: styler.themePaddingMedium
                    text: value + " " + translateSportUnit(unit)
                }

            }

        }

        MapboxMap {
            //height: bMapMaximized ? page.height : (page.height - (listView.y + listView.height))

            id: map

            width: pageItem.width
            anchors.top: bMapMaximized ? pageItem.top : listView.bottom
            anchors.left: pageItem.left
            anchors.right: pageItem.right
            anchors.bottom: pageItem.bottom
            center: QtPositioning.coordinate(51.9854, 9.2743)
            zoomLevel: 8
            minimumZoomLevel: 0
            maximumZoomLevel: 20
            pixelRatio: 3
            accessToken: "pk.eyJ1IjoiamRyZXNjaGVyIiwiYSI6ImNqYmVta256YTJsdjUzMm1yOXU0cmxibGoifQ.JiMiONJkWdr0mVIjajIFZQ"
            cacheDatabaseDefaultPath: true
            styleUrl: "mapbox://styles/mapbox/outdoors-v11"
            visible: true

            Item {
                id: centerButton

                anchors.left: parent.left
                anchors.leftMargin: styler.themePaddingSmall
                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingSmall
                width: parent.width / 10
                height: parent.width / 10
                visible: true
                z: 200

                MouseArea {
                    anchors.fill: parent
                    onReleased: {
                        console.log("centerButton pressed");
                        var trackPointsTemporary = [];
                        //Go through array with track data points
                        for (var i = 0; i < JSTools.trackPointsAt.length; i++) {
                            //add this track point to temporary array. This will be used for drawing the track line
                            trackPointsTemporary.push(JSTools.trackPointsAt[i]);
                        }
                        map.fitView(trackPointsTemporary);
                    }
                }

                IconPL {
                    anchors.fill: parent
                    source: "../pics/map_btn_center.png"
                }

            }

            Item {
                id: minmaxButton

                anchors.right: parent.right
                anchors.rightMargin: styler.themePaddingSmall
                anchors.top: parent.top
                anchors.topMargin: styler.themePaddingSmall
                width: parent.width / 10
                height: parent.width / 10
                visible: true
                z: 200

                MouseArea {
                    anchors.fill: parent
                    onReleased: {
                        console.log("minmaxButton pressed");
                        bMapMaximized = !bMapMaximized;
                    }
                }

                IconPL {
                    anchors.fill: parent
                    source: (map.height === page.height) ? "../pics/map_btn_min.png" : "../pics/map_btn_max.png"
                }

            }

            MapboxMapGestureArea {
                id: mouseArea

                map: map
                activeClickedGeo: true
                activeDoubleClickedGeo: true
                activePressAndHoldGeo: false
                onDoubleClicked: {
                    //console.log("onDoubleClicked: " + mouse)
                    map.setZoomLevel(map.zoomLevel + 1, Qt.point(mouse.x, mouse.y));
                }
                onDoubleClickedGeo: {
                    //console.log("onDoubleClickedGeo: " + geocoordinate);
                    map.center = geocoordinate;
                }
            }

            Behavior on height {
                NumberAnimation {
                    duration: 150
                }

            }

        }

        TrackLoader {
            id: trackLoader

            onTrackChanged: {
                //console.log("JSTools.arrayDataPoints.length: " + JSTools.arrayDataPoints.length.toString());

                var trackLength = trackLoader.trackPointCount();
                var pauseLength = trackLoader.pausePositionsCount();
                var iLastProperHeartRate = 0;
                JSTools.arrayDataPoints = [];
                JSTools.trackPointsAt = [];
                JSTools.trackPausePointsTemporary = [];
                for (var i = 0; i < trackLength; i++) {
                    var iHeartrate = trackLoader.heartRateAt(i);
                    //Problem is there are often HR points with value 0. This will be solved.
                    if (iHeartrate > 0)
                        iLastProperHeartRate = iHeartrate;
                    else
                        iHeartrate = iLastProperHeartRate;
                    //heartrate,elevation,distance,time,unixtime,speed,pace,pacevalue,paceimp,duration
                    JSTools.fncAddDataPoint(iHeartrate, trackLoader.elevationAt(i), trackLoader.distanceAt(i), trackLoader.timeAt(i), trackLoader.unixTimeAt(i), trackLoader.speedAt(i), trackLoader.paceStrAt(i), trackLoader.paceAt(i), trackLoader.paceImperialStrAt(i), trackLoader.durationAt(i));
                    JSTools.trackPointsAt.push(trackLoader.trackPointAt(i));
                }
                //Go through array with pause data points
                for (; i < pauseLength; i++) {
                    //add this track point to temporary array in JS.
                    JSTools.trackPausePointsTemporary.push(trackLoader.pausePositionAt(i));
                }
                //bHeartrateSupported = trackLoader.hasHeartRateData();
                //bPaceRelevantForWorkoutType = trackLoader.paceRelevantForWorkoutType();
                //iPausePositionsCount = trackLoader.pausePositionsCount();
                addActivityToMap();
            }
        }

    }

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconUploadToStrava !== undefined ? styler.iconUploadToStrava : ""
            text: qsTr("Send to Strava")
            visible: o2strava.linked
            onClicked: {
                var dialog = app.pages.push(Qt.resolvedUrl("StravaUploadPage.qml"));
                dialog.activityID = activitytitle.replace(/\s/g, '');
                dialog.tcx = tcx;
                dialog.activityName = activitytitle;
                dialog.activityDescription = trackLoader.description;
                dialog.activityType = kindstring;
            }
        }

    }

}
