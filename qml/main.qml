import QtQuick 2.0
import QtQuick.Controls 2.1
import QtQuick.Controls.Material 2.1


ApplicationWindow {
    visible: true

    width: 600
    height: 300

    Material.theme: Material.Light
    Material.accent: Material.Indigo

    Generator {
        id: gen
    }

    Measure {
    }

}
