/**
 *  OSM
 *  Copyright (C) 2021  Pavel Smokotnin

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
import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15

ComboBox {
    id: control

    Connections {
        target: control.popup.exit
        function onRunningChanged() {
            if (!control.popup.exit.running) {
                control.popup.scale = 1.0;
            }
        }
    }

    popup.onOpened: {
        //Popup ListView    MenuItem       IconLabel
        //popup.contentItem.itemAtIndex(0).contentItem

        let maxWidth = control.width;
        if (text) {
            for (let i = 0; i < popup.contentItem.count; ++i) {
                text.text = popup.contentItem.itemAtIndex(i) ? popup.contentItem.itemAtIndex(i).text : "";

                //MenuItem{padding: 16}
                maxWidth = Math.max(maxWidth, text.paintedWidth + popup.padding * 2 + 32);
            }
        }
        popup.width = maxWidth;
        let x = control.mapToGlobal(0, 0).x - applicationWindow.x + 2; //2 pixels padding
        if (x + popup.width > applicationWindow.width) {
            popup.x = applicationWindow.width - x - popup.width;
        }
    }

    Text {
        id: text
        text: ""
        font.pixelSize: 18
        visible: false
    }
}
