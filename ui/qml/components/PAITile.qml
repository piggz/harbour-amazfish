import QtQuick 2.0
import "./platform"

Tile {
    text: qsTr("PAI")

    contentItem: PercentCircle {
        id: paiCircle
        anchors.horizontalCenter: parent.horizontalCenter
        size: parent.width - styler.themeHorizontalPageMargin * 4
        widthRatio: 0.08

        Item {
            anchors.centerIn: parent
            height: lblPAITotal.height + lblPAIToday.height + styler.paddingSmall
            width: Math.max(lblPAITotal.width, lblPAIToday.width)

            LabelPL {
                id: lblPAITotal
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: paiCenterItem.top
                }
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeHuge
                verticalAlignment: Text.AlignVCenter
            }

            Item {
                id: paiCenterItem
                width: 1
                height: 1
                anchors.centerIn: parent
            }

            LabelPL {
                id: lblPAIToday
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: paiCenterItem.bottom
                    topMargin: styler.themePaddingSmall
                }
                color: styler.blockBg
                font.pixelSize: styler.themeFontSizeExtraLarge
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    function update() {
        console.log("Refreshing PAI");
        PaiModel.update();

        var maybeToday = PaiModel.get(PaiModel.rowCount() - 1);
        var pai_total = maybeToday.pai_total.toFixed(1);
        lblPAITotal.text = pai_total
        paiCircle.percent = pai_total / 200 //200 Is a pretty high target, usually > 100 is good

        if (pai_total < 50 ) {
            paiCircle.gradientColor = "orange"
        } else if (pai_total < 100 ) {
            paiCircle.gradientColor = "lightblue"
        } else {
            paiCircle.gradientColor = "lightgreen"
        }

        var now = new Date();
        now.setHours(0,0,0,0);

        if (maybeToday.pai_day.getTime() === now.getTime()) {
            lblPAIToday.text = PaiModel.get(PaiModel.rowCount() - 1).pai_total_today.toFixed(1)
        } else {
            lblPAIToday.text = 0.0
        }
    }
}
