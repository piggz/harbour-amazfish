import QtQuick 2.0
import QtQml 2.1
import Sailfish.Silica 1.0

import "."

Item {
    id: root
    anchors {
        left: (parent)? parent.left : undefined
        right: (parent)? parent.right : undefined
    }
    height: graphHeight + (doubleAxisXLables ? Theme.itemSizeMedium : Theme.itemSizeSmall)

    signal clicked

    property alias clickEnabled: backgroundArea.enabled
    property string graphTitle: ""
    property int line: 1
    property int bar: 2
    property int graphType: line

    property alias axisX: _axisXobject
    Axis {
        id: _axisXobject
        mask: "hh:mm"
        grid: 4
    }

    property alias axisY: _axisYobject
    Axis {
        id: _axisYobject
        mask: "%1"
        units: "%"
        grid: 4
    }

    property var valueConverter
    property bool valueTotal: false

    property int graphHeight: 250
    property int graphWidth: canvas.width / canvas.stepX
    property bool doubleAxisXLables: false

    property bool scale: false
    property color lineColor: Theme.highlightColor
    property real lineWidth: 1

    property real minY: 0 //Always 0
    property real maxY: 0

    property int minX: 0
    property int maxX: 0

    property var points: []
    onPointsChanged: {
        noData = (points.length == 0);
    }
    property bool noData: true

    function setPoints(data) {
        if (!data) return;

        var pointMaxY = 0;
        if (data.length > 0) {
            minX = data[0].x;
            maxX = data[data.length-1].x;
        }
        data.forEach(function(point) {
            if (point.y > pointMaxY) {
                pointMaxY = point.y
            }
        });
        points = data;
        if (scale) {
            maxY = pointMaxY * 1.20;
        }
        doubleAxisXLables = ((maxX - minX) > 129600); // 1,5 days

        canvas.requestPaint();
    }

    function createYLabel(value) {
        var v = value;
        if (valueConverter) {
            v = valueConverter(value);
        }
        return axisY.mask.arg(v);
    }

    function createXLabel(value) {
        var d = new Date(value*1000);
        return Qt.formatTime(d, axisX.mask);
    }

    Column {
        anchors {
            top: parent.top
            left: parent.left
            leftMargin: 3*Theme.paddingLarge
            right: parent.right
            rightMargin: Theme.paddingLarge
        }

        Label {
            width: parent.width
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeSmall
            text: graphTitle
            wrapMode: Text.Wrap

            Label {
                id: labelLastValue
                anchors {
                    right: parent.right
                }
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.Wrap
                visible: !noData
            }
        }

        Rectangle {
            width: parent.width
            height: graphHeight
            border.color: Theme.secondaryHighlightColor
            color: "transparent"

            BackgroundItem {
                id: backgroundArea
                anchors.fill: parent
                onClicked: {
                    root.clicked();
                }
            }

            Repeater {
                model: noData ? 0 : (axisY.grid + 1)
                delegate: Label {
                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeLarge / 2
                    text: createYLabel( (maxY-minY)/axisY.grid * index + minY)
                    anchors {
                        top: (index == axisY.grid) ? parent.top : undefined
                        bottom: (index == axisY.grid) ? undefined : parent.bottom
                        bottomMargin: (index) ? parent.height / axisY.grid * index - height/2 : 0
                        right: parent.left
                        rightMargin: Theme.paddingSmall
                    }
                }
            }

            Repeater {
                model: noData ? 0 : (axisX.grid + 1)
                delegate: Label {
                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeLarge / 2
                    text: createXLabel( (maxX-minX)/axisX.grid * index + minX )
                    anchors {
                        top: parent.bottom
                        topMargin: Theme.paddingSmall
                        left: (index == axisX.grid) ? undefined : parent.left
                        right: (index == axisX.grid) ? parent.right : undefined
                        leftMargin: (index) ? (parent.width / axisX.grid * index - width/2): 0
                    }
                    Label {
                        color: Theme.primaryColor
                        font.pixelSize: Theme.fontSizeLarge / 2
                        anchors {
                            top: parent.bottom
                            horizontalCenter: parent.horizontalCenter
                        }
                        text: Qt.formatDate(new Date( ((maxX-minX)/axisX.grid * index + minX) * 1000), "ddd");
                        visible: doubleAxisXLables
                    }
                }
            }

            Label {
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeLarge / 2
                text: axisY.units
                anchors {
                    top: parent.top
                    left: parent.left
                    leftMargin: Theme.paddingSmall
                }
                visible: !noData
            }

            Canvas {
                id: canvas
                anchors {
                    fill: parent
                    //leftMargin: Theme.paddingSmall
                    //rightMargin: Theme.paddingSmall
                }

                //renderTarget: Canvas.FramebufferObject
                //renderStrategy: Canvas.Threaded

                property real stepX: lineWidth
                property real stepY: (maxY-minY)/(height-2)

                function drawGrid(ctx) {
                    ctx.save();

                    ctx.lineWidth = 1;
                    ctx.strokeStyle = lineColor;
                    ctx.globalAlpha = 0.4;
                    //i=0 and i=axisY.grid skipped, top/bottom line
                    for (var i=1;i<axisY.grid;i++) {
                        ctx.beginPath();
                        ctx.moveTo(0, height/axisY.grid * i);
                        ctx.lineTo(width, height/axisY.grid * i);
                        ctx.stroke();
                    }

                    ctx.restore();
                }

                //TODO: allow multiple lines to be drawn
                function drawPoints(ctx, points) {
                }

                onPaint: {
                    var ctx = canvas.getContext("2d");
                    ctx.globalCompositeOperation = "source-over";
                    ctx.clearRect(0,0,width,height);

                    //console.log("maxY", maxY, "minY", minY, "height", height, "StepY", stepY);

                    var end = points.length;

                    if (end > 0) {
                        drawGrid(ctx);
                    }

                    ctx.save()
                    ctx.strokeStyle = lineColor;
                    //ctx.globalAlpha = 0.8;

                    //PGZ
                    lineWidth = width / end;

                    ctx.lineWidth = lineWidth;
                    ctx.beginPath();
                    var x = stepX / 2;
                    if (graphType == line) {
                        x = -stepX;
                    }
                    var valueSum = 0;
                    for (var i = 0; i < end; i++) {
                        valueSum += points[i].y;
                        var y = height - Math.floor(points[i].y / stepY) - 1;
                        if (graphType == line) {
                            if (i == 0) {
                                ctx.moveTo(x, y);
                            } else {
                                ctx.lineTo(x, y);
                            }
                        } else if (graphType == bar) {
                            ctx.moveTo(x, y);
                            ctx.lineTo(x, height);

                            if (typeof points[i].z !== 'undefined') {
                                var z = height - Math.floor((points[i].z + points[i].y)  / stepY) - 1;

                                ctx.stroke();
                                ctx.strokeStyle = Theme.secondaryHighlightColor;
                                ctx.beginPath();
                                ctx.moveTo(x, z);
                                ctx.lineTo(x, y);
                                ctx.stroke();
                                ctx.strokeStyle = lineColor;
                                ctx.beginPath();

                            }
                        }

                        x+=stepX; //point[i].x can be used for grid title
                    }
                    ctx.stroke();
                    ctx.restore();

                    if (end > 0) {
                        var lastValue = valueSum;
                        if (!root.valueTotal) {
                            lastValue = points[end-1].y;
                            if (typeof points[end-1].z !== 'undefined') {
                                lastValue += points[end-1].z
                            }
                        }
                        if (lastValue) {
                            labelLastValue.text = root.createYLabel(lastValue)+root.axisY.units;
                        }
                    }
                }
            }

            Text {
                id: textNoData
                anchors.centerIn: parent
                color: lineColor
                text: qsTr("No data");
                visible: noData
            }
        }
    }
}
