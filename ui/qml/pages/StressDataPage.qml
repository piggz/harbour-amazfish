import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Stress")

    property alias day: nav.day
    // number of automatic readings per level, see levelOf()
    property var levelCounts: [0, 0, 0, 0]
    readonly property int readingCount: levelCounts[0] + levelCounts[1] + levelCounts[2] + levelCounts[3]
    readonly property var levelColors: [styler.chartStressRelaxedColor, styler.chartStressMildColor,
                                        styler.chartStressModerateColor, styler.chartStressHighColor]
    readonly property var levelNames: [qsTr("Relaxed"), qsTr("Mild"), qsTr("Moderate"), qsTr("High")]
    property var manualReading: null

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
            text: qsTr("Download Stress Data")
            onClicked: DaemonInterfaceInstance.fetchData(Amazfish.TYPE_STRESS);
        }
    }

    // 0 relaxed (below 40), 1 mild (40-59), 2 moderate (60-79), 3 high (80 and more)
    function levelOf(value) {
        return value >= 80 ? 3 : value >= 60 ? 2 : value >= 40 ? 1 : 0;
    }

    function levelColor(value) {
        return levelColors[levelOf(value)];
    }

    function percentOf(level) {
        return readingCount ? Math.round(levelCounts[level] / readingCount * 100) : 0;
    }

    function formatTime(seconds) {
        var d = new Date(seconds * 1000);
        return Qt.formatDate(d, "ddd d.M.") + " - " + d.toLocaleTimeString(Qt.locale(), Locale.ShortFormat);
    }

    Column {
        id: column
        x: styler.themeHorizontalPageMargin
        width: parent.width - 2 * x
        spacing: styler.themePaddingLarge

        LabelPL {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: dayChart.noData ? "-" : Math.round(dayChart.average)
            color: dayChart.noData ? styler.themeSecondaryColor : levelColor(dayChart.average)
            font.pixelSize: styler.themeFontSizeExtraLarge * 2
        }

        LabelPL {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            color: styler.themeSecondaryColor
            font.pixelSize: styler.themeFontSizeSmall
            text: dayChart.noData ? qsTr("No data") : qsTr("Average stress of the day")
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
            clickable: true
            title: qsTr("Stress")
            info: dayChart.noData ? "" : qsTr("Min %1 - Max %2").arg(Math.round(dayChart.minimum))
                                                                         .arg(Math.round(dayChart.maximum))
            onClicked: updateGraphs()

            DayChart {
                id: dayChart
                bars: true
                minY: 0
                maxY: 100
                colorFor: levelColor
                guides: [
                    { value: 40, color: styler.chartStressMildGuideColor },
                    { value: 60, color: styler.chartStressModerateGuideColor },
                    { value: 80, color: styler.chartStressHighGuideColor }
                ]
            }

            ChartLegend {
                items: [
                    { color: levelColors[0], label: levelNames[0] },
                    { color: levelColors[1], label: levelNames[1] },
                    { color: levelColors[2], label: levelNames[2] },
                    { color: levelColors[3], label: levelNames[3] }
                ]
            }
        }

        ChartCard {
            title: qsTr("Stress levels")
            visible: readingCount > 0

            // share of the readings per level as one segmented bar
            Row {
                width: parent.width
                height: styler.themeFontSizeExtraSmall
                spacing: 0

                Repeater {
                    model: 4
                    delegate: Rectangle {
                        width: readingCount ? parent.width * levelCounts[index] / readingCount : 0
                        height: parent.height
                        color: levelColors[index]
                    }
                }
            }

            Repeater {
                model: 4
                delegate: DetailRow {
                    label: levelNames[index]
                    value: qsTr("%1 %").arg(percentOf(index))
                }
            }
        }

        ChartCard {
            clickable: true
            title: qsTr("Stress Summary")
            info: qsTr("Last %n day(s)", "", 11)
            onClicked: updateGraphs()

            SummaryBarChart {
                id: summaryChart
                minY: 0
                maxY: 100
                colorFor: levelColor
                labelMask: "d.M."
            }
        }

        ChartCard {
            title: qsTr("Last Manual Reading")

            DetailRow {
                label: qsTr("Stress")
                value: manualReading ? Math.round(manualReading.y) + " - " + levelNames[levelOf(manualReading.y)] : "-"
            }
            DetailRow {
                label: qsTr("Time")
                value: manualReading ? formatTime(manualReading.x) : "-"
            }
        }
    }

    function updateGraphs() {
        var start = new Date(day);
        start.setHours(0, 0, 0, 0);
        // 0 means the watch could not measure
        var all = dataSource.data(DataSource.StressAuto, day);
        var points = [];
        var counts = [0, 0, 0, 0];
        for (var i = 0; i < all.length; i++) {
            if (all[i].y <= 0) continue;
            points.push(all[i]);
            counts[levelOf(all[i].y)]++;
        }
        levelCounts = counts;
        dayChart.startTime = start.getTime() / 1000;
        dayChart.points = points;
        summaryChart.points = dataSource.data(DataSource.StressSummary, day);

        var manual = dataSource.data(DataSource.StressManual, day);
        manualReading = manual.length > 0 ? manual[0] : null;
    }

    Component.onCompleted: {
        day = new Date();
        updateGraphs();
    }
}
