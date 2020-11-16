import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Device Settings")

    Column {
        id: column
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        FormLayoutPL {
            ComboBoxPL {
                id: cboLanguage
                label: qsTr("Language")

                //"zh_CN", "zh_TW", "en_US", "es_ES", "ru_RU", "de_DE", "it_IT", "fr_FR", "tr_TR"
                model: ListModel {
                    ListElement { text: qsTr("en_US") }
                    ListElement { text: qsTr("es_ES") }
                    ListElement { text: qsTr("zh_CN") }
                    ListElement { text: qsTr("zh_TW") }
                    ListElement { text: qsTr("ru_RU") }
                    ListElement { text: qsTr("de_DE") }
                    ListElement { text: qsTr("it_IT") }
                    ListElement { text: qsTr("fr_FR") }
                    ListElement { text: qsTr("tr_TR") }
                }

                Component.onCompleted: {
                    cboLanguage.currentIndex = AmazfishConfig.deviceLanguage;
                }
            }

            ComboBoxPL {
                id: cboDateDisplay
                label: qsTr("Date Display")

                model: ListModel {
                    ListElement { text: qsTr("Time") }
                    ListElement { text: qsTr("Date/Time") }
                }

                Component.onCompleted: {
                    cboDateDisplay.currentIndex = AmazfishConfig.deviceDateFormat;
                }
            }

            ComboBoxPL {
                id: cboTimeFormat
                label: qsTr("Time Format")

                model: ListModel {
                    ListElement { text: qsTr("24hr") }
                    ListElement { text: qsTr("12hr") }
                }

                Component.onCompleted: {
                    cboTimeFormat.currentIndex = AmazfishConfig.deviceTimeFormat;
                }
            }

            ComboBoxPL {
                id: cboDistanceUnit
                label: qsTr("Distance Unit")

                model: ListModel {
                    ListElement { text: qsTr("Metric") }
                    ListElement { text: qsTr("Imperial") }
                }

                Component.onCompleted: {
                    cboDistanceUnit.currentIndex = AmazfishConfig.deviceDistanceUnit;
                }
            }

            TextSwitchPL {
                id: chkDisconnectNotification
                width: parent.width
                text: qsTr("Disconnect Notification")

                Component.onCompleted: {
                    chkDisconnectNotification.checked = AmazfishConfig.deviceDisconnectNotification;
                }
            }

            ButtonPL {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("BIP Display Items")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("Settings-bip-shortcuts.qml"))
                }
            }

            SectionHeaderPL {
            }

            ButtonPL {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Save Settings")
                onClicked: {
                    saveSettings();
                }
            }
        }
        Timer {
            //Allow data to sync
            id: tmrSetDelay
            repeat: false
            interval: 500
            running: false
            onTriggered: {
                DaemonInterfaceInstance.applyDeviceSetting(DaemonInterface.SETTING_DEVICE_LANGUAGE);
                DaemonInterfaceInstance.applyDeviceSetting(DaemonInterface.SETTING_DEVICE_DATE);
                DaemonInterfaceInstance.applyDeviceSetting(DaemonInterface.SETTING_DEVICE_TIME);
                DaemonInterfaceInstance.applyDeviceSetting(DaemonInterface.SETTING_DEVICE_UNIT);
                DaemonInterfaceInstance.applyDeviceSetting(DaemonInterface.SETTING_DISCONNECT_NOTIFICATION);
            }
        }
    }

    function saveSettings() {
        AmazfishConfig.deviceLanguage = cboLanguage.currentIndex;
        AmazfishConfig.deviceDateFormat = cboDateDisplay.currentIndex;
        AmazfishConfig.deviceTimeFormat = cboTimeFormat.currentIndex;
        AmazfishConfig.deviceDistanceUnit = cboDistanceUnit.currentIndex;
        AmazfishConfig.deviceDisconnectNotification = chkDisconnectNotification.checked;
        tmrSetDelay.start();
    }
}
