import QtQuick 2.0
import "."

GraphData {
    id: graphData
    property QtObject dataSource: DaemonInterfaceInstance.dataSource()

    property int type: 0
    property alias graphType: graphData.graphType
    property alias lastValue: graphData.lastValue
    property alias lastY: graphData.lastY
    property alias lastZ: graphData.lastZ

    function updateGraph(day) {
        var dataPoints = dataSource.data(type, day);
        setPoints(dataPoints);
    }
}
