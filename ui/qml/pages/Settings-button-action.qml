import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import QtQml.Models 2.1

import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Button Actions")

    ListModel {
        id: modelButtonActions
    }

    Column
    {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        ComboBoxPL {
            id: cboDoubleAction
            model: modelButtonActions
            label: qsTr("Double Press Action")
            textRole: "itemText"
            Component.onCompleted: {

            }
        }

        ComboBoxPL {
            id: cboTripleAction
            model: modelButtonActions
            label: qsTr("Triple Press Action")
            textRole: "itemText"
            Component.onCompleted: {

            }
        }

        ComboBoxPL {
            id: cboQuadAction
            model: modelButtonActions
            label: qsTr("Quad Press Action")
            textRole: "itemText"
            Component.onCompleted: {

            }
        }

        ButtonPL {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Save Settings")
            onClicked: {
                saveSettings();
            }
        }
    }


    Component.onCompleted: {
        buildModel();
        cboDoubleAction.currentIndex = modelIndex(AmazfishConfig.appButtonDoublePressAction);
        cboTripleAction.currentIndex = modelIndex(AmazfishConfig.appButtonTriplePressAction);
        cboQuadAction.currentIndex = modelIndex(AmazfishConfig.appButtonQuadPressAction);
    }

    function saveSettings() {
        AmazfishConfig.appButtonDoublePressAction = modelButtonActions.get(cboDoubleAction.currentIndex).itemId;
        AmazfishConfig.appButtonTriplePressAction = modelButtonActions.get(cboTripleAction.currentIndex).itemId;
        AmazfishConfig.appButtonQuadPressAction = modelButtonActions.get(cboQuadAction.currentIndex).itemId;
    }

    function buildModel() {
        modelButtonActions.append({"itemId": "action-none", "itemText": qsTr("No Action")});
        modelButtonActions.append({"itemId": "action-music-next", "itemText": qsTr("Next Track")});
        modelButtonActions.append({"itemId": "action-music-prev", "itemText": qsTr("Previous Track")});
        modelButtonActions.append({"itemId": "action-vol-up", "itemText": qsTr("Volume Up")});
        modelButtonActions.append({"itemId": "action-vol-down", "itemText": qsTr("Volume Down")});
        modelButtonActions.append({"itemId": "action-custom", "itemText": qsTr("Custom Script")});
    }

    function modelIndex(id) {
        for (var i = 0; i < modelButtonActions.count; i++) {
            if(modelButtonActions.get(i).itemId === id) {
                return i;
            }
        }
        return 0;
    }
}


