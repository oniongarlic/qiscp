import QtQuick 1.1
import org.tal.qiscp 1.0

Row {

    property QISCP iscp;

    Button {
        title: "VOL+"
        enabled: iscp.connected
        onClicked: {
            iscp.volumeUp();
        }
    }
    Button {
        title: "VOL-"
        enabled: iscp.connected
        onClicked: {
            iscp.volumeDown();
        }
    }
    Button {
        title: iscp.masterMuted ? "Unmute" : "Mute"
        enabled: iscp.connected
        onClicked: {
            iscp.setMasterMuted(!iscp.masterMuted);
        }
    }

    // Bass
    Button {
        title: "Bass+"
        enabled: iscp.connected
        onClicked: {
            iscp.bassLevelUp();
        }
    }
    Button {
        title: "Bass-"
        enabled: iscp.connected
        onClicked: {
            iscp.bassLevelDown();
        }
    }

    // Treble
    Button {
        title: "Treble+"
        enabled: iscp.connected
        onClicked: {
            iscp.trebleLevelUp();
        }
    }
    Button {
        title: "Treble-"
        enabled: iscp.connected
        onClicked: {
            iscp.trebleLevelDown();
        }
    }

    // Center
    Button {
        title: "C+"
        enabled: iscp.connected
        onClicked: {
            iscp.centerLevelUp();
        }
    }
    Button {
        title: "C-"
        enabled: iscp.connected
        onClicked: {
            iscp.centerLevelDown();
        }
    }

    // Sub
    Button {
        title: "S+"
        enabled: iscp.connected
        onClicked: {
            iscp.subwooferLevelUp();
        }
    }
    Button {
        title: "S-"
        enabled: iscp.connected
        onClicked: {
            iscp.subwooferLevelDown();
        }
    }
}
