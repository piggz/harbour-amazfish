import "../components/"
import "../components/platform"
import "../tools/JSTools.js" as JSTools
import "../components/Translation.js" as T
import "../components/ChartTools.js" as ChartTools
import MapboxMap 1.0
import QtPositioning 5.3
import QtQuick 2.0
import QtQuick.Layouts 1.1
import uk.co.piggz.amazfish 1.0

PagePL {
    id: page

    property int activityId
    property string date: ""
    property string duration: ""
    property var location: ""
    property string starttime: ""
    property string times: ""
    property string kindstring: ""
    property string activitytitle: ""
    property string tcx: ""
    property string rawGpx: ""
    property bool bMapMaximized: false
    property alias loader: trackLoader

    // One of: loading, ok, nogps (no positions), nobase (no start position), nofile, invalid.
    property string trackStatus: "loading"
    property int gpsPointCount: 0

    // values taken from the track once it has been parsed
    property real trackDistance: 0
    property string trackPace: ""
    property real trackHeartrate: 0
    property bool paceRelevant: true
    property var hrPoints: []
    property var pacePoints: []
    property var rawPacePoints: []

    // Distance measured by the watch, and the latitude estimated from it for tracks without start position.
    property real watchDistance: 0
    property real estimatedLatitude: -1
    property var elevationPoints: []

    title: activitytitle

    function addActivityToMap() {
        var map = mapLoader.item;
        if (!map) {
            return;
        }
        var trackPointsTemporary = [];
        for (var i = 0; i < JSTools.trackPointsAt.length; i++) {
            trackPointsTemporary.push(JSTools.trackPointsAt[i]);
        }
        map.addSourceLine("linesrc", trackPointsTemporary, "line");
        map.addLayer("line", {
            "type": "line",
            "source": "linesrc"
        });
        map.setLayoutProperty("line", "line-join", "round");
        map.setLayoutProperty("line", "line-cap", "round");
        map.setPaintProperty("line", "line-color", "" + styler.chartRouteColor);
        map.setPaintProperty("line", "line-width", 4);

        // start and finish markers
        map.addSourcePoint("startsrc", trackPointsTemporary[0], "start");
        map.addLayer("start", { "type": "circle", "source": "startsrc" });
        map.setPaintProperty("start", "circle-radius", 6);
        map.setPaintProperty("start", "circle-color", "" + styler.chartRouteStartColor);
        map.setPaintProperty("start", "circle-stroke-width", 2);
        map.setPaintProperty("start", "circle-stroke-color", "" + styler.themePrimaryColor);

        map.addSourcePoint("endsrc", trackPointsTemporary[trackPointsTemporary.length - 1], "end");
        map.addLayer("end", { "type": "circle", "source": "endsrc" });
        map.setPaintProperty("end", "circle-radius", 6);
        map.setPaintProperty("end", "circle-color", "" + styler.chartRouteFinishColor);
        map.setPaintProperty("end", "circle-stroke-width", 2);
        map.setPaintProperty("end", "circle-stroke-color", "" + styler.themePrimaryColor);

        fitMap();
    }

    function fitMap() {
        var map = mapLoader.item;
        if (!map || map.width <= 0 || map.height <= 0 || JSTools.trackPointsAt.length === 0) {
            return;
        }
        var pts = [];
        for (var i = 0; i < JSTools.trackPointsAt.length; i++) {
            pts.push(JSTools.trackPointsAt[i]);
        }
        map.fitView(pts);
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
        if (Math.abs(lat) < 1e-9 && Math.abs(lon) < 1e-9)
            positionstring = "---";
        else if (alt < -1000)
            positionstring = qsTr("%1°; %2°").arg(lat.toFixed(3).toLocaleString()).arg(lon.toFixed(3).toLocaleString());
        else
            positionstring = qsTr("%1°; %2°; %3m").arg(lat.toFixed(3).toLocaleString()).arg(lon.toFixed(3).toLocaleString()).arg(alt.toLocaleString());

        console.log("location: " + lat + ", " + lon + ", " + alt + " ---> " + positionstring);

        return positionstring;
    }

    function hasBaseLocation() {
        return location && (Math.abs(location[0]) > 1e-9 || Math.abs(location[1]) > 1e-9);
    }

    // GPS points are deltas to the start position of the summary.
    // Some watches (Amazfit GTS) send 0/0 there, so the route lies around 0N 0E.
    function trackIsRelativeOnly() {
        if (hasBaseLocation() || JSTools.trackPointsAt.length === 0) {
            return false;
        }
        for (var i = 0; i < JSTools.trackPointsAt.length; i++) {
            var c = JSTools.trackPointsAt[i];
            if (Math.abs(c.latitude) > 1 || Math.abs(c.longitude) > 1) {
                return false;
            }
        }
        return true;
    }

    // length of the track in metres when it is placed at latitude latDeg
    function pathLength(latDeg) {
        var pts = JSTools.trackPointsAt;
        var k = Math.cos(latDeg * Math.PI / 180);
        var sum = 0;
        for (var i = 1; i < pts.length; i++) {
            var dy = (pts[i].latitude - pts[i - 1].latitude) * 111320;
            var dx = (pts[i].longitude - pts[i - 1].longitude) * 111320 * k;
            sum += Math.sqrt(dx * dx + dy * dy);
        }
        return sum;
    }

    // East-west steps shrink with the cosine of the latitude, so the distance of the watch gives the latitude.
    function estimateLatitude() {
        if (watchDistance <= 0 || JSTools.trackPointsAt.length < 2) {
            return -1;
        }
        var lo = 0, hi = 85;
        if (pathLength(lo) <= watchDistance || pathLength(hi) >= watchDistance) {
            return -1;   // no east-west movement to measure, or distance does not fit
        }
        for (var n = 0; n < 40; n++) {
            var mid = (lo + hi) / 2;
            if (pathLength(mid) > watchDistance) lo = mid; else hi = mid;
        }
        return (lo + hi) / 2;
    }

    // Recompute distance, pace and shape with the latitude estimated from the watch distance.
    function correctRelativeTrack() {
        if (trackStatus !== "nobase" || watchDistance <= 0) {
            return;
        }
        var lat = estimateLatitude();
        if (lat < 0) {
            return;
        }
        estimatedLatitude = lat;
        routeShape.referenceLatitude = lat;

        var factor = pathLength(0) / pathLength(lat);   // > 1: how much too long it was
        trackDistance = pathLength(lat);
        var fixed = [];
        for (var i = 0; i < rawPacePoints.length; i++) {
            var p = rawPacePoints[i];
            fixed.push({ x: p.x, y: paceRelevant ? p.y * factor : p.y / factor });
        }
        pacePoints = fixed;
        if (trackLoader.duration > 0 && trackDistance > 0) {
            trackPace = formatPace((trackLoader.duration / 60) / (trackDistance / 1000));
        }
        console.log("SportPage: estimated latitude", lat.toFixed(2), "distance", Math.round(trackDistance), "m");
    }

    // Summary values arrive raw (seconds, s/m, m/s, long floats); format them for reading.
    function formatNumber(v) {
        var a = Math.abs(v);
        var decimals = a >= 100 ? 0 : (a >= 10 ? 1 : 2);
        var text = Number(v).toLocaleString(Qt.locale(), "f", decimals);
        if (decimals > 0) {
            // drop trailing zeros after the decimal separator ("5,50" -> "5,5", "7,00" -> "7")
            var sep = Qt.locale().decimalPoint;
            text = text.replace(new RegExp("\\" + sep + "?0+$"), "");
        }
        return text;
    }

    function formatSeconds(sec) {
        sec = Math.round(sec);
        var h = Math.floor(sec / 3600), m = Math.floor((sec % 3600) / 60), s = sec % 60;
        var mm = (h > 0 && m < 10 ? "0" : "") + m;
        return (h > 0 ? h + ":" + mm : mm) + ":" + (s < 10 ? "0" : "") + s;
    }

    // units like "min/km" must not break after the slash (U+2060 WORD JOINER)
    function keepUnit(text) {
        return text.replace(/\//g, "/\u2060");
    }

    function metaValue(value, unit) {
        return keepUnit(rawMetaValue(value, unit));
    }

    function rawMetaValue(value, unit) {
        var v = parseFloat(value);
        if (isNaN(v) || !/^-?[0-9.eE+-]+$/.test(String(value).trim())) {
            return value + (unit ? " " + T.translateSportUnit(unit) : "");   // text, e.g. swim style
        }
        if (unit === "seconds") {
            return formatSeconds(v);
        }
        if (unit === "seconds_km" && v > 0) {
            return formatSeconds(v) + " " + qsTr("min/km");
        }
        if (unit === "seconds_m" && v > 0) {
            return formatSeconds(v * 1000) + " " + qsTr("min/km");
        }
        if (unit === "meters_second") {
            return formatNumber(v * 3.6) + " " + qsTr("km/h");
        }
        return formatNumber(v) + (unit ? " " + T.translateSportUnit(unit) : "");
    }

    function formatPace(minPerKm) {
        var total = Math.round(minPerKm * 60);
        var m = Math.floor(total / 60);
        var s = total % 60;
        return m + ":" + (s < 10 ? "0" : "") + s;
    }

    function update() {
        // SportsModel.gpx() returns the stored file name itself when the file cannot be opened
        var content = tcx ? tcx.replace(/^\s+/, "") : "";
        if (content.charAt(0) !== "<") {
            console.log("SportPage: track file could not be read:", tcx);
            trackStatus = "nofile";
            return;
        }
        trackStatus = "loading";
        loader.loadString(tcx);
        if (trackStatus === "loading") {
            // TrackLoader returns without emitting trackChanged when the XML is not understood
            console.log("SportPage: track file is neither GPX 1.1 nor TCX v2");
            trackStatus = "invalid";
        }
    }

    // reduce a series to at most maxPoints by averaging neighbouring samples
    function downsample(list, maxPoints) {
        if (list.length <= maxPoints) {
            return list;
        }
        var out = [];
        var step = list.length / maxPoints;
        for (var b = 0; b < maxPoints; b++) {
            var from = Math.floor(b * step), to = Math.floor((b + 1) * step);
            var sx = 0, sy = 0, n = 0;
            for (var i = from; i < to; i++) {
                sx += list[i].x; sy += list[i].y; n++;
            }
            if (n) {
                out.push({ x: sx / n, y: sy / n });
            }
        }
        return out;
    }

    Column {
        id: column
        x: styler.themeHorizontalPageMargin
        width: parent.width - 2 * x
        spacing: styler.themePaddingLarge

        // ---------- header ----------
        ChartCard {
            onClicked: {
                var activityPage = app.pages.push(Qt.resolvedUrl("SportsActivityKindPage.qml"), {
                    "kindstring": kindstring
                })
                activityPage.onAccepted.connect(function() {
                    console.log("Accepted " + activityPage.kindstring + " " + activityId)
                    kindstring = activityPage.kindstring;
                    SportsModel.setKind(activityId, activityPage.kindstring );
                    SportsModel.update();
                })
            }

            Row {
                width: parent.width
                spacing: styler.themePaddingLarge

                Rectangle {
                    id: kindCircle
                    width: styler.themeItemSizeLarge * 0.6
                    height: width
                    radius: width / 2
                    color: styler.surfaceChipColor

                    IconPL {
                        anchors.centerIn: parent
                        width: parent.width * 0.62
                        height: width
                        iconSource: styler.activityIconPrefix + "icon-m-" + getKindString(kindstring)
                                    + styler.customIconSuffix
                    }
                }

                Column {
                    anchors.verticalCenter: kindCircle.verticalCenter
                    width: parent.width - kindCircle.width - parent.spacing

                    LabelPL {
                        width: parent.width
                        text: T.translateSportKind(kindstring)
                        color: styler.themeHighlightColor
                        font.pixelSize: styler.themeFontSizeLarge
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                    LabelPL {
                        width: parent.width
                        text: times !== "" ? date + " - " + times : date + " - " + starttime
                        color: styler.themeSecondaryHighlightColor
                        font.pixelSize: styler.themeFontSizeSmall
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                }
            }

            // key figures
            Grid {
                id: statGrid
                width: parent.width
                columns: 2
                columnSpacing: styler.themePaddingLarge
                rowSpacing: styler.themePaddingMedium
                readonly property real cellWidth: (width - columnSpacing) / 2
                // all four values shrink to the size the widest one needs
                readonly property real sharedScale: Math.min(statDuration.fitScale, statDistance.fitScale,
                                                             statPace.visible ? statPace.fitScale : 1,
                                                             statHeartrate.fitScale)

                StatTile {
                    id: statDuration
                    sharedScale: statGrid.sharedScale
                    width: statGrid.cellWidth
                    value: duration
                    label: qsTr("Duration")
                }
                StatTile {
                    id: statDistance
                    sharedScale: statGrid.sharedScale
                    width: statGrid.cellWidth
                    value: trackDistance > 0 ? (trackDistance / 1000).toLocaleString(Qt.locale(), "f", 2) : "-"
                    unit: trackDistance > 0 ? "km" : ""
                    label: qsTr("Distance")
                }
                StatTile {
                    id: statPace
                    sharedScale: statGrid.sharedScale
                    width: statGrid.cellWidth
                    visible: paceRelevant
                    value: trackPace !== "" ? trackPace : "-"
                    unit: trackPace !== "" ? "/km" : ""
                    label: qsTr("Average Pace")
                }
                StatTile {
                    id: statHeartrate
                    sharedScale: statGrid.sharedScale
                    width: statGrid.cellWidth
                    value: trackHeartrate > 0 ? Math.round(trackHeartrate) : "-"
                    unit: trackHeartrate > 0 ? qsTr("BPM") : ""
                    label: qsTr("Average Heart Rate")
                }
            }
        }

        // ---------- map ----------
        ChartCard {
            id: mapCard
            title: qsTr("Route")

            Item {
                width: parent.width
                height: bMapMaximized ? page.height * 0.8 : width * 0.75
                visible: trackStatus === "ok"
                clip: true

                Behavior on height { NumberAnimation { duration: 150 } }

                Loader {
                    id: mapLoader
                    anchors.fill: parent
                    active: trackStatus === "ok"
                    sourceComponent: mapComponent
                    onLoaded: addActivityToMap()
                }

                Item {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.margins: styler.themePaddingMedium
                    width: styler.themeIconSizeMedium
                    height: width
                    z: 200

                    IconPL {
                        anchors.fill: parent
                        source: "../pics/map_btn_center.png"
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: fitMap()
                    }
                }

                Item {
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.margins: styler.themePaddingMedium
                    width: styler.themeIconSizeMedium
                    height: width
                    z: 200

                    IconPL {
                        anchors.fill: parent
                        source: bMapMaximized ? "../pics/map_btn_min.png" : "../pics/map_btn_max.png"
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: bMapMaximized = !bMapMaximized
                    }
                }

            }

            // shape of the route without a map when its location is unknown
            RouteShape {
                id: routeShape
                visible: trackStatus === "nobase"
            }

            // explains why there is no map instead of leaving an empty space
            LabelPL {
                width: parent.width
                visible: trackStatus !== "ok"
                wrapMode: Text.WordWrap
                color: styler.themeSecondaryColor
                font.pixelSize: styler.themeFontSizeSmall
                text: {
                    if (trackStatus === "loading") return qsTr("Loading track...");
                    if (trackStatus === "nobase") {
                        var msg = qsTr("The watch sent no start position.") + " "
                                  + qsTr("Only the shape of the route is known, not its place on the map.");
                        if (estimatedLatitude >= 0)
                            msg += " " + qsTr("Distance and pace were corrected with the watch distance.");
                        return msg;
                    }
                    if (trackStatus === "nogps")
                        return qsTr("This activity has no GPS positions.") + " "
                               + qsTr("The watch recorded it without GPS, so there is no route.");
                    if (trackStatus === "nofile")
                        return qsTr("The track file could not be read: %1").arg(tcx);
                    return qsTr("The track file of this activity has an unsupported format.");
                }
            }
        }

        // ---------- charts ----------
        ChartCard {
            title: qsTr("Heart Rate")
            info: trackHeartrate > 0 ? qsTr("Avg. %1 BPM").arg(Math.round(trackHeartrate)) : ""
            visible: hrPoints.length > 1

            TrackChart {
                points: hrPoints
                color: styler.chartHeartRateColor
                referenceValue: trackHeartrate
            }
        }

        ChartCard {
            title: paceRelevant ? qsTr("Pace") : qsTr("Speed")
            info: paceRelevant ? qsTr("min/km") : qsTr("km/h")
            visible: pacePoints.length > 1

            TrackChart {
                points: pacePoints
                color: styler.chartPaceColor
                invertY: paceRelevant
                valueLabel: function(v) { return paceRelevant ? formatPace(v) : v.toFixed(0); }
            }
        }

        ChartCard {
            title: qsTr("Elevation")
            info: "m"
            visible: elevationPoints.length > 1

            TrackChart {
                points: elevationPoints
                color: styler.chartElevationColor
            }
        }

        // ---------- all values reported by the watch ----------
        ChartCard {
            title: qsTr("Details")
            visible: metaRepeater.count > 0

            Repeater {
                id: metaRepeater
                model: SportsMeta
                delegate: DetailRow {
                    label: T.translateSportKey(model.key)
                    value: page.metaValue(model.value, model.unit)
                    Component.onCompleted: {
                        if (model.key === "distanceMeters") {
                            page.watchDistance = parseFloat(model.value) || 0;
                            page.correctRelativeTrack();
                        }
                    }
                }
            }

            DetailRow {
                label: qsTr("Location")
                value: positionString(location[0], location[1], location[2])
                visible: hasBaseLocation()
            }
        }
    }

    Component {
        id: mapComponent

        MapboxMap {
            id: map

            center: QtPositioning.coordinate(51.9854, 9.2743)
            zoomLevel: 8
            minimumZoomLevel: 0
            maximumZoomLevel: 20
            pixelRatio: 3
            accessToken: "pk.eyJ1IjoiamRyZXNjaGVyIiwiYSI6ImNqYmVta256YTJsdjUzMm1yOXU0cmxibGoifQ.JiMiONJkWdr0mVIjajIFZQ"
            cacheDatabaseDefaultPath: true
            styleUrl: "mapbox://styles/mapbox/outdoors-v11"
            // keep the route and its start/finish markers away from the edges and the map buttons
            margins: Qt.rect(0.12, 0.08, 0.76, 0.76)

            // the first fitView can run before the map has its final size
            onWidthChanged: fitTimer.restart()
            onHeightChanged: fitTimer.restart()

            Timer {
                id: fitTimer
                interval: 200
                onTriggered: page.fitMap()
            }

            MapboxMapGestureArea {
                id: mouseArea

                map: map
                activeClickedGeo: true
                activeDoubleClickedGeo: true
                activePressAndHoldGeo: false
                onDoubleClicked: {
                    map.setZoomLevel(map.zoomLevel + 1, Qt.point(mouse.x, mouse.y));
                }
                onDoubleClickedGeo: {
                    map.center = geocoordinate;
                }
            }
        }
    }

    TrackLoader {
        id: trackLoader

        onTrackChanged: {
            var trackLength = trackLoader.trackPointCount();
            var pauseLength = trackLoader.pausePositionsCount();
            var iLastProperHeartRate = 0;
            var hr = [], pace = [], ele = [];
            var t0 = trackLength > 0 ? trackLoader.unixTimeAt(0) : 0;
            paceRelevant = trackLoader.paceRelevantForWorkoutType();

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
                JSTools.fncAddDataPoint(iHeartrate, trackLoader.elevationAt(i), trackLoader.distanceAt(i),
                                        trackLoader.timeAt(i), trackLoader.unixTimeAt(i), trackLoader.speedAt(i),
                                        trackLoader.paceStrAt(i), trackLoader.paceAt(i),
                                        trackLoader.paceImperialStrAt(i), trackLoader.durationAt(i));
                JSTools.trackPointsAt.push(trackLoader.trackPointAt(i));

                var x = trackLoader.unixTimeAt(i) - t0;
                if (iHeartrate > 0) {
                    hr.push({ x: x, y: iHeartrate });
                }
                var p = trackLoader.paceAt(i);   // min/km
                if (p > 0 && p < 30) {
                    pace.push({ x: x, y: paceRelevant ? p : 60 / p });
                }
                ele.push({ x: x, y: trackLoader.elevationAt(i) });
            }
            //Go through array with pause data points
            for (var j = 0; j < pauseLength; j++) {
                JSTools.trackPausePointsTemporary.push(trackLoader.pausePositionAt(j));
            }

            hrPoints = downsample(hr, 300);
            rawPacePoints = downsample(pace, 300);
            pacePoints = rawPacePoints;
            elevationPoints = downsample(ele, 300);

            trackDistance = trackLoader.distance;
            trackPace = trackLoader.paceStr;
            trackHeartrate = trackLoader.hasHeartRateData() ? trackLoader.heartRate : 0;

            gpsPointCount = trackLength;
            console.log("SportPage: track loaded with", trackLength, "GPS points");
            if (trackLength === 0) {
                trackStatus = "nogps";
            } else if (trackIsRelativeOnly()) {
                console.log("SportPage: no start position from the watch, route location unknown");
                trackStatus = "nobase";
            } else {
                trackStatus = "ok";
            }
            if (trackStatus === "nobase") {
                correctRelativeTrack();
                var shapePts = [];
                for (var k = 0; k < JSTools.trackPointsAt.length; k++) {
                    shapePts.push(JSTools.trackPointsAt[k]);
                }
                routeShape.coordinates = shapePts;
            }
            if (mapLoader.item) {
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
        PageMenuItemPL {
            iconSource: styler.iconUploadToStrava !== undefined ? styler.iconUploadToStrava : ""
            text: qsTr("Send to FitTrackee")
            visible: o2fittrackee.linked
            onClicked: {
                var dialog = app.pages.push(Qt.resolvedUrl("FitTrackeeUploadPage.qml"));
                dialog.activityID = activitytitle.replace(/\s/g, '');
                dialog.tcx = tcx;
                dialog.activityName = activitytitle;
                dialog.activityDescription = trackLoader.description;
                dialog.activityType = kindstring;
            }
        }
        PageMenuItemPL {
            iconSource: styler.iconUploadToStrava !== undefined ? styler.iconUploadToStrava : ""
            text: qsTr("Send to FitPub")
            visible: app.fitpubLinked
            onClicked: {
                var dialog = app.pages.push(Qt.resolvedUrl("FitPubUploadPage.qml"));
                dialog.activityID = activitytitle.replace(/\s/g, '');
                dialog.tcx = rawGpx !== "" ? rawGpx : tcx;
                dialog.activityName = activitytitle;
                dialog.activityDescription = trackLoader.description;
                dialog.activityType = kindstring;
            }
        }

    }

}
