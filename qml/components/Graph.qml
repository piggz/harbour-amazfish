import QtQuick 2.0
import "."

GraphData {
    property QtObject dataSource: sysmon


    function updateGraph() {
        var dataPoints = dataSource.getSystemGraph(dataType, dataDepth, graphWidth, dataAvg);
        setPoints(dataPoints);
    }
}
