import QtQuick 2.0
import "../components"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("Download File")

    property string selectedFile: qsTr("None")
    property string fileVersion
    property bool selectionMade: false

    Column {
        id: column
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: styler.themePaddingMedium
        spacing: styler.themePaddingLarge

        LabelPL {
            width: parent.width
            text: qsTr("Select a file to download.");
        }

        ValueButtonPL {
            label: qsTr("Choose File")
            value: selectedFile ? selectedFile : qsTr("None")
            onClicked: pageStack.push(filePickerPage)
            enabled: DaemonInterfaceInstance.connectionState === "authenticated"
        }

        LabelPL {
            id: lblVersion
            width: parent.width
            text: qsTr("File type/version: ") + fileVersion;
        }

        ButtonPL {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Send file")
            enabled: selectionMade && DaemonInterfaceInstance.connectionState === "authenticated"
            onClicked: {
                if (!DaemonInterfaceInstance.startDownload()) {
                    fileVersion = qsTr("File not supported on this device");
                }
            }
        }

        //ProgressBar {
        //    id: progress
        //    width: parent.width
        //    minimumValue: 0
        //    maximumValue: 100
        //    value: 0
        //}

        LabelPL {
            id: lblPercent
            width: parent.width
            text: "0%"
            horizontalAlignment: Text.AlignHCenter
        }
    }


    Component {
        id: filePickerPage
        FileSelectorPL {
            nameFilters: [ '*.*' ]
            onSelected: {
                page.selectedFile = selectedContentProperties.selectedFilePath;
                page.selectionMade = true;
                fileVersion = DaemonInterfaceInstance.prepareFirmwareDownload(page.selectedFile);
            }
        }
    }

    Connections {
        target: DaemonInterfaceInstance
        onDownloadProgress: {
            progress.value = percent;
            lblPercent.text = percent + "%";
        }
    }
}
