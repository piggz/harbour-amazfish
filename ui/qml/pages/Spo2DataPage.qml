import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Blood Oxygen")

    property alias day: nav.day

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
            text: qsTr("Download SPO2")
            onClicked: DaemonInterfaceInstance.fetchData(Amazfish.TYPE_SPO2);
        }
    }

    function percent(v) {
        return qsTr("%1 %").arg(Math.round(v));
    }

    Column {
        id: column
        x: styler.themeHorizontalPageMargin
        width: parent.width - 2 * x
        spacing: styler.themePaddingLarge

        LabelPL {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: normalChart.noData ? "-" : percent(normalChart.lastValue)
            color: normalChart.noData ? styler.themeSecondaryColor : styler.chartSpo2Color
            font.pixelSize: styler.themeFontSizeExtraLarge * 2
        }

        LabelPL {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            color: styler.themeSecondaryColor
            font.pixelSize: styler.themeFontSizeSmall
            text: normalChart.noData ? qsTr("No data") : qsTr("Latest reading")
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
            title: qsTr("Normal SPO2")
            info: qsTr("Last %n day(s)", "", 11)
            onClicked: updateGraphs()

            SummaryBarChart {
                id: normalChart
                minY: 80
                maxY: 100
                colorY: styler.chartSpo2Color
                labelMask: "d.M."
                averageLabel: percent
            }
        }

        ChartCard {
            clickable: true
            title: qsTr("Sleep SPO2")
            info: qsTr("Last %n day(s)", "", 11)
            onClicked: updateGraphs()

            SummaryBarChart {
                id: sleepChart
                minY: 80
                maxY: 100
                colorY: styler.chartSpo2SleepColor
                labelMask: "d.M."
                averageLabel: percent
            }
        }
    }

    function updateGraphs() {
        normalChart.points = dataSource.data(DataSource.Spo2Normal, day);
        sleepChart.points = dataSource.data(DataSource.Spo2Sleep, day);
    }

    Component.onCompleted: {
        day = new Date();
        updateGraphs();
    }
}
