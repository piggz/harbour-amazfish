import QtQuick 2.0
import "."

GraphData {
    property QtObject dataSource: DeviceInterfaceInstance.dataSource()

    property int type: 0

    function updateGraph(day) {
        var dataPoints = dataSource.data(type, day);
        setPoints(dataPoints);
    }
}
