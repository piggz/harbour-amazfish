import "../components/"
import "../components/platform"
import "../components/Translation.js" as T
import QtQuick 2.0
import QtQuick.Layouts 1.1
import uk.co.piggz.amazfish 1.0

DialogListPL {
    id: dialog
    property string kindstring

    model: ListModel {
        id: actionKindModel;
    }



    delegate: ListItemPL {
        contentHeight: styler.themeItemSizeSmall + (styler.themePaddingMedium * 2)

        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width
        highlighted: currentIndex === index

        Loader {
            id: workoutImage

            anchors.top: parent.top
            anchors.topMargin: styler.themePaddingMedium
            width: styler.themeItemSizeSmall
            height: width

            sourceComponent: IconPL {
                iconSource: styler.activityIconPrefix + "icon-m-" + kindstring.toLowerCase() + styler.customIconSuffix
                width: styler.themeItemSizeSmall
                height: width
            }

        }
        LabelPL {
            height: styler.themeItemSizeSmall

            anchors.margins: styler.themePaddingMedium
            anchors.left: workoutImage.right
            anchors.right: parent.right
            verticalAlignment: Text.AlignVCenter
            text: name
        }

        onClicked: {
            currentIndex = index
            dialog.kindstring = kindstring
        }

    }

    Component.onCompleted: {
        var i = 0;
        for (var key in T.activityKindTranslations) {
            if (T.activityKindTranslations.hasOwnProperty(key)) {
                actionKindModel.append({
                    "kindstring": key,
                    "name": T.activityKindTranslations[key]
                })
                if (key === kindstring) {
                    currentIndex = i
                }

                i++;
            }
        }
    }
}
