import QtQuick 2.0
import "../components"
import "../components/platform"

PageListPL {
    id: page

    title: qsTr("Select Device Type")

    delegate: DeviceButton {
        onSelected: {
            if (needsAuth) {
                var authdialog = app.pages.push(Qt.resolvedUrl("./AuthKeyDialog.qml"));
                authdialog.accepted.connect(function() {
                    app.pages.push(Qt.resolvedUrl("./PairPage.qml"), { deviceType : deviceType, aliases: aliases} );
                })
            } else {
                app.pages.push(Qt.resolvedUrl("./PairPage.qml"), { deviceType : deviceType, aliases: aliases} );
            }
        }
    }

    model: ListModel {
        ListElement {
            deviceType: "AsteroidOS"
            icon: "../pics/devices/asteroidos.png"
            aliases: [
                ListElement { name: 'bass' },
                ListElement { name: 'sturgeon' },
                ListElement { name: 'narwhal' },
                ListElement { name: 'sparrow' },
                ListElement { name: 'dory' },
                ListElement { name: 'lenok' },
                ListElement { name: 'catfish' },
                ListElement { name: 'carp' },
                ListElement { name: 'smelt' },
                ListElement { name: 'anthias' },
                ListElement { name: 'pike' },
                ListElement { name: 'sawfish' },
                ListElement { name: 'ray/firefish' },
                ListElement { name: 'beluga' },
                ListElement { name: 'skipjack' },
                ListElement { name: 'koi' },
                ListElement { name: 'mooneye' },
                ListElement { name: 'swift' },
                ListElement { name: 'nemo' },
                ListElement { name: 'hoki' },
                ListElement { name: 'minnow' },
                ListElement { name: 'tetra' },
                ListElement { name: 'sprat' },
                ListElement { name: 'kingyo' },
                ListElement { name: 'medaka' }
            ]
        }
        ListElement {
            deviceType: "Amazfit Bip Watch"
            icon: "../pics/devices/amazfit-bip.png"
            aliases: []
        }

        ListElement {
            deviceType: "Amazfit Bip Lite"
            icon: "../pics/devices/amazfit-bip.png"
            auth: true
            aliases: []
        }

        ListElement {
            deviceType: "Amazfit Bip S"
            icon: "../pics/devices/amazfit-bips.png"
            auth: true
            aliases: []
        }

        ListElement {
            deviceType: "Amazfit GTS"
            icon: "../pics/devices/amazfit-gts.png"
            auth: true
            aliases: []
        }

        ListElement {
            deviceType: "Amazfit GTS 2"
            icon: "../pics/devices/amazfit-gts2.png"
            auth: true
            aliases: []
        }

        ListElement {
            deviceType: "Amazfit GTR"
            icon: "../pics/devices/amazfit-gtr.png"
            auth: true
            aliases: []
        }

        ListElement {
            deviceType: "Amazfit GTR 2"
            icon: "../pics/devices/amazfit-gtr2.png"
            auth: true
            aliases: []
        }

        ListElement {
            deviceType: "Amazfit Cor"
            icon: "../pics/devices/amazfit-cor.png"
            aliases: []
        }

        ListElement {
            deviceType: "MI Band 2"
            icon: "../pics/devices/miband2.png"
            aliases: []
        }

        ListElement {
            deviceType: "Mi Band 3"
            icon: "../pics/devices/miband3.png"
            aliases: []
        }

        ListElement {
            deviceType: "Mi Smart Band 4"
            icon: "../pics/devices/miband4.png"
            auth: true
            aliases: []
        }

        ListElement {
            deviceType: "Amazfit Stratos 3"
            icon: "../pics/devices/miband4.png"
            auth: true
            aliases: []
        }

        ListElement {
            deviceType: "InfiniTime"
            icon: "../pics/devices/pinetime.png"
            auth: false
            aliases: [ ListElement { name: "Pinetime" } ]
        }
        ListElement {
            deviceType: "Bangle.js"
            icon: "../pics/devices/banglejs.png"
            auth: false
            aliases: []
        }
    }

}
