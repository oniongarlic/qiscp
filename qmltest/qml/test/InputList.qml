import QtQuick 1.1

ListView {
    id: inputs
    delegate: inputDelegate
    clip: true;

    signal inputSelected(variant input);
    property int currentInput: -1;

    Component {
        id: inputDelegate
        Rectangle {
            color: model.input_id==inputs.currentInput ? "#e5e5e5" : "#ffffff";
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
                    console.debug("Input ID: "+model.input_id);
                    inputs.inputSelected(model.input_id);                    
                }
            }
        }
    }
}
