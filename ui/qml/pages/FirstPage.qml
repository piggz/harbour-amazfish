import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import org.SfietKonstantin.weatherfish 1.0
import uk.co.piggz.amazfish 1.0
import "../components/"

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property bool needsProfileSet: false
    property var day: new Date()

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

    ConfigurationValue {
        id: profileName
        key: "/uk/co/piggz/amazfish/profile/name"
        defaultValue: ""
    }

    ConfigurationValue {
        id: fitnessGoal
        key: "/uk/co/piggz/amazfish/profile/fitnessgoal"
        defaultValue: "10000"
    }
    
    onStatusChanged: {
        if (status === PageStatus.Active) {
            pageStack.pushAttached(Qt.resolvedUrl("StepsPage.qml"))
        }
    }
    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView
        PullDownMenu {
            MenuItem {
                text: qsTr("Pair with watch")
                onClicked: pageStack.push(Qt.resolvedUrl("PairSelectDeviceType.qml"))
            }
            MenuItem {
                text: qsTr("Download File")
                onClicked: pageStack.push(Qt.resolvedUrl("BipFirmwarePage.qml"))
            }
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("Settings-menu.qml"))
            }
            MenuItem {
                text: DaemonInterfaceInstance.connectionState == "disconnected" ? qsTr("Connect to watch") : qsTr("Disconnect from watch")
                onClicked: {
                    if (DaemonInterfaceInstance.connectionState == "disconnected") {
                        DaemonInterfaceInstance.connectToDevice(pairedAddress.value);
                    } else {
                        DaemonInterfaceInstance.disconnect();
                    }
                }
            }
        }

        // Tell SilicaFlickable the height of its content.
        contentHeight: column.height

        // Place our content in a Column.  The PageHeader is always placed at the top
        // of the page, followed by our content.
        Column {
            id: column
            x: Theme.horizontalPageMargin
            width: page.width - 2*Theme.horizontalPageMargin
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("Amazfish")
            }
            Row {
                spacing: Theme.paddingLarge

                Label {
                    text: pairedName.value
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeExtraLarge
                }
                Item {
                    width: childrenRect.width
                    height: childrenRect.height
                    BusyIndicator {
                        size: BusyIndicatorSize.Medium
                        visible: DaemonInterfaceInstance.connectionState === "connecting"
                        running: DaemonInterfaceInstance.connectionState === "connecting"
                    }
                    Image {
                        source: "image://theme/icon-m-bluetooth-device"
                        visible: DaemonInterfaceInstance.connectionState === "connected" || DaemonInterfaceInstance.connectionState === "authenticated"
                    }
                }
                Item {
                    width: childrenRect.width
                    height: childrenRect.height
                    BusyIndicator {
                        size: BusyIndicatorSize.Medium
                        visible: DaemonInterfaceInstance.connectionState === "connected"
                        running: DaemonInterfaceInstance.connectionState === "connected"
                    }
                    Image {
                        source: "image://theme/icon-m-watch"
                        visible: DaemonInterfaceInstance.connectionState === "authenticated"
                    }
                }

            }

            Separator {
                width: parent.width
                horizontalAlignment: Qt.AlignHCenter
                color: Theme.highlightColor
            }

            // battery
            Row {
                width: parent.width

                Image {
                    id: imgBattery
                    source: "image://theme/icon-m-battery"
                    width: Theme.iconSizeMedium
                    height: width
                }

                ProgressBar {
                    id: btryProgress
                    width: parent.width - imgBattery.width
                    minimumValue: 0
                    maximumValue: 100
                }
            }

            Separator {
                width: parent.width
                horizontalAlignment: Qt.AlignHCenter
                color: Theme.highlightColor
            }

            // steps
            Image {
                id: imgSteps
                source: "../pics/icon-m-steps.png"
                height: Theme.iconSizeMedium
                width: height
            }

            PercentCircle {
                id: stpsCircle

                anchors.horizontalCenter: parent.horizontalCenter
                size: parent.width / 2.5
                percent: 0.06
                widthRatio: 0.08

                Label {
                    id: lblSteps
                    anchors.centerIn: parent
                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeMedium
                    height: Theme.iconSizeMedium
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Label {
                id: lblGoal
                anchors.horizontalCenter: parent.horizontalCenter
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
                height: Theme.iconSizeMedium
                verticalAlignment: Text.AlignVCenter
                width: parent.width
                text: qsTr("Goal: ") + fitnessGoal.value + qsTr(" Steps")
            }

            Separator {
                width: parent.width
                horizontalAlignment: Qt.AlignHCenter
                color: Theme.highlightColor
            }

            //Heartrate
            Row {
                spacing: Theme.paddingLarge
                width: parent.width
                visible: supportsFeature(DaemonInterface.FEATURE_HRM)

                Image {
                    id: imgHeartrate
                    source: "../pics/icon-m-heartrate.png"
                    width: Theme.iconSizeMedium
                    height: width
                }
                Label {
                    id: lblHeartrate
                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeLarge
                    height: Theme.iconSizeMedium
                    verticalAlignment: Text.AlignVCenter
                    width: parent.width - imgHeartrate.width - btnHR.width - 2* Theme.paddingLarge
                }

                IconButton {
                    id: btnHR
                    icon.source: "image://theme/icon-m-refresh"
                    onClicked: {
                        DaemonInterfaceInstance.requestManualHeartrate();
                    }
                }
            }

            Button {
                text: qsTr("Start Service")
                visible: serviceActiveState == false
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: {
                    systemdServiceIface.call("Start", ["replace"])
                }
            }

            Button {
                text: qsTr("Enable Service")
                visible: serviceEnabledState == false
                anchors.horizontalCenter: parent.horizontalCenter

                onClicked: {
                    systemdManager.enableService();
                }
            }
        }
    }

    Timer {
        id: tmrStartup
        running: true
        repeat: false
        interval: 200
        onTriggered: {
            if (needsProfileSet) {
                pageStack.push(Qt.resolvedUrl("Settings-profile.qml"))
            }
        }
    }

    Connections {
        target: DaemonInterfaceInstance
        onConnectionStateChanged: {
            console.log(DaemonInterfaceInstance.connectionState);
            if (DaemonInterfaceInstance.connectionState === "authenticated") {
                supportedFeatures = DaemonInterfaceInstance.supportedFeatures();
                console.log(supportedFeatures);

                DaemonInterfaceInstance.refreshInformation();
            }
        }
        onInformationChanged: {
            console.log("Information changed", infoKey, infoValue);

            switch (infoKey) {
            case DaemonInterface.INFO_BATTERY:
                btryProgress.label = infoValue + "%"
                btryProgress.value = infoValue
                break;
            case DaemonInterface.INFO_HEARTRATE:
                lblHeartrate.text = infoValue + " bpm"
                break;
            case DaemonInterface.INFO_STEPS:
                lblSteps.text = infoValue
                stpsCircle.percent = infoValue / fitnessGoal.value
                break;
            }
        }
    }

    Component.onCompleted: {
        if (profileName.value === "") {
            needsProfileSet = true;
            return;
        }
        if (DaemonInterfaceInstance.connectionState === "authenticated") {
            DaemonInterfaceInstance.refreshInformation();
            supportedFeatures = DaemonInterfaceInstance.supportedFeatures();
        }
    }
}
