import QtQuick 1.1

ListView {
    id: services
    delegate: serviceDelegate
    clip: true;

    signal serviceSelected(variant service);
    property int currentService: -1;

    Component {
        id: serviceDelegate
        Rectangle {
            color: model.service_id==services.currentService ? "#a5ffef" : "#ffffff";
            width: parent.width;
            height: txt.height;
            MouseArea {
                anchors.fill: parent;
                Text {
                    id: txt
                    text: model.name
                    anchors.margins: 8;
                    font.pointSize: 12
                }
                onClicked: {
                    services.currentIndex=index;
                    services.serviceSelected(model);
                }
            }
        }
    }
}
