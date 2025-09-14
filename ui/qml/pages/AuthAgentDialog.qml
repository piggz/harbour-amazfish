import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components"
import "../components/platform"

DialogPL {
//    onAccepted: AmazfishConfig.deviceAuthKey = authKeyField.text
    canAccept: (authKeyField.text || authType == "confirmation")

    property string authType
    property string deviceName
    property string devicePath
    property int passkey
    property alias enteredPasskey: authKeyField.text

    function authTypeToLabel(t) {
	switch(t) {
	case "passkey":
	    return qsTr("Enter authentication passkey")
	case "confirmation":
	    return qsTr("Confirm authentication key")
	case "pincode":
	    return qsTr("Enter authentication pin code")
	default:
	    return qsTr("Authentication agent")
	}
    }

    Column {
	width: parent.width
	anchors.top: parent.top
	anchors.margins: styler.themePaddingMedium


	LabelPL {
	    text: authTypeToLabel(authType);
	}

	LabelPL {
	    text: qsTr("Device name: %1").arg(deviceName)
	}

	LabelPL {
	    text: qsTr("Device path: %1)").arg(devicePath)
	}

	LabelPL {
	    text: qsTr("Passkey: %1").arg(passkey)
	    visible: authType == "confirmation"
	}

	TextFieldPL {
	    id: authKeyField
	    width: parent.width
	    label: qsTr("Passkey: ")
	    visible: authType == "passkey"
	}
    }
}
