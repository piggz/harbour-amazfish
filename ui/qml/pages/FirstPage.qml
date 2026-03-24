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
                text: _percentText
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
            Layout.preferredWidth: parent.width

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
        }

        GridLayout {
            id: pageGrid
            columns: 3
            Layout.preferredWidth: parent.width

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
                visible: supportsFeature(Amazfish.FEATURE_STEPS)
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
                            font.pixelSize: styler.themeFontSizeHuge
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
                            color: styler.blockBg
                            font.pixelSize: styler.themeFontSizeExtraLarge
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
                visible: supportsData(Amazfish.TYPE_SLEEP)
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
                visible: supportsData(Amazfish.TYPE_HEART_RATE)
                Layout.preferredWidth: pageGrid.prefWidth(this)
                Layout.preferredHeight: pageGrid.prefWidth(this)

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
                visible: supportsData(Amazfish.TYPE_PAI)
                Layout.preferredWidth: pageGrid.prefWidth(this)
                Layout.preferredHeight: pageGrid.prefWidth(this)
                Layout.rowSpan: 2
                Layout.columnSpan: 2

                onClicked: {
                    app.pages.push(Qt.resolvedUrl("PaiDataPage.qml"))
                }

                contentItem: PercentCircle {
                    id: paiCircle
                    anchors.horizontalCenter: parent.horizontalCenter
                    size: parent.width - styler.themeHorizontalPageMargin * 4
                    widthRatio: 0.08

                    Item {
                        anchors.centerIn: parent
                        height: lblPAITotal.height + lblPAIToday.height + styler.paddingSmall
                        width: Math.max(lblPAITotal.width, lblPAIToday.width)

                        LabelPL {
                            id: lblPAITotal
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                bottom: paiCenterItem.top
                            }
                            color: styler.themeHighlightColor
                            font.pixelSize: styler.themeFontSizeHuge
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item {
                            id: paiCenterItem
                            width: 1
                            height: 1
                            anchors.centerIn: parent
                        }

                        LabelPL {
                            id: lblPAIToday
                            anchors {
                                horizontalCenter: parent.horizontalCenter
                                top: paiCenterItem.bottom
                                topMargin: styler.themePaddingSmall
                            }
                            color: styler.blockBg
                            font.pixelSize: styler.themeFontSizeExtraLarge
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                    Component.onCompleted: {
                        updatePAI();
                    }
                }
            }

            Tile {
                text: qsTr("Spo2")
                visible: supportsData(Amazfish.TYPE_SPO2)
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
        console.log("Refreshing PAI");
        PaiModel.update();

        var maybeToday = PaiModel.get(PaiModel.rowCount() - 1);
        const pai_total = maybeToday.pai_total.toFixed(1);
        lblPAITotal.text = pai_total
        paiCircle.percent = pai_total / 200 //200 Is a pretty high target, usually > 100 is good

        if (pai_total < 50 ) {
            paiCircle.gradientColor = "orange"
        } else if (pai_total < 100 ) {
            paiCircle.gradientColor = "lightblue"
        } else {
            paiCircle.gradientColor = "lightgreen"
        }

        var now = new Date();
        now.setHours(0,0,0,0);

        if (maybeToday.pai_day.getTime() === now.getTime()) {
            lblPAIToday.text = PaiModel.get(PaiModel.rowCount() - 1).pai_total_today.toFixed(1)
        } else {
            lblPAIToday.text = 0.0
        }
    }
}
