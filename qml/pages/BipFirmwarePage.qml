import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import Sailfish.Pickers 1.0

Page {
    id: page

    property string selectedFile: "None"

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
                title: qsTr("Download File")
            }

            Label {
                width: parent.width
                text: qsTr("Select a file to download.");
            }

            Label {
                width: parent.width
                text: qsTr("Supported files include:");
            }

            Label {
                width: parent.width
                text: qsTr("Firmware");
            }

            Label {
                width: parent.width
                text: qsTr("Watchface");
            }

            Label {
                width: parent.width
                text: qsTr("GPS");
            }

            Label {
                width: parent.width
                text: qsTr("and more");
            }

            ValueButton {
                label: "Choose File"
                value: selectedFile ? selectedFile : "None"
                onClicked: pageStack.push(filePickerPage)
                enabled: DeviceInterface.connectionState === "authenticated"

            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Send file")
                enabled: selectedFile !== "None" && DeviceInterface.connectionState === "authenticated"
                onClicked: {
                    DeviceInterface.firmwareService().downloadFile(page.selectedFile);
                }
            }

            ProgressBar {
                id: progress
                width: parent.width
                minimumValue: 0
                maximumValue: 100
                value: 0
            }

        }
    }

    Component {
        id: filePickerPage
        FilePickerPage {
            nameFilters: [ '*.*' ]
            onSelectedContentPropertiesChanged: {
                page.selectedFile = selectedContentProperties.filePath;
            }
        }
    }


    Connections {
        target: DeviceInterface
        onDownloadProgress: {
            progress.value = percent;
        }
    }
}
