import QtQuick 2.0
import QtQuick.Controls 2.2

Popup {
    id: popup
    width: app.width - 40
    y: parent.height + 10
    z: 999
    height: contentChildren.height + 20
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    Text {
        id: txtMessage
        x: 10
        y: 10
        width: parent.width - 20
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
        color: styler.themeHighlightColor
        font.pixelSize: styler.themeFontSizeMedium
    }
    Timer {
        id: tmrHideMessage
        interval: 5000
        running: false
        repeat: false
        onTriggered: {
            popup.close();
        }
    }

    function showMessage(msg)
    {
        //        notification.replacesId = _lastNotificationId
        //        notification.previewBody = msg
        //        notification.publish()
        //        _lastNotificationId = notification.replacesId
        console.log("PopupPL Message:", msg);
        txtMessage.text = msg;
        popup.open();
        popup.y = app.height - popup.height - 20;
        tmrHideMessage.start();
    }
}
