import QtQuick 2.0
import "platform"
import uk.co.piggz.amazfish 1.0

PageMenuItemPL {
    iconSource: styler.iconDownloadData !== undefined ? styler.iconDownloadData : ""
    text: qsTr("Download Data")
    onClicked: DaemonInterfaceInstance.fetchData(Amazfish.TYPE_ACTIVITY)
    enabled: DaemonInterfaceInstance.connectionState === "authenticated"
}
