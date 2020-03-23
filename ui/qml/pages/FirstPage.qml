import QtQuick 2.0
import Sailfish.Silica 1.0
import org.SfietKonstantin.weatherfish 1.0
import uk.co.piggz.amazfish 1.0
import "../components/"

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    property var day: new Date()

    readonly property string _connectionState: DaemonInterfaceInstance.connectionState
    readonly property bool _disconnected: _connectionState === "disconnected"
    readonly property bool _connecting: _connectionState === "connecting"
    readonly property bool _connected: _connectionState === "connected"
    readonly property bool _authenticated: _connectionState === "authenticated"
    property int _steps: 0

    function _refreshInformation() {
        if (!_authenticated) {
            return
        }

        supportedFeatures = DaemonInterfaceInstance.supportedFeatures();
        console.log("Supported features", supportedFeatures);

        DaemonInterfaceInstance.refreshInformation();

        _steps = DaemonInterfaceInstance.information(DaemonInterface.INFO_STEPS);
    }

    on_ConnectionStateChanged: console.log(_connectionState)

    on_AuthenticatedChanged: _refreshInformation()
    
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
                enabled: !_connecting
                text: _disconnected ? qsTr("Connect to watch") : qsTr("Disconnect from watch")
                onClicked: {
                    if (_disconnected) {
                        DaemonInterfaceInstance.connectToDevice(AmazfishConfig.pairedAddress);
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

            Item {
                height: Theme.itemSizeMedium
                width: parent.width

                Label {
                    id: pairedNameLabel
                    anchors.verticalCenter: parent.verticalCenter
                    width: {
                        var dif = column.width - parent.width;
                        return dif < 0 ? implicitWidth + dif : implicitWidth;
                    }
                    text: AmazfishConfig.pairedName
                    color: Theme.secondaryHighlightColor
                    font.pixelSize: Theme.fontSizeLarge
                    truncationMode: TruncationMode.Fade
                }

                Row {
                    id: statusRow
                    anchors {
                        left: pairedNameLabel.right
                        leftMargin: Theme.paddingMedium
                        verticalCenter: parent.verticalCenter
                    }
                    spacing: Theme.paddingSmall

                    Image {
                        source: "image://theme/icon-m-bluetooth-device"
                        visible: _connected || _authenticated
                    }

                    Image {
                        source: "image://theme/icon-m-watch"
                        visible: _authenticated
                    }

                    Image {
                        id: btryImage
                        source: "image://theme/icon-m-battery"
                        visible: _authenticated
                    }

                    Label {
                        id: btryPercent
                        anchors.verticalCenter: parent.verticalCenter
                        visible: _authenticated
                        font.pixelSize: Theme.fontSizeMedium
                    }

                    BusyIndicator {
                        size: BusyIndicatorSize.Medium
                        visible: _connecting || _connected
                        running: visible
                    }
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
                size: parent.width - Theme.horizontalPageMargin * 4
                percent: _steps ? _steps / AmazfishConfig.profileFitnessGoal : 0.06
                widthRatio: 0.08

                Item {
                    anchors.centerIn: parent
                    height: lblSteps.height + lblGoal.height + Theme.paddingSmall
                    width: Math.max(lblSteps.width, lblGoal.width)

                    Label {
                        id: lblSteps
                        anchors.horizontalCenter: parent.horizontalCenter
                        color: Theme.highlightColor
                        font.pixelSize: Theme.fontSizeExtraLarge
                        verticalAlignment: Text.AlignVCenter
                        text: _steps.toLocaleString()
                    }

                    Label {
                        id: lblGoal
                        anchors {
                            horizontalCenter: parent.horizontalCenter
                            top: lblSteps.bottom
                            topMargin: Theme.paddingSmall
                        }
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeLarge
                        verticalAlignment: Text.AlignVCenter
                        text: AmazfishConfig.profileFitnessGoal.toLocaleString()
                    }
                }
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
            if (!AmazfishConfig.profileName) {
                pageStack.push(Qt.resolvedUrl("Settings-profile.qml"))
            }
        }
    }

    Connections {
        target: DaemonInterfaceInstance
        onInformationChanged: {
            console.log("Information changed", infoKey, infoValue);

            switch (infoKey) {
            case DaemonInterface.INFO_BATTERY:
                btryPercent.text = qsTr("%1%").arg(infoValue)
                break;
            case DaemonInterface.INFO_HEARTRATE:
                lblHeartrate.text = qsTr("%1 bpm").arg(infoValue)
                break;
            case DaemonInterface.INFO_STEPS:
                _steps = infoValue
                break;
            }
        }
    }

    Component.onCompleted: {
        if (AmazfishConfig.profileName) {
            _refreshInformation();
        }
    }
}
