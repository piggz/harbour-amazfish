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

            Column {
                Rectangle {
                    width: styler.themeItemSizeLarge
                    height: width
                    radius: width / 2
                    color: "#f1c984"
                    Text {
                        id: txtLow
                        anchors.centerIn: parent
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: parent.height / 2
                    }
                }
                LabelPL {
                    id: txtTimeLow
                    font.pixelSize: styler.themeFontSizeLarge
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }

            Column {
                Rectangle {
                    width: styler.themeItemSizeLarge
                    height: width
                    radius: width / 2
                    color: "#5fc5dc"
                    Text {
                        id: txtModerate
                        anchors.centerIn: parent
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: parent.height / 2
                    }
                }
                LabelPL {
                    id: txtTimeModerate
                    font.pixelSize: styler.themeFontSizeLarge
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }

            Column {
                Rectangle {
                    width: styler.themeItemSizeLarge
                    height: width
                    radius: width / 2
                    color: "#32a32d"
                    Text {
                        id: txtHigh
                        anchors.centerIn: parent
                        anchors.fill: parent
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: parent.height / 2
                    }
                }
                LabelPL {
                    id: txtTimeHigh
                    font.pixelSize: styler.themeFontSizeLarge
                    anchors.horizontalCenter: parent.horizontalCenter
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
            txtLow.text = maybeToday.pai_low.toFixed(0);
            txtModerate.text = maybeToday.pai_moderate.toFixed(0);
            txtHigh.text = maybeToday.pai_high.toFixed(0);

            txtTimeLow.text = maybeToday.pai_time_low + qsTr(" min");
            txtTimeModerate.text = maybeToday.pai_time_moderate + qsTr(" min");
            txtTimeHigh.text = maybeToday.pai_time_high + qsTr(" min");
        } else {
            console.log("latest PAI record is not today");
            lblToday.text = 0.0

            txtLow.text = "0"
            txtModerate.text = "0"
            txtHigh.text = "0"

            txtTimeLow.text = "0" + qsTr(" min");
            txtTimeModerate.text = "0" + qsTr(" min");
            txtTimeHigh.text = "0" + qsTr(" min");
        }
    }
}
