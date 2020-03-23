import QtQuick 2.0
import Sailfish.Silica 1.0
import uk.co.piggz.amazfish 1.0

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

        // PullDownMenu and PushUpMenu must be declared in SilicaFlickable, SilicaListView or SilicaGridView


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
                title: qsTr("Device Settings")
            }

            ComboBox {
                id: cboLanguage
                width: parent.width
                label: qsTr("Language")

                //"zh_CN", "zh_TW", "en_US", "es_ES", "ru_RU", "de_DE", "it_IT", "fr_FR", "tr_TR"

                menu: ContextMenu {
                    MenuItem { text: qsTr("en_US") }
                    MenuItem { text: qsTr("es_ES") }
                    MenuItem { text: qsTr("zh_CN") }
                    MenuItem { text: qsTr("zh_TW") }
                    MenuItem { text: qsTr("ru_RU") }
                    MenuItem { text: qsTr("de_DE") }
                    MenuItem { text: qsTr("it_IT") }
                    MenuItem { text: qsTr("fr_FR") }
                    MenuItem { text: qsTr("tr_TR") }
                }
            }

            ComboBox {
                id: cboDateDisplay
                width: parent.width
                label: qsTr("Date Display")

                menu: ContextMenu {
                    MenuItem { text: qsTr("Time") }
                    MenuItem { text: qsTr("Date/Time") }
                }
            }

            ComboBox {
                id: cboTimeFormat
                width: parent.width
                label: qsTr("Time Format")

                menu: ContextMenu {
                    MenuItem { text: qsTr("24hr") }
                    MenuItem { text: qsTr("12hr") }
                }
            }

            ComboBox {
                id: cboDistanceUnit
                width: parent.width
                label: qsTr("Distance Unit")

                menu: ContextMenu {
                    MenuItem { text: qsTr("Metric") }
                    MenuItem { text: qsTr("Imperial") }
                }
            }

            TextSwitch {
                id: chkDisconnectNotification
                width: parent.width
                text: qsTr("Disconnect Notification")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("BIP Display Items")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("Settings-bip-shortcuts.qml"))
                }
            }

            Separator {
                width: parent.width
                horizontalAlignment: Qt.AlignHCenter
                color: Theme.highlightColor
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Save Settings")
                onClicked: {
                    saveSettings();
                }
            }
        }
    }
    Component.onCompleted: {
        cboLanguage.currentIndex = AmazfishConfig.deviceLanguage;
        cboDateDisplay.currentIndex = AmazfishConfig.deviceDateFormat;
        cboTimeFormat.currentIndex = AmazfishConfig.deviceTimeFormat;
        chkDisconnectNotification.checked = AmazfishConfig.deviceDisconnectNotification;
    }
    function saveSettings() {
        AmazfishConfig.deviceLanguage = cboLanguage.currentIndex;
        AmazfishConfig.deviceDateFormat = cboDateDisplay.currentIndex;
        AmazfishConfig.deviceTimeFormat = cboTimeFormat.currentIndex;
        AmazfishConfig.deviceDisconnectNotification = chkDisconnectNotification.checked;
        tmrSetDelay.start();
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
