/**
 *  OSM
 *  Copyright (C) 2018  Pavel Smokotnin

 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
import QtQuick 2.12
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtGraphicalEffects 1.15
import OpenSoundMeter 1.0
import "elements"
import "SPL" as SPL;

Item {
    id: chartview
    property alias type: chart.type
    property alias settings: chart.settings

    VariableChart {
        id: chart
        anchors.fill: parent
        sources: sourceList
        darkMode: applicationAppearance.darkMode

        Component.onCompleted: {
            initOpener();
        }
        Connections {
            target: chart.plot

            function onRendererErrorChanged() {
                if (chart.plot.rendererError) {
                    message.showError(chart.plot.rendererError);
                }
            }
        }
        Connections {
            target: sourceList
            function onPostItemRemoved() {
                gcTimer.running = true;
            }
        }
        Timer {
            id: gcTimer
            interval: 100
            running: false
            repeat: false
            onTriggered: {
                gc();
            }
        }

        onTypeChanged: function() {
            initOpener();
        }

        function initOpener() {
            opener.propertiesQml = null;
            switch(type) {
            case "Spectrum":
                opener.propertiesQml = "qrc:/Plot/RTAProperties.qml";
                break;
            case "Magnitude":
                opener.propertiesQml = "qrc:/Plot/MagnitudeProperties.qml";
                break;
            case "Phase":
                opener.propertiesQml = "qrc:/Plot/PhaseProperties.qml";
                break;
            case "Group Delay":
                opener.propertiesQml = "qrc:/Plot/GroupDelayProperties.qml";
                break;
            case "Phase Delay":
                opener.propertiesQml = "qrc:/Plot/PhaseDelayProperties.qml";
                break;
            case "Impulse":
                opener.propertiesQml = "qrc:/Plot/ImpulseProperties.qml";
                break;
            case "Step":
                opener.propertiesQml = "qrc:/Plot/StepProperties.qml";
                break;
            case "Coherence":
                opener.propertiesQml = "qrc:/Plot/CoherenceProperties.qml";
                break;
            case "Spectrogram":
                opener.propertiesQml = "qrc:/Plot/SpectrogramProperties.qml";
                break;
            case "Crest Factor":
                opener.propertiesQml = "qrc:/Plot/CrestFactorProperties.qml";
                break;
            case "Nyquist":
                opener.propertiesQml = "qrc:/Plot/NyquistProperties.qml";
                break;
            case "Level":
                opener.propertiesQml = "qrc:/Plot/LevelProperties.qml";
                break;
            }
        }
    }

    MultiPointTouchArea {
        id: touchArea
        anchors.fill: parent
        mouseEnabled: false

        readonly property int gestureNone  : 0;
        readonly property int gestureZoomX  : 1;
        readonly property int gestureZoomY  : 2;
        readonly property int gestureZoomXY : 3;
        readonly property int gestureMoveX  : 4;
        readonly property int gestureMoveY  : 5;
        readonly property int gestureMoveXY : 6;

        property int gesture: gestureNone;
        touchPoints: [
            TouchPoint { id: touchPoint1 },
            TouchPoint { id: touchPoint2 }
        ]
        onReleased: function(e) {
            if (gesture === gestureNone && touchPoint1.pressed && !touchPoint2.pressed) {
                opener.openCalculator();
                opener.mouseButtonClicked = Qt.NoButton;
            }

            gesture = gestureNone;
        }

        onGestureStarted: function (gestureEvent) {

            let directionIndexForPoint = function (point) {
                let direction = Qt.point(point.x - point.startX, point.startY - point.y);
                let alpha = Math.atan2(direction.y, direction.x) * 180 / Math.PI;
                if (alpha < 0) {
                    alpha += 360;
                }
                let directionIndex = Math.round((alpha / 45) % 8);
                if (directionIndex === 8) {
                    directionIndex = 0;
                }
                return directionIndex;
            }

            if (chart.plot && gestureEvent.touchPoints.length === 2) {
                let p1 = gestureEvent.touchPoints[0];
                let p2 = gestureEvent.touchPoints[1];

                let di1 = directionIndexForPoint(p1);
                let di2 = directionIndexForPoint(p2);

                let gestrureMatrix = [
                        [gestureMoveX, 0, 0, 0, gestureZoomX, 0, 0, 0],
                        [0, gestureMoveXY, 0, 0, 0, gestureZoomXY, 0, 0],
                        [0, 0, gestureMoveY, 0, 0, 0, gestureZoomY, 0],
                        [0, 0, 0, gestureMoveXY, 0, 0, 0, gestureZoomXY],
                        [gestureZoomX, 0, 0, 0, gestureMoveX, 0, 0, 0],
                        [0, gestureZoomXY, 0, 0, 0, gestureMoveXY, 0, 0],
                        [0, 0, gestureZoomY, 0, 0, 0, gestureMoveY, 0],
                        [0, 0, 0, gestureZoomXY, 0, 0, 0, gestureMoveXY]
                    ];

                let currentGesture = gestrureMatrix[di1][di2];
                if (gesture === gestureNone) {
                    //begin gesture;
                    gesture = currentGesture;
                    chart.plot.beginGesture();
                }

                if (currentGesture !== gesture) {
                    return;
                }

                let base1 = Qt.point(p1.startX, p1.startY);
                let base2 = Qt.point(p2.startX, p2.startY);
                let base = Qt.point(
                        (p1.startX + p2.startX) / 2,
                        (p1.startY + p2.startY) / 2
                );
                let touchPoint1 = Qt.point(p1.x, p1.y);
                let touchPoint2 = Qt.point(p2.x, p2.y);
                let move = Qt.point(0, 0);
                let scale = Qt.point(1, 1);

                switch (gesture) {
                    case gestureZoomX:
                        scale.x = Math.abs(base1.x - base2.x) / parseFloat(Math.abs(touchPoint1.x - touchPoint2.x));
                        break;
                    case gestureZoomY:
                        scale.y = Math.abs(base1.y - base2.y) / parseFloat(Math.abs(touchPoint1.y - touchPoint2.y));
                        break;
                    case gestureZoomXY:
                        scale.x = Math.abs(base1.x - base2.x) / parseFloat(Math.abs(touchPoint1.x - touchPoint2.x));
                        scale.y = Math.abs(base1.y - base2.y) / parseFloat(Math.abs(touchPoint1.y - touchPoint2.y));
                        break;
                    case gestureMoveX:
                        move.x = touchPoint1.x - base1.x;
                        break;
                    case gestureMoveY:
                        move.y = base1.y - touchPoint1.y;
                        break;
                    case gestureMoveXY:
                        move.x = touchPoint1.x - base1.x;
                        move.y = base1.y - touchPoint1.y;
                        break;
                }
                chart.plot.applyGesture(base, move, scale);
            }
        }

        PropertiesOpener {
            id: opener
            property int mouseButtonClicked: Qt.NoButton
            pushObject: chart.plot;
            enabled: chart.plot
            cursorShape: "CrossCursor";
            hoverEnabled: true
            onEntered: cursor.visible = true
            onExited: cursor.visible = false
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onPressed: {
                if (pressedButtons & Qt.LeftButton) {
                    mouseButtonClicked = Qt.LeftButton;
                } else if (pressedButtons & Qt.RightButton) {
                    mouseButtonClicked = Qt.RightButton;
                }
            }

            onClicked: function(e) {
                if (mouseButtonClicked === Qt.LeftButton) {
                    open();
                } else if (mouseButtonClicked === Qt.RightButton) {
                    openCalculator();
                }
            }

            onDoubleClicked: {
                chart.plot.resetAxis();
            }

            onWheel: function (e) {
                let d = Math.max(Math.abs(e.angleDelta.x), Math.abs(e.angleDelta.y));

                if (d >= 120) {
                    let base = Qt.point(e.x, e.y);
                    let move = Qt.point(0.0, 0.0);
                    let scale = Qt.point(1.0, 1.0);
                    move = Qt.point(e.angleDelta.x / 8, e.angleDelta.y / 8);

                    chart.plot.beginGesture();
                    chart.plot.applyGesture(base, move, scale);

                    e.accepted = true;
                }
            }

            function openCalculator() {
                var obj = {};
                switch(type) {
                    case "Spectrum":
                    case "Magnitude":
                    case "Phase":
                    case "Group Delay":
                    case "Phase Delay":
                    case "Coherence":
                    case "Spectrogram":
                    case "Crest Factor":
                        obj.frequency = chart.plot.x2v(opener.mouseX);
                        break;
                    case "Impulse":
                    case "Step":
                        obj.time = Math.abs(chart.plot.x2v(opener.mouseX));
                        break
                    case "Nyquist":
                    case "Level":
                        return;
                }
                applicationWindow.properiesbar.open(obj, "qrc:/Calculator.qml");
            }
        }
    }
    Label {
        id: cursor
        property int xRight;
        property int xLeft;
        property int yTop;
        property int yBottom;
        property int cursorX : 0
        property int cursorY : 0

        text: (chart.plot ? "%1".arg(chart.plot.y2v(cursor.cursorY).toFixed(2)) + chart.plot.yLabel + "\n" +
              "%1".arg(chart.plot.x2v(cursor.cursorX).toFixed(2)) + chart.plot.xLabel : "");
        xRight:  cursorX + applicationAppearance.cursorOffset + cursor.fontInfo.pixelSize / 2
        xLeft:   cursorX - applicationAppearance.cursorOffset - cursor.width - cursor.fontInfo.pixelSize / 2
        yTop:    cursorY - applicationAppearance.cursorOffset - cursor.height / 2
        yBottom: cursorY + applicationAppearance.cursorOffset + cursor.height / 2
        x: xRight < parent.width  - width  - 50 ? xRight : xLeft
        y: yTop   < 2 * height ? yBottom : yTop + 30 + height / 2 > parent.height ? yTop - height : yTop
        visible: chart.plot && opener.containsMouse

        Label {
            id: cursorText
            text: cursor.text
        }

        DropShadow {
            anchors.fill: cursorText
            horizontalOffset: 0
            verticalOffset: 0
            radius: 1.0
            samples: 9
            spread: 1
            color: applicationAppearance.darkMode ? "#99000000" : "#99FFFFFF"
            source: cursorText
        }

        onCursorXChanged: {
            if (chart.plot) chart.plot.setHelper(cursorX, cursorY);
        }
        onCursorYChanged: {
            if (chart.plot) chart.plot.setHelper(cursorX, cursorY);
        }

        Connections {
            target: opener
            function onMouseXChanged() {
                var x = touchPoint2.pressed ? touchPoint1.startX : opener.mouseX;
                cursor.cursorX = x;
                if (chart.plot) chart.plot.setHelper(cursor.cursorX, cursor.cursorY);
            }
            function onMouseYChanged() {
                var y = touchPoint2.pressed ? touchPoint1.startY : opener.mouseY;
                cursor.cursorY = y;
                if (chart.plot) chart.plot.setHelper(cursor.cursorX, cursor.cursorY);
            }
            function onExited() {
                if (chart.plot) chart.plot.unsetHelper();
            }
        }
    }

    SPL.Grid {
        enabled: chart.type == "Numeric"
        visible: chart.type == "Numeric"

        Component.onCompleted: {
            settings = chartview.settings;
        }
    }

    DropDown {
        anchors.top: parent.top
        anchors.right: parent.right
        implicitWidth: 145
        implicitHeight: Material.buttonHeight
        background: null
        model: applicationAppearance.experimentFunctions ?
                   ["Spectrum", "Magnitude", "Phase", "Impulse", "Step", "Coherence", "Group Delay", "Phase Delay", "Spectrogram", "Level", "Numeric", "Crest Factor", "Nyquist"] :
                   ["Spectrum", "Magnitude", "Phase", "Impulse", "Step", "Coherence", "Group Delay", "Spectrogram", "Level", "Numeric"]
        currentIndex: model.indexOf(type)
        onCurrentIndexChanged: {
            var pb = applicationWindow.properiesbar;
            var reopen = false;
            if (pb.currentObject === chart.plot) {
                pb.reset();
                reopen = true;
            }
            chart.type = model[currentIndex];
            if (chart.plot && model[currentIndex] === "Spectrogram") {
                chart.plot.selected = [sourceList.first];
            }
            if (chart.plot && reopen) {
                pb.open(chart.plot, opener.propertiesQml);
            }
        }
        Component.onCompleted: {
            chart.type = model[currentIndex];
        }
    }
}
