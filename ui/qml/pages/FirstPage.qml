import QtQuick 2.0
import org.SfietKonstantin.weatherfish 1.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: "Amazfish"

    function unpairAccepted() {
        DaemonInterfaceInstance.disconnect();
        DaemonInterfaceInstance.unpair()
        AmazfishConfig.pairedAddress = "";
        AmazfishConfig.pairedName = "";
        AmazfishConfig.pairedType = "";
    }

    pageMenu: PageMenuPL {
        //        PageMenuItemPL {
        //            text: qsTr("Test Icons")
        //            onClicked: app.pages.push(Qt.resolvedUrl("TestIconsPage.qml"))
        //        }
        PageMenuItemPL {
            text: qsTr("Pair with watch")
            onClicked: {
                if (AmazfishConfig.pairedAddress) {
                    var obj = app.pages.push(Qt.resolvedUrl("UnpairDeviceDialog.qml"));
                    obj.acceptDestination = Qt.resolvedUrl("PairDevicePage.qml");
                    obj.accepted.connect(unpairAccepted);
                } else {
                    app.pages.push(Qt.resolvedUrl("PairDevicePage.qml"));
                }
            }
        }

        PageMenuItemPL {
            text: qsTr("Settings")
            onClicked: app.pages.push(Qt.resolvedUrl("Settings-menu.qml"))
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

        PageMenuItemPL {
            id: btnSystemdEnable
            text: qsTr("Enable service on boot")
            visible: serviceEnabledState == false && (ENABLE_SYSTEMD === "YES")

            onClicked: {
                systemdManager.enableService();
            }
        }
    }

    GridLayout {
        id: pageGrid
        width: parent.width

        columns: 3
        columnSpacing: 0
        rowSpacing: 0

        property double colMulti: pageGrid.width / pageGrid.columns

        function prefWidth(item){
            return colMulti * item.Layout.columnSpan
        }

        //========== Busy Notification Row ==========

        Row {
            id: rowUpdateOperation
            Layout.preferredHeight: styler.themeIconSizeMedium
            Layout.fillWidth: true
            Layout.columnSpan: 3
            spacing: styler.themePaddingSmall
            visible: DaemonInterfaceInstance.operationRunning

            LabelPL {
                id: lblLastMessage
                text: _lastMessage
                color: styler.themeSecondaryHighlightColor
                font.pixelSize: styler.themeFontSizeMedium
                truncMode: truncModes.fade
                width: parent.width - styler.themeIconSizeMedium - styler.themeIconSizeLarge - (3 * styler.themePaddingSmall)
            }

            LabelPL {
                id: lblProgress
                x: 10
                anchors.leftMargin: 5
                horizontalAlignment: Text.AlignLeft
                color: styler.themeSecondaryHighlightColor
                font.pixelSize: styler.themeFontSizeMedium
                width: styler.themeIconSizeLarge
                text: _percentText
            }

            BusyIndicatorSmallPL {
                y: 2
                running: DaemonInterfaceInstance.operationRunning
                Layout.alignment: Qt.AlignRight
                anchors.rightMargin: 2
                width: styler.themeIconSizeMedium
            }
        }

        //========== Device Row ==========

        Row {
            Layout.preferredHeight: styler.themeIconSizeMedium
            Layout.fillWidth: true
            Layout.columnSpan: 3
            spacing: styler.themePaddingSmall

            LabelPL {
                id: pairedNameLabel
                text: AmazfishConfig.pairedName
                color: styler.themeSecondaryHighlightColor
                font.pixelSize: styler.themeFontSizeLarge
                truncMode: truncModes.fade
                width: parent.width - (2 * styler.themeIconSizeMedium)
            }

            IconPL {
                iconName: styler.iconBluetooth
                iconHeight: styler.themeIconSizeMedium
                visible: _connected || _authenticated || _connecting
                BusyIndicatorSmallPL {
                    visible: _connecting
                    running: visible
                    anchors.centerIn: parent
                }
            }

            IconPL {
                iconName: styler.iconWatch
                iconHeight: styler.themeIconSizeMedium
                visible: _authenticated || _connected
                BusyIndicatorSmallPL {
                    visible: _connected
                    running: visible
                    anchors.centerIn: parent
                }
            }
        }

        //========== Tiles ==========

        StepsTile {
            visible: supportsFeature(Amazfish.FEATURE_STEPS)
            Layout.rowSpan: 2
            Layout.columnSpan: 2
            size: pageGrid.prefWidth(this)
            stepCount: _InfoSteps
            stepGoal: AmazfishConfig.profileFitnessGoal

            Component.onCompleted: {
                if (_connected) {
                    _InfoSteps = parseInt(DaemonInterfaceInstance.information(Amazfish.INFO_STEPS), 10) || 0;
                }
            }

            onClicked: {
                app.pages.push(Qt.resolvedUrl("StepsPage.qml"))
            }
        }

        Tile {
            text: qsTr("Sleep")
            visible: supportsData(Amazfish.TYPE_SLEEP)
            size: pageGrid.prefWidth(this)

            contentItem: Image {
                id: imgHeartrate
                source: "../page-icons/icon-page-sleep.png"
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
            }

            onClicked: {
                app.pages.push(Qt.resolvedUrl("SleepPage.qml"))
            }
        }

        Tile {
            text: qsTr("Heartrate")
            visible: supportsData(Amazfish.TYPE_HEART_RATE)
            size: pageGrid.prefWidth(this)

            contentItem: Image {
                source: "../page-icons/icon-page-heartrate.png"
                anchors.fill: parent
                anchors.topMargin: styler.themeFontSizeLarge
                fillMode: Image.PreserveAspectFit
                Text {
                    text: qsTr("%1 bpm").arg(_InfoHeartrate)
                    anchors.top: parent.top
                    anchors.topMargin: -styler.themeFontSizeLarge
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: styler.themeFontSizeLarge
                    color: styler.blockBg
                }
            }
            actionItem: IconButtonPL {
                iconName: styler.iconRefresh
                iconHeight: styler.themeIconSizeMedium
                iconWidth: iconHeight

                anchors.fill: parent

                onClicked: {
                    console.log("Request manual HR");
                    DaemonInterfaceInstance.requestManualHeartrate();
                }
            }

            onClicked: {
                app.pages.push(Qt.resolvedUrl("HeartratePage.qml"))
            }
        }

        Tile {
            text: qsTr("Sports")
            visible: supportsFeature(Amazfish.FEATURE_ACTIVITY)
            size: pageGrid.prefWidth(this)

            contentItem: Image {
                source: "../page-icons/icon-page-sport.png"
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
            }

            onClicked: {
                app.pages.push(Qt.resolvedUrl("SportsSummaryPage.qml"))
            }
        }

        PAITile {
            id: paiTile
            visible: supportsData(Amazfish.TYPE_PAI)
            size: pageGrid.prefWidth(this)
            Layout.rowSpan: 2
            Layout.columnSpan: 2

            onClicked: {
                app.pages.push(Qt.resolvedUrl("PaiDataPage.qml"))
            }
        }

        Tile {
            text: qsTr("Spo2")
            visible: supportsData(Amazfish.TYPE_SPO2)
            size: pageGrid.prefWidth(this)

            contentItem: Image {
                source: "../page-icons/icon-page-spo2.png"
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
            }

            onClicked: {
                app.pages.push(Qt.resolvedUrl("Spo2DataPage.qml"))
            }
        }

        Tile {
            text: qsTr("Data")
            size: pageGrid.prefWidth(this)

            contentItem: Image {
                source: "../page-icons/icon-page-data.png"
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
            }

            onClicked: {
                app.pages.push(Qt.resolvedUrl("AnalysisPage.qml"))
            }
        }

        Tile {
            text: qsTr("Battery")
            size: pageGrid.prefWidth(this)

            contentItem: Image {
                source: "../page-icons/icon-page-battery.png"
                anchors.fill: parent
                anchors.topMargin: styler.themeFontSizeLarge
                fillMode: Image.PreserveAspectFit
                Text {
                    text: qsTr("%1%").arg(_InfoBatteryPercent)
                    anchors.top: parent.top
                    anchors.topMargin: -styler.themeFontSizeLarge
                    anchors.horizontalCenter: parent.horizontalCenter
                    font.pixelSize: styler.themeFontSizeLarge
                    color: styler.blockBg
                }
            }

            onClicked: {
                app.pages.push(Qt.resolvedUrl("BatteryPage.qml"))
            }
        }

        Tile {
            text: qsTr("Install File")
            visible: _authenticated && supportsFeature(Amazfish.FEATURE_FILE_INSTALL)
            size: pageGrid.prefWidth(this)

            contentItem: Image {
                source: "../page-icons/icon-page-install.png"
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
            }

            onClicked: {
                app.pages.push(Qt.resolvedUrl("BipFirmwarePage.qml"))
            }
        }
        // }
    }

    Timer {
        id: tmrStartup
        running: false
        repeat: false
        interval: 500
        onTriggered: {
            // console.log("Start timer triggered");
            if (!AmazfishConfig.profileName) {
                app.pages.push(Qt.resolvedUrl("Settings-user.qml"))
            }
        }
    }

    onPageStatusActive: {
        tmrStartup.start();
        updatePAI();
    }

    Component.onCompleted: {
        if (AmazfishConfig.profileName) {
            _refreshInformation();
        }
        start();
    }

    function start() {
        app.rootPage = page;
    }

    function updatePAI() {
        if (!supportsData(Amazfish.TYPE_PAI)) {
            return;
        }
        paiTile.update();
    }
}
