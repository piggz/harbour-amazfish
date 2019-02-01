import QtQuick 2.0
import "."

GraphData {
    id: graphData
    property QtObject dataSource: DeviceInterfaceInstance.dataSource()

    property int type: 0
    property alias graphType: graphData.graphType
    property alias lastValue: graphData.lastValue

    function updateGraph(day) {
        var dataPoints = dataSource.data(type, day);
        setPoints(dataPoints);
    }
}
