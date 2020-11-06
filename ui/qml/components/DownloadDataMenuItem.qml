import QtQuick 2.0
import "platform"

PageMenuItemPL {
    text: qsTr("Download Data")
    onClicked: DaemonInterfaceInstance.downloadActivityData()
    enabled: DaemonInterfaceInstance.connectionState === "authenticated"
}
