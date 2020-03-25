import QtQuick 2.0
import Sailfish.Silica 1.0
import uk.co.piggz.amazfish 1.0

Dialog {
    acceptDestination: Qt.resolvedUrl("PairSelectDeviceType.qml")
    acceptDestinationAction: PageStackAction.Replace

    onAccepted: {
        DaemonInterfaceInstance.disconnect()
        AmazfishConfig.pairedAddress = ""
        AmazfishConfig.pairedName = ""
        AmazfishConfig.deviceAuthKey = ""
    }

    Column {
        width: parent.width

        DialogHeader {}

        Label {
            anchors {
                left: parent.left
                right: parent.right
                margins: Theme.horizontalPageMargin
            }
            height: implicitHeight + Theme.paddingLarge
            text: qsTr("Unpair")
            color: Theme.highlightColor
            font.family: Theme.fontFamilyHeading
            font.pixelSize: Theme.fontSizeExtraLarge
            wrapMode: Text.WordWrap
        }

        Label {
            anchors {
                left: parent.left
                right: parent.right
                margins: Theme.horizontalPageMargin
            }
            text: qsTr("Before proceeding you need to unpair your current device.")
            color: Theme.highlightColor
            wrapMode: Text.WordWrap
        }
    }
}
