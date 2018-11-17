import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property string devicePath: "";
    property string deviceName: "";

    property bool tryAgainAvail: false

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

    Timer {
        id: tmrScan
        interval: 30000
        running: false
        repeat: false
        onTriggered: {
            BluezAdapter.stopDiscovery();
            pair();
        }
    }

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
        PullDownMenu {
            MenuItem {
                text: qsTr("Start scan")
                onClicked: {

                    BluezAdapter.startDiscovery();
                    tmrScan.start();
                    lblStatus.text = "Searching...";
                    tryAgainAvail = false
                }

            }
        }

        Label {
            id: lblStatus
            anchors.centerIn: parent
        }
        Label {
            id: lblWatch
            text: DeviceInterface.connectionState
            anchors.top: lblStatus.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: Theme.paddingMedium
        }
        Button {
            id: btnTryAgain
            text: qsTr("Try again");
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.margins: Theme.paddingMedium
            anchors.top: lblWatch.bottom
            visible: tryAgainAvail

            onClicked: {
                pair();
            }
        }

    }

    function pair()
    {
        var path = BluezAdapter.matchDevice("Amazfit");
        if (path === "") {
            path = BluezAdapter.matchDevice("MI Band 2");
            if (path === ""){
                lblStatus.text = "Watch not found";
                return;
            }
        }

        devicePath = path;
        deviceName = BluezAdapter.deviceName(path);

        lblStatus.text = "Connecting to watch...\n" + path
        var err = DeviceInterface.pair(deviceName, path);

        console.log(err);

        if (err !== "") {
            tryAgainAvail = true;
        }
    }

    Connections {
        target: DeviceInterface
        onConnectionStateChanged: {
            if (DeviceInterface.connectionState === "authenticated") {
                pairedAddress.value = devicePath;
                pairedName.value = deviceName;
                pairedAddress.sync();
                pairedName.sync()
            }
        }
    }

}
