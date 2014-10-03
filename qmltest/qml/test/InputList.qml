import QtQuick 1.1

ListView {
    id: inputs
    delegate: inputDelegate
    clip: true;

    signal inputSelected(variant input);

    Component {
        id: inputDelegate
        Rectangle {
            color: "#fff0f0";
            width: parent.width;
            height: txt.height;
            MouseArea {
                anchors.fill: parent;
                Text {
                    id: txt
                    text: model.name;
                    anchors.margins: 8;
                    font.pointSize: 12
                }
                onClicked: {
                    inputs.inputSelected(model.input_id);                    
                }
            }
        }
    }
}
