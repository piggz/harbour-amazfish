import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components/"
import QtPositioning 5.3
import MapboxMap 1.0
import uk.co.piggz.amazfish 1.0
import "../tools/JSTools.js" as JSTools

Page {
    id: page

    property string title: ""
    property string date: ""
    property string duration: ""
    property string location: ""
    property string startdate: ""
    property string kindstring: ""

    property bool bMapMaximized: false
    property alias loader: trackLoader

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    PageHeader {
        id: pageHeader
        title: page.title
    }

    Image
    {
        id: workoutImage
        anchors.top: pageHeader.bottom
        anchors.margins: Theme.paddingMedium
        anchors.left: parent.left
        width: Theme.itemSizeHuge
        height: width
        source: "../pics/icon-m-" + getKindString(kindstring) + ".png"
    }

    Label
    {
        id: dateLabel
        anchors.top: pageHeader.bottom
        anchors.right: parent.right
        anchors.margins: Theme.paddingMedium
        text: startdate
    }

    Label
    {
        id: durationLabel
        anchors.top: dateLabel.bottom
        anchors.right: parent.right
        anchors.margins: Theme.paddingMedium
        text: duration
    }

    Label
    {
        id: locationLabel
        anchors.top: durationLabel.bottom
        anchors.right: parent.right
        anchors.margins: Theme.paddingMedium
        text: location
    }

    Label {
        id: dataLabel
        anchors.top: workoutImage.bottom
        anchors.left: parent.left
        anchors.margins: Theme.paddingMedium
        text: qsTr("Data:")
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaListView {
        id: listView
        anchors.top: dataLabel.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: Theme.paddingMedium
        height: page.height / 3
        clip: true

        model: SportsMeta

        VerticalScrollDecorator {}

        delegate: ListItem {
            id: listItem
            contentHeight: keyLabel.height + Theme.paddingMedium

            Label
            {
                id: keyLabel
                width: page.width / 2
                anchors.topMargin: Theme.paddingMedium
                anchors.left: parent.left
                anchors.leftMargin: Theme.paddingMedium
                truncationMode: TruncationMode.Fade
                text: key
            }

            Label
            {
                id: valueLabel
                width: page.width / 3
                anchors.top: parent.top
                anchors.left: keyLabel.right
                anchors.leftMargin: Theme.paddingMedium
                text: value + " " + unit
            }

        }
    }

    MapboxMap
    {
        id: map

        width: parent.width
        //anchors.top: bMapMaximized ? page.top : listView.bottom
        height: bMapMaximized ? page.height : (page.height - (listView.y + listView.height))
        anchors.bottom: parent.bottom

        center: QtPositioning.coordinate(51.9854, 9.2743)
        zoomLevel: 8.0
        minimumZoomLevel: 0
        maximumZoomLevel: 20
        pixelRatio: 3.0

        accessToken: "pk.eyJ1IjoiamRyZXNjaGVyIiwiYSI6ImNqYmVta256YTJsdjUzMm1yOXU0cmxibGoifQ.JiMiONJkWdr0mVIjajIFZQ"
        cacheDatabaseDefaultPath: true

        visible: true

        Behavior on height {
            NumberAnimation { duration: 150 }
        }

        Item
        {
            id: centerButton
            anchors.left: parent.left
            anchors.leftMargin: Theme.paddingSmall
            anchors.top: parent.top
            anchors.topMargin: Theme.paddingSmall
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
            anchors.rightMargin: Theme.paddingSmall
            anchors.top: parent.top
            anchors.topMargin: Theme.paddingSmall
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
}
