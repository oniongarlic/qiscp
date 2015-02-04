import QtQuick 1.1
import org.tal.qiscp 1.0

Row {
    id: tvCmds

    property QISCP iscp;

    Text {
        text: "TV/CEC"
    }
    Button {
        title: "Power"
        enabled: iscp.connected
        onClicked: { iscp.tvCommand(QISCP.Power) }
    }
    Button {
        title: "Prog+"
        enabled: iscp.connected
        onClicked: { iscp.tvCommand(QISCP.ChannelUp) }
    }
    Button {
        title: "Prog-"
        enabled: iscp.connected
        onClicked: { iscp.tvCommand(QISCP.ChannelDown) }
    }
    Button {
        title: "Vol+"
        enabled: iscp.connected
        onClicked: { iscp.tvCommand(QISCP.VolumeUp) }
    }
    Button {
        title: "Vol-"
        enabled: iscp.connected
        onClicked: { iscp.tvCommand(QISCP.VolumeDown) }
    }
    Button {
        title: "Mute"
        enabled: iscp.connected
        onClicked: { iscp.tvCommand(QISCP.Mute) }
    }
    Button {
        title: "Guide"
        enabled: iscp.connected
        onClicked: { iscp.tvCommand(QISCP.Guide) }
    }
    Button {
        title: "Ent"
        enabled: iscp.connected
        onClicked: { iscp.tvCommand(QISCP.Enter) }
    }
    Button {
        title: "Previous"
        enabled: iscp.connected
        onClicked: { iscp.tvCommand(QISCP.Previous) }
    }
}

