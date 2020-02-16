import QtQuick 2.0
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import "../components"

Page {
    id: page

    // The effective value will be restricted by ApplicationWindow.allowedOrientations
    allowedOrientations: Orientation.Portrait

    // To enable PullDownMenu, place our content in a SilicaFlickable
    SilicaFlickable {
        anchors.fill: parent

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
                title: qsTr("Select Device Type")
            }

            DeviceButton {
                txt: qsTr("Amazfit Bip Watch")
                icn: "../pics/devices/amazfit-bip.png"

                onClicked: {
                    pushPairPage("Amazfit Bip Watch", false);
                }
            }

            DeviceButton {
                txt: qsTr("Amazfit Bip Lite")
                icn: "../pics/devices/amazfit-bip.png"

                onClicked: {
                    pushPairPage("Amazfit Bip Lite", true);
                }
            }

            DeviceButton {
                txt: qsTr("Amazfit GTS")
                icn: "../pics/devices/amazfit-gts.png"

                onClicked: {
                    pushPairPage("Amazfit GTS", true);
                }
            }

            DeviceButton {
                txt: qsTr("Amazfit Cor")
                icn: "../pics/devices/amazfit-cor.png"

                onClicked: {
                    pushPairPage("Amazfit Cor", false);
                }
            }

            DeviceButton {
                txt: qsTr("Mi Band 2")
                icn: "../pics/devices/miband2.png"

                onClicked: {
                    pushPairPage("Mi Band 2", false);
                }
            }

            DeviceButton {
                txt: qsTr("Mi Band 3")
                icn: "../pics/devices/miband3.png"

                onClicked: {
                    pushPairPage("Mi Band 3", false);
                }
            }
        }
    }

    function pushPairPage(deviceType, auth) {
        var pairPage = pageStack.push(Qt.resolvedUrl("PairPage.qml"));
        pairPage.deviceType = deviceType;
        pairPage.deviceRequiresAuthKey = auth;
    }
}
