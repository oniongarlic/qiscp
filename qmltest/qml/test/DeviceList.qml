import QtQuick 1.1

ListView {
    id: hosts
    delegate: hostDelegate
    clip: true;

    signal deviceSelected(variant device);

    Component {
        id: hostDelegate
        Rectangle {
            color: "#f0f0ff";
            width: parent.width;
            height: txt.height;
            MouseArea {
                anchors.fill: parent;
                Text {
                    id: txt
                    text: model.model+" / "+model.mac
                    anchors.margins: 8;
                    font.pointSize: 12
                }
                onClicked: {
                    hosts.currentIndex=index;
                    hosts.deviceSelected(model);
                }
            }
        }
    }
}
