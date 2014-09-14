import QtQuick 1.1

ListView {
    id: presets
    delegate: presetDelegate
    clip: true;

    signal presetSelected(variant preset);

    Component {
        id: presetDelegate
        Rectangle {
            color: "#f0f0ff";
            width: parent.width;
            height: txt.height;
            MouseArea {
                anchors.fill: parent;
                Text {
                    id: txt
                    text: model.name + "("+model.freq+")"
                    anchors.margins: 8;
                    font.pointSize: 12
                }
                onClicked: {
                    presets.presetSelected(model);
                }
            }
        }
    }
}
