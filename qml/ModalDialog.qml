import QtQuick 2.12
import QtQuick.Controls 2.12

Dialog {
      title: ""
      modal: true
      x: (parent ? (parent.width  - width) / 2 : 100)
      y: (parent ? (parent.height - height) / 2 : 100)
      standardButtons: Dialog.Ok | Dialog.Cancel
      onAccepted: {}
      onRejected: {}
  }
