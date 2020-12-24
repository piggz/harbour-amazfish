import QtQuick 2.0
import Nemo.Notifications 1.0

Notification {
    id: notification
    expireTimeout: 5000

    function showMessage(msg)
    {
        notification.replacesId = _lastNotificationId
        notification.previewBody = msg
        notification.publish()
        _lastNotificationId = notification.replacesId
    }
}
