import QtQuick 2.0
import Sailfish.Silica 1.0
import "pages"
import Nemo.Notifications 1.0
import org.nemomobile.mpris 1.0

ApplicationWindow
{
    id: app
    initialPage: Component { FirstPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    Component.onCompleted: {
        console.log("Application started");
    }

    onStateChanged: {
        console.log("State: " + state);

    }

    function showMessage(msg)
    {
        txtMessage.text = msg;
        rectMessage.y = app.height - rectMessage.height - 20;
        tmrHideMessage.start();
    }

    BusyIndicator {
        size: BusyIndicatorSize.Large
        anchors.centerIn: parent
        visible: DeviceInterface.operationRunning
        running: DeviceInterface.operationRunning
    }

    Rectangle {
        id: rectMessage
        width: parent.width - 40
        anchors.horizontalCenter: parent.horizontalCenter
        height: childrenRect.height + 20
        radius: 5
        color: Theme.highlightBackgroundColor
        y: parent.height + 10

        Behavior on y {
                NumberAnimation { duration: 100 }
            }

        Text {
            id: txtMessage
            x: 10
            y: 10
            width: parent.width - 20
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeMedium
        }
        Timer {
            id: tmrHideMessage
            interval: 5000
            running: false
            repeat: false
            onTriggered: {
                rectMessage.y = app.height + 10
            }
        }
    }

    MprisManager {
        id: mprisManager
    }
    
    Connections {
        target: DeviceInterface
        onMessage: {
            showMessage(text);
        }

        onButtonPressed: {
            console.log("Button pressed:", presses);
            
            if (presses == 3 && mprisManager.canGoPrevious) {
                mprisManager.previous();
            }
            
            if (presses == 2 && mprisManager.canGoNext) {
                mprisManager.next();
            }
        }
    }
}
