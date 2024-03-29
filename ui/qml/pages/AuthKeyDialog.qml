import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

DialogPL {
    onAccepted: AmazfishConfig.deviceAuthKey = authKeyField.text
    canAccept: authKeyField.text

    Column {
        width: parent.width
        anchors.top: parent.top
        anchors.margins: styler.themePaddingMedium

        TextFieldPL {
            id: authKeyField
            width: parent.width
            label: qsTr("Enter auth key")
            placeholderText: label
            Component.onCompleted: text = AmazfishConfig.deviceAuthKey
        }
    }
}
