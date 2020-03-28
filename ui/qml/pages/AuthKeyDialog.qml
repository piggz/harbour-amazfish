import QtQuick 2.0
import Sailfish.Silica 1.0
import uk.co.piggz.amazfish 1.0

Dialog {
    onAccepted: AmazfishConfig.deviceAuthKey = authKeyField.text
    acceptDestinationAction: PageStackAction.Replace
    canAccept: authKeyField.text

    Column {
        width: parent.width

        DialogHeader {}

        TextField {
            id: authKeyField
            width: parent.width
            label: qsTr("Enter auth key")
            placeholderText: label
            Component.onCompleted: text = AmazfishConfig.deviceAuthKey
        }
    }
}
