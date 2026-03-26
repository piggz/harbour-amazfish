import "../components/"
import "../components/platform"
import "../components/Translation.js" as T
import QtQuick 2.0
import QtQuick.Layouts 1.1
import uk.co.piggz.amazfish 1.0

DialogListPL {
    id: dialog
    property string kindstring

    title: qsTr("Select activity type")

    model: ListModel {
        id: actionKindModel;
    }



    delegate: ListItemPL {
        contentHeight: styler.themeItemSizeSmall + (styler.themePaddingMedium * 2)
        width: dialog.width
        emphasized: currentIndex === index
        Loader {
            id: workoutImage

            anchors.margins: styler.themePaddingMedium
            anchors.top: parent.top
            anchors.left: parent.left
            width: styler.themeItemSizeSmall
            height: width

            sourceComponent: IconPL {
                anchors.margins: styler.themePaddingMedium
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
