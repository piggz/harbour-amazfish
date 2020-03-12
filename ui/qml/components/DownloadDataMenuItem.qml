import QtQuick 2.0
import Sailfish.Silica 1.0

MenuItem {
    text: qsTr("Download Data")
    onClicked: DaemonInterfaceInstance.downloadActivityData()
    enabled: DaemonInterfaceInstance.connectionState === "authenticated"
}
