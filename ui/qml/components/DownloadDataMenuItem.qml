import QtQuick 2.0
import "platform"

PageMenuItemPL {
    iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
    text: qsTr("Download Data")
    onClicked: DaemonInterfaceInstance.downloadActivityData()
    enabled: DaemonInterfaceInstance.connectionState === "authenticated"
}
