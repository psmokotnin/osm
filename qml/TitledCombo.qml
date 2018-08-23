import QtQuick 2.7
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

Item {
    id: i
    implicitWidth: label.implicitWidth + cb.implicitWidth + 20
    implicitHeight: cb.implicitHeight

    property string title : ""
    property var model : []
    property int currentIndex : 0

    RowLayout {

        spacing: 1

        Text {
            id: label
            Layout.margins: 5
            text: title
        }

        ComboBox {
            id: cb
            Layout.leftMargin: 5
            Layout.bottomMargin: 5
            model: i.model
            currentIndex: i.currentIndex
            onCurrentIndexChanged: {
                i.currentIndex = currentIndex
            }
        }
    }
}
