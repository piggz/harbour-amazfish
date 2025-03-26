import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

PageListPL {
    id: page
    title: "Test Icons"

    model: iconsModel

    delegate: ListItemPL {

        contentHeight: styler.themeItemSizeLarge
        IconPL {
            id: settingsIcon
            anchors.verticalCenter: parent.verticalCenter
            iconName: icon
            height: styler.themeItemSizeLarge
            width: height
        }
        LabelPL {
            id: settingsName
            anchors.verticalCenter: settingsIcon.verticalCenter
            anchors.left: settingsIcon.right
            anchors.leftMargin: 20
            text: name
        }
    }


    ListModel {
        id: iconsModel

        property bool completed: false
        Component.onCompleted: {

            var properties = Object.keys(styler); // Get all property names
            var regex = /^icon/; // Regex to match property names starting with "icon"
            var regexEnd = /Changed$/; // Regex to match property names starting with "icon"

            for (var i = 0; i < properties.length; ++i) {
                var propertyName = properties[i];
                    if (regex.test(propertyName) && !regexEnd.test(propertyName)) {
//                        console.log(propertyName + ": " + styler[propertyName]);
                        append({"icon": styler[propertyName], "name": propertyName});
                    }
            }
            completed = true;
        }
    }
}
