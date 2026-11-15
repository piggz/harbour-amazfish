import QtQuick
import QtTest
import "qrc:/qml/components/platform"

Item {
    id: root
    width: 200
    height: 200

    Component {
        id: itemComponent
        PageMenuItemPL {}
    }

    TestCase {
        name: "PageMenuItemPL"

        function test_icon_data() {
            return [
                { tag: "iconName only", iconName: "edit-delete", iconSource: "", expected: "edit-delete" },
                { tag: "iconSource only", iconName: "", iconSource: "document-save", expected: "document-save" },
                { tag: "both, iconSource wins", iconName: "edit-delete", iconSource: "document-save", expected: "document-save" },
                { tag: "neither", iconName: "", iconSource: "", expected: "" }
            ]
        }

        function test_icon(data) {
            var item = createTemporaryObject(itemComponent, root, { iconName: data.iconName, iconSource: data.iconSource })
            verify(!!item, "Component exists")
            compare(item.icon.name, data.expected)
        }

        function test_iconChangesLater() {
            var item = createTemporaryObject(itemComponent, root)
            verify(!!item, "Component exists")
            item.iconName = "edit-delete"
            compare(item.icon.name, "edit-delete")
            item.iconSource = "document-save"
            compare(item.icon.name, "document-save")
            item.iconSource = ""
            compare(item.icon.name, "edit-delete")
        }
    }
}
