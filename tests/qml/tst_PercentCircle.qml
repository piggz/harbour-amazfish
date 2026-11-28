import QtQuick
import QtTest
import "qrc:/qml/components"
import "qrc:/qml/components/platform"

Item {
    id: root
    width: 200
    height: 200

    // Components look up the app-wide styler by id, as in harbour-amazfish.qml.
    StylerPL {
        id: styler
    }

    Component {
        id: percentCircleComponent
        PercentCircle {}
    }

    TestCase {
        name: "PercentCircle"
        when: windowShown

        function test_defaults() {
            var circle = createTemporaryObject(percentCircleComponent, root)
            verify(!!circle, "Component exists")
            compare(circle.percent, 0)
            compare(circle.size, 0)
            compare(circle.color, "#00000000")
            compare(circle.gradientColor, styler.themeSecondaryHighlightColor)
        }

        function test_sizeSetsImplicitSize() {
            var circle = createTemporaryObject(percentCircleComponent, root, { size: 120 })
            verify(!!circle, "Component exists")
            compare(circle.implicitWidth, 120)
            compare(circle.implicitHeight, 120)
            circle.size = 0
            compare(circle.implicitWidth, 0)
            compare(circle.implicitHeight, 0)
        }

        function test_percent_data() {
            return [
                { tag: "empty", percent: 0 },
                { tag: "half", percent: 0.5 },
                { tag: "full", percent: 1 },
                { tag: "over", percent: 1.5 }
            ]
        }

        function test_percent(data) {
            var circle = createTemporaryObject(percentCircleComponent, root, { size: 100, widthRatio: 0.1 })
            verify(!!circle, "Component exists")
            circle.percent = data.percent
            compare(circle.percent, data.percent)
        }
    }
}
