import QtQuick 2.0
import "./platform"
import "GlassStyle.js" as Glass

Tile {
    text: qsTr("PAI")

    contentItem: GaugeArc {
        id: paiCircle
        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height)
        height: width
        trackColor: Glass.track

        Column {
            anchors.centerIn: parent

            LabelPL {
                id: lblPAITotal
                anchors.horizontalCenter: parent.horizontalCenter
                color: styler.themePrimaryColor
                font.pixelSize: styler.themeFontSizeHuge
            }
            LabelPL {
                id: lblPAIToday
                anchors.horizontalCenter: parent.horizontalCenter
                color: styler.themeSecondaryColor
                font.pixelSize: styler.themeFontSizeSmall
            }
        }
    }

    function update() {
        console.log("Refreshing PAI");
        PaiModel.update();

        var maybeToday = PaiModel.get(PaiModel.rowCount() - 1);
        if (typeof maybeToday.pai_total == "undefined") {
            return;
        }

        var pai_total = maybeToday.pai_total.toFixed(1);
        lblPAITotal.text = pai_total
        paiCircle.value = pai_total / 200 //200 Is a pretty high target, usually > 100 is good

        if (pai_total < 50 ) {
            paiCircle.color = "#f5a623"
        } else if (pai_total < 100 ) {
            paiCircle.color = "#46acea"
        } else {
            paiCircle.color = "#5ad24a"
        }

        var now = new Date();
        now.setHours(0,0,0,0);

        if (maybeToday.pai_day.getTime() === now.getTime()) {
            lblPAIToday.text = qsTr("today %1").arg(PaiModel.get(PaiModel.rowCount() - 1).pai_total_today.toFixed(1))
        } else {
            lblPAIToday.text = qsTr("today %1").arg("0.0")
        }
    }
}
