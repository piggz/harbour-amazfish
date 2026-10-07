import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("PAI")

    // PAI aims at 100 over the last 7 days
    readonly property int paiGoal: 100
    property var points: []
    property var latest: null

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
            text: qsTr("Download PAI")
            onClicked: DaemonInterfaceInstance.fetchData(Amazfish.TYPE_PAI);
        }
    }

    // Same levels as the PAI tile on the first page
    function levelColor(pai) {
        if (pai < 50) {
            return styler.chartPaiLowColor;
        }
        return pai < paiGoal ? styler.chartPaiMediumColor : styler.chartPaiHighColor;
    }

    function intensity(pai, minutes) {
        return qsTr("%1 PAI - %2 min").arg(Number(pai).toFixed(1)).arg(minutes);
    }

    Column {
        id: column
        x: styler.themeHorizontalPageMargin
        width: parent.width - 2 * x
        spacing: styler.themePaddingLarge

        LabelPL {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: latest ? Math.round(latest.pai_total) : "-"
            color: latest ? levelColor(latest.pai_total) : styler.themeSecondaryColor
            font.pixelSize: styler.themeFontSizeExtraLarge * 2
        }

        LabelPL {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            color: styler.themeSecondaryColor
            font.pixelSize: styler.themeFontSizeSmall
            wrapMode: Text.WordWrap
            text: latest ? qsTr("PAI of the last 7 days, as of %1").arg(Qt.formatDate(latest.pai_day, "ddd d.M."))
                         : qsTr("No data")
        }

        ChartCard {
            title: qsTr("PAI")
            info: qsTr("Last 7 Days")
            onClicked: updateData()

            SummaryBarChart {
                id: paiChart
                points: page.points
                goal: paiGoal
                colorY: styler.chartPaiHighColor
                colorBelowGoal: styler.chartPaiMediumColor
                showAverage: false
            }

            ChartLegend {
                items: [
                    { color: styler.chartPaiHighColor, label: qsTr("Goal reached") },
                    { color: styler.chartPaiMediumColor, label: qsTr("Below goal") },
                    { color: styler.chartGoalColor, label: qsTr("Goal %1").arg(paiGoal), line: true }
                ]
            }
        }

        ChartCard {
            title: qsTr("Latest day")
            info: latest ? Qt.formatDate(latest.pai_day, "ddd d.M.") : ""
            visible: latest !== null

            DetailRow {
                label: qsTr("Earned")
                value: latest ? qsTr("%1 PAI").arg(Number(latest.pai_total_today).toFixed(1)) : "-"
            }
            DetailRow {
                label: qsTr("Low intensity")
                value: latest ? intensity(latest.pai_low, latest.pai_time_low) : "-"
            }
            DetailRow {
                label: qsTr("Moderate intensity")
                value: latest ? intensity(latest.pai_moderate, latest.pai_time_moderate) : "-"
            }
            DetailRow {
                label: qsTr("High intensity")
                value: latest ? intensity(latest.pai_high, latest.pai_time_high) : "-"
            }
        }
    }

    // Show new data once a download from the watch has finished
    Connections {
        target: DaemonInterfaceInstance
        onOperationRunningChanged: {
            if (!DaemonInterfaceInstance.operationRunning) {
                updateData();
            }
        }
    }

    Component.onCompleted: {
        updateData();
    }

    function updateData() {
        PaiModel.update();
        var data = [];
        var count = PaiModel.rowCount();
        for (var i = 0; i < count; i++) {
            var r = PaiModel.get(i);
            data.push({ x: r.pai_day.getTime() / 1000, y: r.pai_total });
        }
        points = data;
        latest = count > 0 ? PaiModel.get(count - 1) : null;
    }
}
