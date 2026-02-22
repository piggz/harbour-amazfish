import Lomiri.Components 1.3
import QtQuick 2.4

ListItem {
    property var menu: null
    property int contentHeight

    height: visible ? implicitHeight : 0
    leadingActions: menu ? menu : null
}