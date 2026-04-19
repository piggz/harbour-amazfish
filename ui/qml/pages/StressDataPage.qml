import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Stress")

    property alias day: nav.day
    property real relaxed: 0
    property real mild: 0
    property real moderate: 0
    property real high: 0
    property real minstress: 0
    property real maxstress: 0
    property real total: relaxed + mild + moderate + high
    property real totalstress: 0
    property real avgstress: totalstress / total

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
            text: qsTr("Download Stress Data")
            onClicked: DaemonInterfaceInstance.fetchData(Amazfish.TYPE_STRESS);
        }
    }

    Column {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        LabelPL {
            id: lblStressAvk
            font.pixelSize: styler.themeFontSizeExtraLarge * 3
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Average: %1%").arg(Math.round(avgstress))
            horizontalAlignment: Text.AlignHCenter
        }

        Row { //Min and Max Stress
            //height: childrenRect.height
            anchors.horizontalCenter: parent.horizontalCenter
            IconPL { iconName: styler.iconDown; iconHeight: styler.themeIconSizeSmall; iconWidth: styler.themeIconSizeSmall }
            LabelPL { text: minstress; anchors.verticalCenter: parent.verticalCenter
            }
            IconPL { iconName: styler.iconUp; iconHeight: styler.themeIconSizeSmall; iconWidth: styler.themeIconSizeSmall }
            LabelPL { text: maxstress; anchors.verticalCenter: parent.verticalCenter
            }
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

        Graph {
            id: graphStressAuto
            graphTitle: qsTr("Stress")
            graphHeight: 300

            axisX.mask: "hh:mm"
            axisY.units: qsTr("%")
            type: DataSource.StressAuto
            graphType: bar

            minY: 1
            maxY: 100

            colorMap: [
                {"limit": 39, "color": "lightblue"},
                {"limit": 59, "color": "green"},
                {"limit": 79, "color": "orange"},
            ]
            defaultColor: "red"

            valueConverter: function(value) {
                return value.toFixed(0);
            }
            onClicked: {
                updateGraph(day);
            }
        }

        //Type summary
        Grid {
            columns: 2
            spacing: styler.themePaddingMedium
            width: parent.width - (styler.themePaddingMedium * 2)
            LabelPL {text: qsTr("Relaxed")}
            Item {
                width: parent.width * 0.5
                height: 50
                Rectangle { color: "lightblue"; width: parent.width * (relaxed  / total) ; height: parent.height }
                LabelPL { text: Math.round((relaxed / total) * 100) + "%"; anchors.centerIn: parent}
            }

            LabelPL {text: qsTr("Mild")}
            Item {
                width: parent.width * 0.5
                height: 50
                Rectangle { color: "green"; width: parent.width * (mild  / total) ; height: parent.height }
                LabelPL { text: Math.round((mild / total) * 100) + "%"; anchors.centerIn: parent}
            }

            LabelPL {text: qsTr("Moderate")}
            Item {
                width: parent.width * 0.5
                height: 50
                Rectangle { color: "orange"; width: parent.width * (moderate  / total) ; height: parent.height }
                LabelPL { text: Math.round((moderate / total) * 100) + "%"; anchors.centerIn: parent}
            }

            LabelPL {text: qsTr("High")}
            Item {
                width: parent.width * 0.5
                height: 50
                Rectangle { color: "red"; width: parent.width * (high  / total) ; height: parent.height }
                LabelPL { text: Math.round((high / total) * 100) + "%"; anchors.centerIn: parent}
            }

        }

    }

    function calculateZones() {
        var points = graphStressAuto.points;
        var end = points.length;

        relaxed = 0;
        mild = 0;
        moderate = 0;
        high = 0;
        minstress = 0;
        maxstress = 0;

        for (var i = 0; i < end; i++) {
            var point = points[i];
            totalstress += point.y;
            if (point.y >= 80) {
                high++;
            } else if (point.y >= 60) {
                moderate++;
            } else if (point.y >= 40) {
                mild++
            } else {
                relaxed++;
            }
            if (point.y > maxstress) {
                maxstress = point.y;
            }
            if (minstress == 0) {
                minstress = point.y;
            }
            if (point.y > 0 && point.y < minstress)  {
                minstress = point.y;
            }
        }
    }

    function updateGraphs() {
        graphStressAuto.updateGraph(day);
        calculateZones();
    }

    Component.onCompleted: {
        day = new Date();
        updateGraphs();
    }
}
