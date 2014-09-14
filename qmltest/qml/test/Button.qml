import QtQuick 1.1

Rectangle {
    id: btn
    width: abc.width+16
    height: 32
    color: enabled ? "#80E980" : "#C0C0C0"
    property alias title: abc.text
    property bool enabled: true
    Text {
        id: abc
        anchors.centerIn: parent;
    }
    MouseArea {
        anchors.fill: parent
        enabled: btn.enabled
        onClicked: {
           btn.clicked();
        }
    }
    signal clicked();
}
