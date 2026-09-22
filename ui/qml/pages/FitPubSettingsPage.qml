import QtQuick 2.0
import uk.co.piggz.amazfish 1.0
import "../components/"
import "../components/platform"

PagePL {
    id: page
    title: qsTr("FitPub settings")

    pageMenu: PageMenuPL {
        PageMenuItemPL
        {
            id: btnAuth
            iconSource: styler.iconStravaLogin !== undefined ? styler.iconStravaLogin : ""
            text: qsTr("Save")
            onClicked: {
                saveSettings();
            }
        }
    }

    Column
    {
        id: column
        width: page.width
        spacing: styler.themePaddingLarge

        TextFieldPL {
            id: fldBaseURL
            label: qsTr("Instance URL:")
            placeholderText: "https://fitpub.social"
            width: parent.width
        }

        TextFieldPL {
            id: fldUsername
            label: qsTr("Username or Email:")
            placeholderText: "username"
            width: parent.width
        }

        TextFieldPL {
            id: fldPassword
            label: qsTr("Password:")
            placeholderText: "password"
            echoMode: TextInput.Password
            width: parent.width
        }

        LabelPL {
            text: qsTr("Account linked")
            visible: app.fitpubLinked
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
        }
    }

    function saveSettings() {
        AmazfishConfig.fitpubBaseURL = fldBaseURL.text;
        AmazfishConfig.fitpubUsername = fldUsername.text;
        AmazfishConfig.fitpubPassword = fldPassword.text;
    }

    Component.onCompleted: {
        fldBaseURL.text = AmazfishConfig.fitpubBaseURL;
        fldUsername.text = AmazfishConfig.fitpubUsername;
        fldPassword.text = AmazfishConfig.fitpubPassword;
    }
}
