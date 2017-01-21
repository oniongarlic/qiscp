import QtQuick 1.1

Rectangle {
    id: btn
    width: abc.width+16
    height: abc.height+16
    radius: 4
    border.color: "#54ad3b"
    color: getColor(enabled, ma.pressed)

    property alias title: abc.text
    property bool enabled: true

    property string eColor: "#80E980";
    property string dColor: "#C0C0C0";

    function getColor(e,p) {
        if (p) {
            return "#90FA90"
        }

        return e ? eColor : dColor
    }

    Text {
        id: abc
        anchors.centerIn: parent;
    }
    MouseArea {
        id: ma
        anchors.fill: parent
        enabled: btn.enabled
        onClicked: {
           btn.clicked();
        }
    }
    signal clicked();
}
