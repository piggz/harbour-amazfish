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

    ColumnLayout {
        id: column
        spacing: styler.themePaddingLarge
        width: page.width
        anchors.margins: styler.themePaddingSmall

        RowLayout {
            id: rowUpdateOperation
            height: styler.themeItemSizeSmall
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width
            visible: DaemonInterfaceInstance.operationRunning
            LabelPL {
                id: lblLastMessage
                text: _lastMessage
                color: styler.themeSecondaryHighlightColor
                font.pixelSize: styler.themeFontSizeMedium
                truncMode: truncModes.fade
                Layout.preferredWidth: parent.width * 0.7
            }
            LabelPL {
                id: lblProgress
                x: 10
                anchors.leftMargin: 5
                horizontalAlignment: Text.AlignLeft
                color: styler.themeSecondaryHighlightColor
                font.pixelSize: styler.themeFontSizeMedium
                text: _percentText + "100%"
                Layout.preferredWidth: parent.height * 2
            }

            BusyIndicatorSmallPL {
                y: 2
                running: DaemonInterfaceInstance.operationRunning
                Layout.alignment: Qt.AlignRight
                anchors.rightMargin: 2
                height: parent.height - 4
                Layout.preferredWidth: parent.height
            }
        }

        RowLayout {
            height: styler.themeItemSizeSmall
            Layout.fillWidth: true

            LabelPL {
                id: pairedNameLabel
                text: AmazfishConfig.pairedName
                color: styler.themeSecondaryHighlightColor
                font.pixelSize: styler.themeFontSizeLarge
                truncMode: truncModes.fade
                Layout.fillWidth: true
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

            IconPL {
                id: btryImage
                iconName: styler.iconBattery
                iconHeight: styler.themeIconSizeMedium
                visible: _authenticated
            }

            LabelPL {
                id: btryPercent
                visible: _authenticated
                font.pixelSize: styler.themeFontSizeMedium
                width: styler.themeIconSizeMedium
                text: qsTr("%1%").arg(_InfoBatteryPercent)
            }
        }

        GridLayout {
            id: pageGrid
            columns: 3
            Layout.fillWidth: true
            width: parent.width

            columnSpacing: 0
            rowSpacing: 0

            property double colMulti: pageGrid.width / pageGrid.columns
            property double rowMulti: pageGrid.height / pageGrid.rows
            function prefWidth(item){
                return colMulti * item.Layout.columnSpan
            }
            function prefHeight(item){
                return rowMulti * item.Layout.rowSpan
            }

            Tile {
                text: qsTr("Steps")
                Layout.rowSpan: 2
                Layout.columnSpan: 2
                Layout.preferredWidth: pageGrid.prefWidth(this)
                Layout.preferredHeight: pageGrid.prefWidth(this)

                contentItem: PercentCircle {
                    id: stpsCircle
                    anchors.horizontalCenter: parent.horizontalCenter
                    size: parent.width - styler.themeHorizontalPageMargin * 4
                    percent: _InfoSteps ? _InfoSteps / AmazfishConfig.profileFitnessGoal : 0.06
                    widthRatio: 0.08

                    Item {
                        anchors.centerIn: parent
                        height: lblSteps.height + lblGoal.height + styler.paddingSmall
                        width: Math.max(lblSteps.width, lblGoal.width)

                        LabelPL {
                            id: lblSteps
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                bottom: centerItem.top
                            }
                            color: styler.themeHighlightColor
                            font.pixelSize: styler.themeFontSizeExtraLarge
                            verticalAlignment: Text.AlignVCenter
                            text: _InfoSteps.toLocaleString()
                        }

                        Item {
                            id: centerItem
                            width: 1
                            height: 1
                            anchors.centerIn: parent
                        }

                        LabelPL {
                            id: lblGoal
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                top: centerItem.bottom
                                topMargin: styler.themePaddingSmall
                            }
                            color: styler.themeSecondaryHighlightColor
                            font.pixelSize: styler.themeFontSizeLarge
                            verticalAlignment: Text.AlignVCenter
                            text: AmazfishConfig.profileFitnessGoal.toLocaleString()
                        }
                    }
                    Component.onCompleted: {
                        if (_connected) {
                            _InfoSteps = parseInt(DaemonInterfaceInstance.information(Amazfish.INFO_STEPS), 10) || 0;
                        }
                    }
                }

                onClicked: {
                    app.pages.push(Qt.resolvedUrl("StepsPage.qml"))
                }
            }

            Tile {
                text: qsTr("Sleep")
                Layout.preferredWidth: pageGrid.prefWidth(this)
                Layout.preferredHeight: pageGrid.prefWidth(this)

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
                Layout.preferredWidth: pageGrid.prefWidth(this)
                Layout.preferredHeight: pageGrid.prefWidth(this)

                contentItem: Image {
                    source: "../page-icons/icon-page-heartrate.png"
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                }
                onClicked: {
                    app.pages.push(Qt.resolvedUrl("HeartratePage.qml"))
                }
            }

            Tile {
                text: qsTr("Sports")
                Layout.preferredWidth: pageGrid.prefWidth(this)
                Layout.preferredHeight: pageGrid.prefWidth(this)

                contentItem: Image {
                    source: "../page-icons/icon-page-sport.png"
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    app.pages.push(Qt.resolvedUrl("SportsSummaryPage.qml"))
                }
            }

            Tile {
                text: qsTr("PAI")
                Layout.preferredWidth: pageGrid.prefWidth(this)
                Layout.preferredHeight: pageGrid.prefWidth(this)
                Layout.rowSpan: 2
                Layout.columnSpan: 2

                onClicked: {
                    app.pages.push(Qt.resolvedUrl("PaiDataPage.qml"))
                }
            }

            Tile {
                text: qsTr("Spo2")
                Layout.preferredWidth: pageGrid.prefWidth(this)
                Layout.preferredHeight: pageGrid.prefWidth(this)

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
                Layout.preferredWidth: pageGrid.prefWidth(this)
                Layout.preferredHeight: pageGrid.prefWidth(this)

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
                Layout.preferredWidth: pageGrid.prefWidth(this)
                Layout.preferredHeight: pageGrid.prefWidth(this)

                contentItem: Image {
                    source: "../page-icons/icon-page-battery.png"
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    app.pages.push(Qt.resolvedUrl("BatteryPage.qml"))
                }
            }

            Tile {
                text: qsTr("Install File")
                Layout.preferredWidth: pageGrid.prefWidth(this)
                Layout.preferredHeight: pageGrid.prefWidth(this)

                contentItem: Image {
                    source: "../page-icons/icon-page-install.png"
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                }

                onClicked: {
                    app.pages.push(Qt.resolvedUrl("BipFirmwarePage.qml"))
                }
            }
        }

        /*


        //Heartrate
        RowLayout {
            anchors.left: parent.left
            anchors.leftMargin: styler.themePaddingLarge
            anchors.right: parent.right
            anchors.rightMargin: styler.themePaddingLarge
            spacing: styler.themePaddingLarge
            width: parent.width
            visible: supportsFeature(Amazfish.FEATURE_HRM)

            IconPL {
                id: imgHeartrate
                iconName: styler.iconHeartrate
                width: styler.themeIconSizeMedium
                height: width
            }
            LabelPL {
                id: lblHeartrate
                color: styler.themePrimaryColor
                font.pixelSize: styler.themeFontSizeLarge
                height: styler.iconSizeMedium
                verticalAlignment: Text.AlignVCenter
                text: qsTr("%1 bpm").arg(_InfoHeartrate)
            }

            Item {
                Layout.fillWidth: true
            }

            IconButtonPL {
                id: btnHR
                iconName: styler.iconRefresh
                iconHeight: styler.themeIconSizeMedium
                iconWidth: iconHeight
                onClicked: {
                    DaemonInterfaceInstance.requestManualHeartrate();
                }
            }
        }

        */

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
    }

    onPageStatusActive: {
        tmrStartup.start();
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
}
