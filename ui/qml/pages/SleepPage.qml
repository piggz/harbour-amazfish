import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"
import "../components/ChartColors.js" as ChartColors

PagePL {
    id: page
    title: qsTr("Sleep")

    property alias day: nav.day

    // Recommended sleep by age (National Sleep Foundation / AASM). Without a date
    // of birth in the profile the range for adults is used.
    readonly property int ageYears: ageFromDob(AmazfishConfig.profileDOB)
    readonly property var sleepRange: recommendedSleep(ageYears)

    function ageFromDob(dob) {
        var d = new Date(dob);
        if (isNaN(d.getTime()) || d.getFullYear() < 1900) {
            return -1;
        }
        var now = new Date();
        var age = now.getFullYear() - d.getFullYear();
        if (now.getMonth() < d.getMonth() || (now.getMonth() === d.getMonth() && now.getDate() < d.getDate())) {
            age--;
        }
        return age;
    }

    function recommendedSleep(age) {
        if (age < 0)   return { low: 7, high: 9 };     // unknown: adults
        if (age < 6)   return { low: 10, high: 13 };
        if (age < 13)  return { low: 9, high: 12 };
        if (age < 18)  return { low: 8, high: 10 };
        if (age < 65)  return { low: 7, high: 9 };
        return { low: 7, high: 8 };
    }

    // Values of the night ending on 'day', from the same calculation as before
    readonly property var lastNight: summaryChart.points.length ? summaryChart.points[summaryChart.points.length - 1] : null
    readonly property real lightHours: lastNight ? lastNight.y : 0
    readonly property real deepHours: lastNight ? lastNight.z : 0

    pageMenu: PageMenuPL {
        DownloadDataMenuItem{}
    }

    Column {
        id: column
        x: styler.themeHorizontalPageMargin
        width: parent.width - 2 * x
        spacing: styler.themePaddingLarge

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
            title: qsTr("Last night")
            info: phaseChart.noData ? "" : Qt.formatTime(phaseChart.sleepStart, "hh:mm") + " – " + Qt.formatTime(phaseChart.sleepEnd, "hh:mm")
            onClicked: updateGraphs()

            LabelPL {
                id: lblSleepLastnight
                text: ChartColors.formatDuration((lightHours + deepHours) * 60)
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeHuge
            }

            SleepPhaseChart {
                id: phaseChart
            }

            // share of each phase, as one segmented bar
            Row {
                id: shareBar
                width: parent.width
                height: styler.themePaddingMedium
                visible: lightHours + deepHours > 0
                readonly property real total: deepHours + lightHours + phaseChart.awakeMinutes / 60
                Rectangle { width: shareBar.total ? shareBar.width * deepHours / shareBar.total : 0; height: parent.height; color: ChartColors.deepSleep }
                Rectangle { width: shareBar.total ? shareBar.width * lightHours / shareBar.total : 0; height: parent.height; color: ChartColors.lightSleep }
                Rectangle { width: shareBar.total ? shareBar.width * (phaseChart.awakeMinutes / 60) / shareBar.total : 0; height: parent.height; color: ChartColors.awake }
            }

            DetailRow { label: qsTr("Deep sleep"); value: ChartColors.formatDuration(deepHours * 60) }
            DetailRow { label: qsTr("Light sleep"); value: ChartColors.formatDuration(lightHours * 60) }
            DetailRow { label: qsTr("Awake"); value: ChartColors.formatDuration(phaseChart.awakeMinutes); visible: !phaseChart.noData }
        }

        ChartCard {
            title: qsTr("Sleep Summary")
            info: qsTr("Recommended %1–%2 h").arg(sleepRange.low).arg(sleepRange.high)
            onClicked: updateGraphs()

            SummaryBarChart {
                id: summaryChart
                bandLow: sleepRange.low
                bandHigh: sleepRange.high
                colorY: ChartColors.lightSleep
                colorZ: ChartColors.deepSleep
                valueLabel: function(v) {
                    return v.toLocaleString(Qt.locale(), "f", v % 1 ? 1 : 0) + " h";
                }
            }

            ChartLegend {
                items: [
                    { color: ChartColors.deepSleep, label: qsTr("Deep sleep") },
                    { color: ChartColors.lightSleep, label: qsTr("Light sleep") },
                    { color: ChartColors.withAlpha(ChartColors.goal, 0.45), label: qsTr("Recommended range") },
                    { color: ChartColors.average, label: qsTr("Average"), line: true }
                ]
            }

            DetailRow { label: qsTr("Average"); value: ChartColors.formatDuration(summaryChart.average * 60) }
            DetailRow { label: qsTr("In recommended range"); value: qsTr("%1 of %2 nights").arg(summaryChart.bandDays).arg(summaryChart.count) }
            DetailRow {
                label: qsTr("Recommendation")
                value: ageYears >= 0
                       ? qsTr("%1–%2 h at age %3").arg(sleepRange.low).arg(sleepRange.high).arg(ageYears)
                       : qsTr("%1–%2 h for adults (no date of birth set)").arg(sleepRange.low).arg(sleepRange.high)
            }
        }
    }

    function updateGraphs() {
        summaryChart.points = dataSource.data(DataSource.SleepSummary, day);
        phaseChart.samples = dataSource.data(DataSource.SleepPhases, day);
    }

    Component.onCompleted: {
        day = new Date();
        updateGraphs();
    }
}
