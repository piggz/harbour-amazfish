import QtQuick 2.0
import QtQuick.Layouts 1.1
import "./platform"

Tile {
    id: tile

    property int stepCount: 0
    property int stepGoal: 0

    text: qsTr("Steps")

    contentItem: PercentCircle {
        id: stpsCircle
        anchors.horizontalCenter: parent.horizontalCenter
        size: parent.width - styler.themeHorizontalPageMargin * 4
        percent: stepCount ? stepCount / stepGoal : 0.06
        widthRatio: 0.08

        Item {
            anchors.centerIn: parent
            height: lblSteps.height + lblGoal.height + styler.paddingSmall
            width: Math.max(lblSteps.width, lblGoal.width)

            LabelPL {
                id: lblSteps
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: centerItem.top
                }
                color: styler.themeHighlightColor
                font.pixelSize: styler.themeFontSizeHuge
                verticalAlignment: Text.AlignVCenter
                text: stepCount.toLocaleString()
            }

            Item {
                id: centerItem
                width: 1
                height: 1
                anchors.centerIn: parent
            }

            LabelPL {
                id: lblGoal
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: centerItem.bottom
                    topMargin: styler.themePaddingSmall
                }
                color: styler.blockBg
                font.pixelSize: styler.themeFontSizeExtraLarge
                verticalAlignment: Text.AlignVCenter
                text: stepGoal.toLocaleString()
            }
        }

    }
}
