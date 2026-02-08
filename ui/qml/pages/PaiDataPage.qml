import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("PAI")

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
            text: qsTr("Download PAI")
            onClicked: DaemonInterfaceInstance.fetchData(Amazfish.TYPE_PAI);
        }
    }

    Component {
        id: paiColumnDelegate
        Column {
            id: item
            property int value
            property int time
            property color fillColor
            Rectangle {
                width: styler.themeItemSizeLarge
                height: width
                radius: width / 2
                color: fillColor
                Text {
                    id: txtLow
                    anchors.centerIn: parent
                    anchors.fill: parent
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: parent.height / 2
                    text: value
                }
            }
            LabelPL {
                id: txtTimeLow
                font.pixelSize: styler.themeFontSizeLarge
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("%1 min").arg(time)
            }
        }
    }

    Column {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        LabelPL {
            id: lblHeading
            font.pixelSize: styler.themeFontSizeExtraLarge * 2
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Last 7 Days")
            horizontalAlignment: Text.AlignHCenter
        }

        //PAI 7 day chart
        GraphData {
            id: graphPAI
            anchors.margins: styler.themePaddingLarge
            graphTitle: ""
            graphHeight: 300
            axisX.mask: ""

            axisY.units: ""
            graphType: 2

            minY: 0
            maxY: 200
            valueConverter: function(value) {
                return value.toFixed(0);
            }
            onClicked: {
                updateData();
            }
        }

        LabelPL {
            id: lblNowHeading
            font.pixelSize: styler.themeFontSizeExtraLarge
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Now")
            horizontalAlignment: Text.AlignHCenter
        }

        LabelPL {
            id: lblNow
            font.pixelSize: styler.themeFontSizeExtraLarge * 2
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: PaiModel.get(PaiModel.rowCount() - 1).pai_total_today
            horizontalAlignment: Text.AlignHCenter
        }

        LabelPL {
            id: lblTodayHeading
            font.pixelSize: styler.themeFontSizeExtraLarge
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            text: qsTr("Earned Today")
            horizontalAlignment: Text.AlignHCenter
        }

        LabelPL {
            id: lblToday
            font.pixelSize: styler.themeFontSizeExtraLarge * 2
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
        }

        Row {
            width: childrenRect.width
            height: childrenRect.height
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: styler.themePaddingLarge * 2

            Loader {
                sourceComponent: paiColumnDelegate
                id: low
                Component.onCompleted: {
                    low.item.fillColor = "#f1c984"
                }
            }
            Loader {
                sourceComponent: paiColumnDelegate
                id: moderate

                Component.onCompleted: {
                    moderate.item.fillColor = "#5fc5dc"
                }
            }
            Loader {
                sourceComponent: paiColumnDelegate
                id: high
                Component.onCompleted: {
                    high.item.fillColor = "#32a32d"
                }
            }
        }

    }

    Component.onCompleted: {
        PaiModel.update();

        updateData();
    }

    function updateData() {
        var data = [];

        for(var i = 0; i < PaiModel.rowCount(); i++) {
            var rec = {
                x: PaiModel.get(i).pai_day.getTime() / 1000,
                y: PaiModel.get(i).pai_total
            }
            data.push(rec);
        }
        graphPAI.setPoints(data);

        var maybeToday = PaiModel.get(PaiModel.rowCount() - 1);
        lblNow.text = maybeToday.pai_total.toFixed(1)

        var now = new Date();
        now.setHours(0,0,0,0);
        var latest = maybeToday.pai_day;
        latest.setHours(0,0,0,0);

        if (maybeToday.pai_day.getTime() === now.getTime()) {
            lblToday.text = PaiModel.get(PaiModel.rowCount() - 1).pai_total_today.toFixed(1)
            low.item.value = maybeToday.pai_low.toFixed(0);
            moderate.item.value = maybeToday.pai_moderate.toFixed(0);
            high.item.value = maybeToday.pai_high.toFixed(0);
            low.item.time = maybeToday.pai_time_low;
            moderate.item.time = maybeToday.pai_time_moderate;
            high.item.time = maybeToday.pai_time_high;
        } else {
            console.log("latest PAI record is not today");
            lblToday.text = 0.0
            low.item.value = "0"
            moderate.item.value = "0"
            high.item.value = "0"
            low.item.time = "0"
            moderate.item.time = "0"
            high.item.time = "0"
        }
    }
}
