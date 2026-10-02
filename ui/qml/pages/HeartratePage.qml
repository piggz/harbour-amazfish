import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"
import "../components/ChartColors.js" as ChartColors

PagePL {
    id: page
    title: qsTr("Heartrate")

    property alias day: nav.day
    property real relaxed: 0
    property real light: 0
    property real intensive: 0
    property real aerobic: 0
    property real anerobic: 0
    property real vo2max: 0
    property real total: relaxed + light + intensive + aerobic + anerobic + vo2max
    property real minhr: 0
    property real maxhr: 0
    property real avghr: 0

    property real maxHRforAge: wingate()

    pageMenu: PageMenuPL {
        DownloadDataMenuItem{}
    }

    Column {
        id: column
        x: styler.themeHorizontalPageMargin
        width: parent.width - 2 * x
        spacing: styler.themePaddingLarge

        LabelPL {
            id: lblCurrentHeartrate
            font.pixelSize: styler.themeFontSizeExtraLarge * 2
            width: parent.width
            text: qsTr("%1 bpm").arg(_InfoHeartrate)
            color: styler.themeHighlightColor
            horizontalAlignment: Text.AlignHCenter
        }

        DateNavigation {
            id: nav
            onBackward: {
                var d = new Date(day);
                d.setDate(day.getDate() - 1);
                day = d;
            }
            onForward: {
                var d = new Date(day);
                d.setDate(day.getDate() + 1);
                day = d;
            }
            onDayChanged: {
                updateGraphs();
            }
        }

        ChartCard {
            title: qsTr("Heartrate")
            onClicked: updateGraphs()

            // resting / average / max, like the header of Gadgetbridge's heart rate chart
            Row {
                width: parent.width
                Repeater {
                    model: [
                        { value: minhr, label: qsTr("Resting") },
                        { value: avghr, label: qsTr("Average") },
                        { value: maxhr, label: qsTr("Max") }
                    ]
                    delegate: Column {
                        width: parent.width / 3
                        LabelPL {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            text: modelData.value ? Math.round(modelData.value) : "-"
                            color: styler.themeHighlightColor
                            font.pixelSize: styler.themeFontSizeExtraLarge
                        }
                        LabelPL {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            text: modelData.label + " · " + qsTr("BPM")
                            color: styler.themeSecondaryHighlightColor
                            font.pixelSize: styler.themeFontSizeExtraSmall
                        }
                    }
                }
            }

            HeartRateChart {
                id: hrChart
                restingHeartrate: minhr
                maxY: Math.max(160, 40 + Math.ceil((maxhr + 10 - 40) / 40) * 40)   // keeps grid steps round
                zoneLimits: [0.5, 0.6, 0.7, 0.8, 0.9, 1.0].map(function(f) { return Math.round(maxHRforAge * f); })
            }

            ChartLegend {
                items: [
                    { color: ChartColors.heartrate, label: qsTr("Heartrate"), line: true },
                    { color: ChartColors.lightSleep, label: qsTr("Resting"), line: true }
                ]
            }
        }

        ChartCard {
            title: qsTr("Time in zones")
            visible: total > 0

            Repeater {
                model: [
                    { name: qsTr("Relaxed"),   count: relaxed,   limit: 0.5 },
                    { name: qsTr("Light"),     count: light,     limit: 0.6 },
                    { name: qsTr("Intensive"), count: intensive, limit: 0.7 },
                    { name: qsTr("Aerobic"),   count: aerobic,   limit: 0.8 },
                    { name: qsTr("Anerobic"),  count: anerobic,  limit: 0.9 },
                    { name: qsTr("VO2 Max"),   count: vo2max,    limit: 1.0 }
                ]
                delegate: Column {
                    width: parent.width
                    spacing: styler.themePaddingSmall

                    Item {
                        width: parent.width
                        height: lblZone.height
                        LabelPL {
                            id: lblZone
                            text: modelData.name + "  ≤ " + Math.round(maxHRforAge * modelData.limit)
                            color: styler.themeSecondaryHighlightColor
                            font.pixelSize: styler.themeFontSizeSmall
                        }
                        LabelPL {
                            anchors.right: parent.right
                            // samples are one minute apart
                            text: ChartColors.formatDuration(modelData.count) + " · " + Math.round(modelData.count / total * 100) + " %"
                            color: styler.themeHighlightColor
                            font.pixelSize: styler.themeFontSizeSmall
                        }
                    }
                    Rectangle {
                        width: parent.width
                        height: styler.themePaddingMedium
                        radius: height / 2
                        color: ChartColors.withAlpha(styler.themeSecondaryColor, 0.2)
                        Rectangle {
                            width: total ? Math.max(height, parent.width * modelData.count / total) : 0
                            height: parent.height
                            radius: height / 2
                            color: ChartColors.zones[index]
                            visible: modelData.count > 0
                        }
                    }
                }
            }
        }
    }

    function updateGraphs() {
        var start = new Date(day);
        start.setHours(0, 0, 0, 0);
        hrChart.startTime = start.getTime() / 1000;
        hrChart.points = dataSource.data(DataSource.Heartrate, day);
        calculateZones();
    }

    function calculateZones() {
        var points = hrChart.points;
        var end = points.length;

        relaxed = 0;
        light = 0;
        intensive = 0;
        aerobic = 0;
        anerobic = 0;
        vo2max = 0;

        minhr = 0;
        maxhr = 0;
        avghr = 0;
        var sum = 0;
        var count = 0;
        for (var i = 0; i < end; i++) {
            var point = points[i];
            if (point.y <= 0) {
                continue;   // no reading
            }
            if (point.y >= (maxHRforAge * 0.9)) {
                vo2max++;
            } else if (point.y >= (maxHRforAge * 0.8)) {
                anerobic++;
            } else if (point.y >= (maxHRforAge * 0.7)) {
                aerobic++
            } else if (point.y >= (maxHRforAge * 0.6)) {
                intensive++;
            } else if (point.y >= (maxHRforAge * 0.5)) {
                light++;
            } else {
                relaxed++;
            }
            if (point.y > maxhr) {
                maxhr = point.y;
            }
            if (minhr == 0) {
                minhr = point.y;
            }

            if (point.y > 0 && point.y < minhr)  {
                minhr = point.y;
            }
            if (point.y > 0) {
                sum += point.y;
                count++;
            }
        }
        avghr = count ? sum / count : 0;
    }

    function wingate() {
        var dob = AmazfishConfig.profileDOB;
        var gender = AmazfishConfig.profileGender;
        var diff_ms = Date.now() - dob.getTime();
        var age_dt = new Date(diff_ms);
        var age = Math.abs(age_dt.getUTCFullYear() - 1970);
        var max_hr;

        // if no age is provided, use an average age
        // this is to avoid providing too height values which may be a health risk
        if (!age) {
            age = 50
        }
        // max HR calculated with Wingate formula as the most recent evaluation with a large test group
        // for details see https://en.wikipedia.org/wiki/Heart_rate#Maximum_heart_rate
        if (gender = 1) { // 1=male
            max_hr = 208.609-(0.716*age)
        } else {
            max_hr = 209.273-(0.804*age)
        }

        return max_hr;
    }

    Component.onCompleted: {
        day = new Date();
        updateGraphs();
        DaemonInterfaceInstance.requestManualHeartrate();
    }
}
