import QtQuick 2.0
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Item {
    RowLayout {
        ProgressBar {
            value: 0.5

            contentItem: Item
        }
        Button {
            text: qsTr("Settings")
        }
    }
}
