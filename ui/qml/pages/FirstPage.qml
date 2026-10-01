import QtQuick 2.0
import org.SfietKonstantin.weatherfish 1.0
import uk.co.piggz.amazfish 1.0
import QtQuick.Layouts 1.1
import "../components/"
import "../components/platform"
import "../components/GlassStyle.js" as Glass
import "../components/ChartColors.js" as ChartColors

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
                    DaemonInterfaceInstance.connectToDevice();
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
        // tiles bring their own gap (themePaddingSmall), so the outer margin adds up
        // to the page margin used everywhere else
        x: styler.themeHorizontalPageMargin - styler.themePaddingSmall
        width: parent.width - 2 * x

        columns: 3
        columnSpacing: 0
        rowSpacing: 0

        property double colMulti: pageGrid.width / pageGrid.columns

        function prefWidth(item){
            return colMulti * item.Layout.columnSpan
        }

        //========== Busy Notification Row ==========

        Item {
            id: rowUpdateOperation
            Layout.preferredHeight: styler.themeItemSizeSmall
            Layout.fillWidth: true
            Layout.columnSpan: 3
            visible: DaemonInterfaceInstance.operationRunning

            GlassPanel {
                round: true
                anchors.fill: parent
                anchors.margins: styler.themePaddingSmall

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: styler.themePaddingLarge
                    anchors.rightMargin: styler.themePaddingMedium
                    spacing: styler.themePaddingMedium

                    LabelPL {
                        id: lblLastMessage
                        anchors.verticalCenter: parent.verticalCenter
                        text: _lastMessage
                        color: styler.themeSecondaryColor
                        font.pixelSize: styler.themeFontSizeSmall
                        truncMode: truncModes.fade
                        width: parent.width - lblProgress.width - busyIndicator.width - 2 * parent.spacing
                    }

                    LabelPL {
                        id: lblProgress
                        anchors.verticalCenter: parent.verticalCenter
                        color: styler.themePrimaryColor
                        font.pixelSize: styler.themeFontSizeSmall
                        font.bold: true
                        text: _percentText
                    }

                    BusyIndicatorSmallPL {
                        id: busyIndicator
                        anchors.verticalCenter: parent.verticalCenter
                        running: DaemonInterfaceInstance.operationRunning
                    }
                }
            }
        }

        //========== Device Row ==========

        Item {
            Layout.preferredHeight: deviceColumn.height + 2 * styler.themePaddingLarge
            Layout.fillWidth: true
            Layout.columnSpan: 3

            GlassPanel {
                anchors.fill: parent
                anchors.margins: styler.themePaddingSmall

                Column {
                    id: deviceColumn
                    anchors.left: parent.left
                    anchors.right: deviceIcons.left
                    anchors.leftMargin: styler.themePaddingLarge
                    anchors.rightMargin: styler.themePaddingMedium
                    anchors.verticalCenter: parent.verticalCenter

                    LabelPL {
                        id: pairedNameLabel
                        width: parent.width
                        text: AmazfishConfig.pairedName
                        color: styler.themePrimaryColor
                        font.pixelSize: styler.themeFontSizeLarge
                        truncMode: truncModes.fade
                    }
                    LabelPL {
                        width: parent.width
                        text: _authenticated ? qsTr("Connected")
                                             : (_connecting ? qsTr("Connecting…") : qsTr("Not connected"))
                        color: styler.themeSecondaryColor
                        font.pixelSize: styler.themeFontSizeSmall
                        truncMode: truncModes.fade
                    }
                }

                Row {
                    id: deviceIcons
                    anchors.right: parent.right
                    anchors.rightMargin: styler.themePaddingMedium
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: styler.themePaddingSmall

                    Rectangle {
                        width: styler.themeIconSizeMedium
                        height: width
                        radius: width / 2
                        color: Glass.chip
                        visible: _connected || _authenticated || _connecting

                        IconPL {
                            anchors.centerIn: parent
                            iconName: styler.iconBluetooth
                            iconHeight: styler.themeIconSizeSmall
                        }
                        BusyIndicatorSmallPL {
                            visible: _connecting
                            running: visible
                            anchors.centerIn: parent
                        }
                    }

                    Rectangle {
                        width: styler.themeIconSizeMedium
                        height: width
                        radius: width / 2
                        color: Glass.chip
                        visible: _authenticated || _connected

                        IconPL {
                            anchors.centerIn: parent
                            iconName: styler.iconWatch
                            iconHeight: styler.themeIconSizeSmall
                        }
                        BusyIndicatorSmallPL {
                            visible: _connected && !_authenticated
                            running: visible
                            anchors.centerIn: parent
                        }
                    }
                }
            }
        }

        //========== Tiles ==========

        StepsTile {
            visible: supportsFeatureRefresh(Amazfish.FEATURE_STEPS)
            Layout.rowSpan: 2
            Layout.columnSpan: 2
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
            visible: supportsDataRefresh(Amazfish.TYPE_SLEEP)
            iconSource: "../page-icons/icon-page-sleep.png"

            onClicked: {
                app.pages.push(Qt.resolvedUrl("SleepPage.qml"))
            }
        }

        Tile {
            text: qsTr("Heartrate")
            visible: supportsDataRefresh(Amazfish.TYPE_HEART_RATE)
            iconSource: "../page-icons/icon-page-heartrate.png"
            value: qsTr("%1 bpm").arg(_InfoHeartrate)
            iconColor: ChartColors.heartrate
            actionItem: IconButtonPL {
                iconName: styler.iconRefresh
                iconHeight: styler.themeIconSizeSmall
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
            visible: supportsFeatureRefresh(Amazfish.FEATURE_ACTIVITY)
            iconSource: "../page-icons/icon-page-sport.png"

            onClicked: {
                app.pages.push(Qt.resolvedUrl("SportsSummaryPage.qml"))
            }
        }

        PAITile {
            id: paiTile
            visible: supportsDataRefresh(Amazfish.TYPE_PAI)
            Layout.rowSpan: 2
            Layout.columnSpan: 2

            onClicked: {
                app.pages.push(Qt.resolvedUrl("PaiDataPage.qml"))
            }
        }

        Tile {
            text: qsTr("SpO₂")
            visible: supportsDataRefresh(Amazfish.TYPE_SPO2)
            iconSource: "../page-icons/icon-page-spo2.png"

            onClicked: {
                app.pages.push(Qt.resolvedUrl("Spo2DataPage.qml"))
            }
        }

        Tile {
            text: qsTr("Stress")
            visible: supportsDataRefresh(Amazfish.TYPE_STRESS)
            iconSource: "../page-icons/icon-page-stress.png"

            onClicked: {
                app.pages.push(Qt.resolvedUrl("StressDataPage.qml"))
            }
        }

        Tile {
            text: qsTr("Data")
            iconSource: "../page-icons/icon-page-data.png"

            onClicked: {
                app.pages.push(Qt.resolvedUrl("AnalysisPage.qml"))
            }
        }

        Tile {
            text: qsTr("Battery")
            iconSource: "../page-icons/icon-page-battery.png"
            value: qsTr("%1%").arg(_InfoBatteryPercent)

            onClicked: {
                app.pages.push(Qt.resolvedUrl("BatteryPage.qml"))
            }
        }

        Tile {
            text: qsTr("Install File")
            visible: _authenticated && supportsFeatureRefresh(Amazfish.FEATURE_FILE_INSTALL)
            iconSource: "../page-icons/icon-page-install.png"

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
        if (!supportsDataRefresh(Amazfish.TYPE_PAI)) {
            return;
        }
        paiTile.update();
    }
}
