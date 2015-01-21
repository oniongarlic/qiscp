import QtQuick 1.1

ListView {
    id: presets
    delegate: presetDelegate
    clip: true;

    signal presetSelected(variant preset);
    property int currentInput: -1;

    Component {
        id: presetDelegate
        Rectangle {
            color: model.input_id==presets.currentInput ? "#e5ffe5" : "#ffffff";
            width: parent.width;
            height: txt.height;
            MouseArea {
                anchors.fill: parent;
                Text {
                    id: txt
                    text: model.name + " ("+model.freq+")"
                    anchors.margins: 8;
                    font.pointSize: 12
                }
                onClicked: {
                    presets.currentIndex=index;
                    presets.presetSelected(model);
                }
            }
        }
    }
}
