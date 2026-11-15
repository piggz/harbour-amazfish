import QtQuick
import QtTest

// Compiles every shared and platform component from the app resources.
// A missing module or a removed type (for example QtGraphicalEffects in Qt 6) fails here.
TestCase {
    name: "ComponentsLoad"

    // Known failures. When one is fixed, the test reports an unexpected pass; remove it from the list then.
    readonly property var knownFailures: ({
        "Alarm.qml": "needs uk.co.piggz.amazfish, registered in C++ by the app",
        "DownloadDataMenuItem.qml": "needs uk.co.piggz.amazfish, registered in C++ by the app",
        "platform/ClipboardPL.qml": "needs Clipboard, registered in C++ by the app",
        "platform/ExpandingSectionGroupPL.qml": "Spacer is not defined; component not used yet",
        "platform/MenuDrawerPL.qml": "bannerImageSource alias fails with Kirigami 6; component not used yet"
    })

    function test_load_data() {
        var rows = []
        for (var i = 0; i < componentFiles.length; i++) {
            rows.push({ tag: componentFiles[i], file: componentFiles[i] })
        }
        return rows
    }

    function test_load(data) {
        var component = Qt.createComponent("qrc:/qml/components/" + data.file)
        if (knownFailures[data.file] !== undefined) {
            expectFail(data.tag, knownFailures[data.file])
        }
        compare(component.status, Component.Ready, component.errorString())
    }
}
