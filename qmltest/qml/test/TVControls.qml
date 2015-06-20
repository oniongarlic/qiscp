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
    Button {
        title: "1"
        enabled: iscp.connected
        onClicked: iscp.command(QISCP.Key1);
    }
    Button {
        title: "2"
        enabled: iscp.connected
        onClicked: iscp.command(QISCP.Key2);
    }
    Button {
        title: "3"
        enabled: iscp.connected
        onClicked: iscp.command(QISCP.Key3);
    }
    Button {
        title: "4"
        enabled: iscp.connected
        onClicked: iscp.command(QISCP.Key4);
    }
    Button {
        title: "5"
        enabled: iscp.connected
        onClicked: iscp.tvCommand(QISCP.Key5);
    }
    Button {
        title: "6"
        enabled: iscp.connected
        onClicked: iscp.tvCommand(QISCP.Key6);
    }
    Button {
        title: "7"
        enabled: iscp.connected
        onClicked: iscp.command(QISCP.Key7);
    }
    Button {
        title: "8"
        enabled: iscp.connected
        onClicked: iscp.command(QISCP.Key8);
    }
    Button {
        title: "9"
        enabled: iscp.connected
        onClicked: iscp.command(QISCP.Key9);
    }
    Button {
        title: "0"
        enabled: iscp.connected
        onClicked: iscp.command(QISCP.Key0);
    }
    Button {
        title: "A"
        enabled: iscp.connected
        onClicked: iscp.command(QISCP.KeyA);
    }
    Button {
        title: "B"
        enabled: iscp.connected
        onClicked: iscp.command(QISCP.KeyB);
    }
    Button {
        title: "C"
        enabled: iscp.connected
        onClicked: iscp.command(QISCP.KeyC);
    }
    Button {
        title: "D"
        enabled: iscp.connected
        onClicked: iscp.command(QISCP.KeyD);
    }
}
