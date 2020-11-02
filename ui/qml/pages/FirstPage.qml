import QtQuick 2.0
import org.SfietKonstantin.weatherfish 1.0
import uk.co.piggz.amazfish 1.0
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: "Amazfish"

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    //allowedOrientations: Orientation.Portrait

    //property var day: new Date()

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

        _steps = parseInt(DaemonInterfaceInstance.information(DaemonInterface.INFO_STEPS));
    }

    on_ConnectionStateChanged: console.log(_connectionState)

    on_AuthenticatedChanged: _refreshInformation()
    
    //onStatusChanged: {
    //    if (status === PageStatus.Active) {
    //        pageStack.pushAttached(Qt.resolvedUrl("StepsPage.qml"))
    //    }
    //}

    pageMenu: PageMenuPL {
        PageMenuItemPL {
            text: qsTr("Pair with watch")
            onClicked: {
                var page = AmazfishConfig.pairedAddress
                        ? "UnpairDeviceDialog.qml"
                        : "PairSelectDeviceType.qml"
                pageStack.push(Qt.resolvedUrl(page))
            }
        }
        PageMenuItemPL {
            text: qsTr("Download File")
            onClicked: pageStack.push(Qt.resolvedUrl("BipFirmwarePage.qml"))
        }
        PageMenuItemPL {
            text: qsTr("Settings")
            onClicked: pageStack.push(Qt.resolvedUrl("Settings-menu.qml"))
        }
        PageMenuItemPL {
            text: qsTr("Data Graphs")
            onClicked: pageStack.push(Qt.resolvedUrl("AnalysisPage.qml"))
        }
        PageMenuItemPL {
            visible: AmazfishConfig.pairedAddress
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

    Column {
        id: column
        width: parent.width
        spacing: styler.themePaddingLarge

        Item {
            height: styler.themeItemSizeSmall
            width: parent.width

            LabelPL {
                id: pairedNameLabel
                anchors.verticalCenter: parent.verticalCenter
                width: {
                    var dif = column.width - parent.width;
                    return dif < 0 ? implicitWidth + dif : implicitWidth;
                }
                text: AmazfishConfig.pairedName
                color: styler.themeSecondaryHighlightColor
                font.pixelSize: styler.themeFontSizeLarge
                truncMode: truncModes.fade
            }

            Row {
                id: statusRow
                anchors {
                    left: pairedNameLabel.right
                    leftMargin: styler.themePaddingMedium
                    verticalCenter: parent.verticalCenter
                }
                spacing: styler.themePaddingSmall
                height: parent.height

                Image {
                    source: "image://theme/icon-m-bluetooth-device"
                    visible: _connected || _authenticated
                    BusyIndicatorSmallPL {
                        visible: _connecting
                        running: visible
                    }
                }

                Image {
                    source: "image://theme/icon-m-watch"
                    visible: _authenticated
                    BusyIndicatorSmallPL {
                        visible: _connected
                        running: visible
                    }
                }

                Image {
                    id: btryImage
                    source: "image://theme/icon-m-battery"
                    visible: _authenticated
                }

                LabelPL {
                    id: btryPercent
                    anchors.verticalCenter: parent.verticalCenter
                    visible: _authenticated
                    font.pixelSize: styler.themeFontSizeMedium
                }
            }
        }

        SectionHeaderPL {
            width: parent.width
        }

        // steps
        Image {
            id: imgSteps
            source: "../pics/icon-m-steps.png"
            height: styler.themeIconSizeMedium
            width: height
        }

        PercentCircle {
            id: stpsCircle

            anchors.horizontalCenter: parent.horizontalCenter
            size: parent.width - styler.themeHorizontalPageMargin * 4
            percent: _steps ? _steps / AmazfishConfig.profileFitnessGoal : 0.06
            widthRatio: 0.08

            Item {
                anchors.centerIn: parent
                height: lblSteps.height + lblGoal.height + styler.paddingSmall
                width: Math.max(lblSteps.width, lblGoal.width)

                LabelPL {
                    id: lblSteps
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: styler.themeHighlightColor
                    font.pixelSize: styler.themeFontSizeExtraLarge
                    verticalAlignment: Text.AlignVCenter
                    text: _steps.toLocaleString()
                }

                LabelPL {
                    id: lblGoal
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        top: lblSteps.bottom
                        topMargin: styler.themePaddingSmall
                    }
                    color: styler.themeSecondaryHighlightColor
                    font.pixelSize: styler.themeFontSizeLarge
                    verticalAlignment: Text.AlignVCenter
                    text: AmazfishConfig.profileFitnessGoal.toLocaleString()
                }
            }
        }

        SectionHeaderPL {
            width: parent.width
        }

        //Heartrate
        Row {
            spacing: styler.themePaddingLarge
            width: parent.width
            visible: supportsFeature(DaemonInterface.FEATURE_HRM)

            Image {
                id: imgHeartrate
                source: "../pics/icon-m-heartrate.png"
                width: styler.themeIconSizeMedium
                height: width
            }
            LabelPL {
                id: lblHeartrate
                color: styler.themePrimaryColor
                font.pixelSize: styler.themeFontSizeLarge
                height: styler.iconSizeMedium
                verticalAlignment: Text.AlignVCenter
                width: parent.width - imgHeartrate.width - btnHR.width - 2* styler.paddingLarge
            }

            IconButtonPL {
                id: btnHR
                iconSource: "image://styler/icon-m-refresh"
                onClicked: {
                    DaemonInterfaceInstance.requestManualHeartrate();
                }
            }
        }

        ButtonPL {
            text: qsTr("Start Service")
            visible: serviceActiveState == false
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                systemdServiceIface.call("Start", ["replace"])
            }
        }

        ButtonPL {
            text: qsTr("Enable Service")
            visible: serviceEnabledState == false
            anchors.horizontalCenter: parent.horizontalCenter

            onClicked: {
                systemdManager.enableService();
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
                    _steps = parseInt(infoValue);
                    break;
                }
            }
        }
    }

    Component.onCompleted: {
        if (AmazfishConfig.profileName) {
            _refreshInformation();
        }
    }
}
