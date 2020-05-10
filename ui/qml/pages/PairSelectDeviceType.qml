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
                deviceType: "Amazfit Bip Watch"
                icon: "../pics/devices/amazfit-bip.png"
            }

            ListElement {
                deviceType: "Amazfit Bip Lite"
                icon: "../pics/devices/amazfit-bip.png"
                auth: true
            }

            ListElement {
                deviceType: "Amazfit GTS"
                icon: "../pics/devices/amazfit-gts.png"
                auth: true
            }

            ListElement {
                deviceType: "Amazfit Cor"
                icon: "../pics/devices/amazfit-cor.png"
            }

            ListElement {
                deviceType: "MI Band 2"
                icon: "../pics/devices/miband2.png"
            }

            ListElement {
                deviceType: "Mi Band 3"
                icon: "../pics/devices/miband3.png"
            }

            ListElement {
                deviceType: "Mi Smart Band 4"
                icon: "../pics/devices/miband4.png"
                auth: true
            }
            ListElement {
                deviceType: "Pinetime-JF"
                icon: "../pics/devices/pinetime.png"
                auth: false
            }
        }
    }
}
