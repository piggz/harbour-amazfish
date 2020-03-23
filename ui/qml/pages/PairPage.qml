import QtQuick 2.0
import Sailfish.Silica 1.0
import uk.co.piggz.amazfish 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property string deviceType: "";
    property string devicePath: "";
    property string deviceName: "";
    property bool deviceRequiresAuthKey: false

    property bool tryAgainAvail: false

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
                    lblStatus.text = "Searching for: " + deviceType;
                    tryAgainAvail = false
                }
            }
        }

        Column {
            id: column
            x: Theme.horizontalPageMargin
            width: page.width - 2*Theme.horizontalPageMargin
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("Pair Device")
            }

            Label {
                text: "Enter pairing key"
                visible: deviceRequiresAuthKey
            }

            TextField {
                id: fldAuthKey
                width: parent.width
                visible: deviceRequiresAuthKey
            }

            Button {
                text: qsTr("Save Key")
                anchors.horizontalCenter: parent.horizontalCenter
                visible: deviceRequiresAuthKey

                onClicked: {
                    AmazfishConfig.authKey = fldAuthKey.text;
                }
            }

            Label {
                id: lblStatus
                text: qsTr("Pull down to start scan")
            }

            Label {
                id: lblWatch
                text: DaemonInterfaceInstance.connectionState
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.margins: Theme.paddingMedium
            }
            Button {
                id: btnTryAgain
                text: qsTr("Try again");
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.margins: Theme.paddingMedium
                visible: tryAgainAvail

                onClicked: {
                    pair();
                }
            }

        }


    }

    function pair()
    {
        var path = BluezAdapter.matchDevice(deviceType);
        if (path === "") {
            lblStatus.text = "Watch not found";
            return;
        }

        devicePath = path;
        deviceName = BluezAdapter.deviceName(path);

        lblStatus.text = "Connecting to watch...\n" + path
        var err = DaemonInterfaceInstance.pair(deviceName, path);

        console.log("Pair result:", err);

        if (err !== "") {
            tryAgainAvail = true;
        }
    }

    Connections {
        target: DaemonInterfaceInstance
        onConnectionStateChanged: {
            if (DaemonInterfaceInstance.connectionState === "authenticated") {
                AmazfishConfig.pairedAddress = devicePath;
                AmazfishConfig.pairedName = deviceName;
                pageStack.pop(previousPage(previousPage()));
            }
        }
    }

    Component.onCompleted: {
        fldAuthKey.text = AmazfishConfig.authKey;
    }

}
