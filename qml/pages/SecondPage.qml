import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    ConfigurationValue {
        id: pairedAddress
        key: "/uk/co/piggz/amazfish/pairedAddress"
        defaultValue: ""
    }


    ConfigurationValue {
        id: pairedName
        key: "/uk/co/piggz/amazfish/pairedName"
        defaultValue: ""
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
        PullDownMenu {
            MenuItem {
                text: qsTr("Start scan")
                onClicked: BipPair.startScan()
            }
        }

        Label {
            id: lblStatus
            anchors.centerIn: parent

            text: BipPair.status
        }
        Label {
            id: lblWatch
            anchors.top: lblStatus.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: Theme.paddingMedium
        }
    }

    Connections {
        target: BipPair

        onPairComplete: {
            console.log("Paired with", BipPair.watchName(), BipPair.watchAddress())
            lblWatch.text = "Paired with " + BipPair.watchName() + "\n(" + BipPair.watchAddress() + ")";
            pairedAddress.value = BipPair.watchAddress();
            pairedName.value  = BipPair.watchName();
        }
    }
}
