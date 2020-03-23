import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    SilicaListView {
        anchors.fill: parent

        header: PageHeader {
            title: qsTr("Select Device Type")
        }

        delegate: DeviceButton {}

        model: ListModel {
            ListElement {
                label: qsTr("Amazfit Bip Watch")
                icon: "../pics/devices/amazfit-bip.png"
                deviceType: "Amazfit Bip Watch"
            }

            ListElement {
                label: qsTr("Amazfit Bip Lite")
                icon: "../pics/devices/amazfit-bip.png"
                deviceType: "Amazfit Bip Lite"
                auth: true
            }

            ListElement {
                label: qsTr("Amazfit GTS")
                icon: "../pics/devices/amazfit-gts.png"
                deviceType: "Amazfit GTS"
                auth: true
            }

            ListElement {
                label: qsTr("Amazfit Cor")
                icon: "../pics/devices/amazfit-cor.png"
                deviceType: "Amazfit Cor"
            }

            ListElement {
                label: qsTr("MI Band 2")
                icon: "../pics/devices/miband2.png"
                deviceType: "MI Band 2"
            }

            ListElement {
                label: qsTr("Mi Band 3")
                icon: "../pics/devices/miband3.png"
                deviceType: "Mi Band 3"
            }

            ListElement {
                label: qsTr("Mi Smart Band 4")
                icon: "../pics/devices/miband4.png"
                deviceType: "Mi Smart Band 4"
                auth: true
            }
        }
    }
}
