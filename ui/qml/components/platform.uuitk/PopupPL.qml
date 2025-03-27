import Nemo.DBus 2.0

DBusInterface {
    id: notificationsIface
    bus: DBus.SessionBus
    service: 'org.freedesktop.Notifications'
    path: '/org/freedesktop/Notifications'
    iface: 'org.freedesktop.Notifications'

    function showMessage(msg) {

        // https://specifications.freedesktop.org/notification-spec/latest/protocol.html
        // https://wiki.ubuntu.com/Touch/Notifications

        console.log(msg) // notification will not apear in `clickable desktop`

        var app_icon = APPLICATION_FILE_DIR + "/../share/icons/hicolor/scalable/apps/harbour-amazfish-ui.svg"

        notificationsIface.typedCall("Notify", [
            { "type": "s", "value": "harbour-amazfish" },     // app_name
            { "type": "u", "value": _lastNotificationId },    // replaces_id
            { "type": "s", "value": app_icon },               // app_icon
            { "type": "s", "value": msg },                    // summary
            { "type": "s", "value": "" },                     // body
            { "type": "as", "value": [] },                    // actions
            { "type": "a{sv}", "value": {}},                  // hints
            { "type": "i", "value": 5000 }                    // expire_timeout
        ]);

    }
}
