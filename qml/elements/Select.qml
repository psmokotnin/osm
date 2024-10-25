/**
 *  OSM
 *  Copyright (C) 2022  Pavel Smokotnin

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
import QtQuick 2.15
import QtQuick.Controls 2.15
import SourceModel 1.0

DropDown {
    id: control
    property SourceList sources;
    property var dataObject
    property string checkedRole : "checked"
    property string colorRole   : "color"
    property string tooltip : ""

    ToolTip.visible: (tooltip ? hovered : none)
    ToolTip.text: tooltip

    textRole: "title"
    valueRole: "source"

    model: SourceModel {
        id: sourceModel
        addNone: true
        addAll: true
        unrollGroups: true
        list: sources
        checked: dataObject.selected
        onCheckedChanged: {
            dataObject.selected = checked;
        }
    }

    delegate: ColoredMenuItem {
        width: ListView.view.width
        text: model[control.textRole]
        hoverEnabled: control.hoverEnabled
        checkable: model["source"] ? true : false
        checked: model[control.checkedRole]
        checkedColor: model[control.colorRole]

        MouseArea {
            anchors.fill: parent

            hoverEnabled: true;

            onClicked: {
                if (model["source"]) {
                    sourceModel.check(index, !parent.checked);
                } else {
                    sourceModel.check(index, true);
                }
                updateDisplayText();
            }
        }

        Connections {
            target: sourceModel
            function onCheckedChanged() {
                checked = model[control.checkedRole];
            }
        }
    }

    Component.onCompleted: {
        updateDisplayText();
    }

    function updateDisplayText() {
        let count = sourceModel.checkedCount();
        if (count === 0) {
            control.displayText = qsTr("None");
        } else if (count === 1) {
            control.displayText = sourceModel.firstChecked()["name"];
        } else if (count === sourceModel.count) {
            control.displayText = qsTr("All");
        } else {
            control.displayText = qsTr("Many");
        }
    }
}
