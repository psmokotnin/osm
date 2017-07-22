import QtQuick 2.0
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Item {
    Column {
        ProgressBar {
            value: 0.5
        }
        ProgressBar {
            indeterminate: true
        }
    }
}
