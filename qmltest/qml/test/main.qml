import QtQuick 1.1
import org.tal.qiscp 1.0

Rectangle {
    id: root
    width: 800
    height: 480

    Row {
        width: parent.width
        anchors.top: parent.top
        anchors.bottom: controls.top

        Column {
            width: parent.width/2
            height: parent.height
            DeviceList {
                id: hosts
                height: parent.height/2;
                width: parent.width;
                model: hostsModel
                onDeviceSelected: {
                    connectToDevice(device);
                }
            }
            Text {
                text: iscp.currentArtist;
                font.pointSize: 12
            }
            Text {
                text: iscp.currentAlbum;
                font.pointSize: 12
            }
            Text {
                text: iscp.currentTitle;
                font.pointSize: 12
            }
            Text {
                text: iscp.currentTrackPosition+"/"+iscp.currentTrackLength;
                font.pointSize: 12
            }
        }

        InputList {
            id: inputs
            enabled: iscp.connected
            height: parent.height;
            width: parent.width/4
            clip: true;
            model: inputsModel;
            onInputSelected: {
                iscp.setMasterInput(input);
            }
        }

        TunerPresetList {
            id: presets
            height: parent.height;
            width: parent.width/4
            model: presetsModel
            onPresetSelected: {
                iscp.tunePreset(preset.preset_id);
            }
        }
    }   

    ListModel {
        id: presetsModel
    }

    ListModel {
        id: hostsModel
    }

    ListModel {
        id: inputsModel
    }

    Column {
        id: controls
        width: parent.width
        anchors.bottom: parent.bottom

        Row {

            width: parent.width

            TextEdit {
                id: cmdText
                focus: true
                textFormat: TextEdit.PlainText
                width: parent.width/6
            }

            TextEdit {
                id: paramText
                focus: true;
                textFormat: TextEdit.PlainText
                width: parent.width/6
            }

            Button {
                id: sendBtn
                enabled: cmdText.text!=='' && paramText!==''
                title: "Send CMD"
                onClicked: {
                    console.debug("Sending CMD: "+cmdText.text)
                    iscp.writeCommand(cmdText.text, paramText.text);
                }
                width: parent.width/3
            }
        }

        Row {
            id: infoRow
            width: parent.width
            spacing: 8

            Text {
                text: "VOL:" + iscp.masterVolume                
            }

            Text {
                text: "B:" + iscp.bassLevel;
            }

            Text {
                text: "T:" + iscp.trebleLevel;
            }

            Text {
                text: "C:" + iscp.centerLevel;
            }

            Text {
                text: "S:" + iscp.subwooferLevel;
            }

            Text {
                visible: iscp.masterInput===0x24 || iscp.masterInput===0x25;
                text: "Freq:" + formatFreq(iscp.masterTunerFreq);
                function formatFreq(freq) {
                    // AM 535-1605 kHz
                    if (freq>=535 && freq<=1605) {
                        return freq+"kHz"
                    } else {
                        // FM 88 to 108
                        return freq+" MHz"
                    }
                }
            }
        }

        GenericControls {
            iscp: iscp
        }

        // Main zone commands
        Row {
            id: cmdRow
            width: parent.width
            Text {
                text: "Main"
            }

            Button {
                title: "Discover"
                onClicked: {
                    iscp.discoverHosts();
                }
            }
            Button {
                enabled: iscp.connected
                title: "DisCon"
                onClicked: {
                    iscp.close();
                }
            }
            Button {
                enabled: iscp.connected
                title: iscp.power ? "Master On" : "Master Off"
                onClicked: {
                    iscp.setPower(!iscp.power)
                }
            }

            Button {
                title: "Tune+"
                enabled: iscp.connected
                onClicked: {
                    iscp.tuneUp();
                }
            }
            Button {
                title: "Tune-"
                enabled: iscp.connected
                onClicked: {
                    iscp.tuneDown();
                }
            }


            Button {
                title: "Preset+"
                enabled: iscp.connected
                onClicked: {
                    iscp.presetUp();
                }
            }
            Button {
                title: "Preset-"
                enabled: iscp.connected
                onClicked: {
                    iscp.presetDown();
                }
            }
            // misc
            Button {
                title: iscp.musicOptimizer ? "MO Off" : "MO On";
                enabled: iscp.connected
                onClicked: {
                    iscp.setMusicOptimizer(!iscp.musicOptimizer);
                }
            }
        }

        // Zone2 commands
        Row {
            id: cmdZ2Row
            width: parent.width
            Text {
                text: "Zone 2"
            }

            Button {
                enabled: iscp.connected
                title: iscp.zone2power ? "On" : "Off"
                onClicked: {
                    iscp.setZone2Power(!iscp.zone2power)
                }
            }

            Button {
                title: "Tune+"
                enabled: iscp.connected
                onClicked: {
                    iscp.tuneUp(QISCP.Zone2);
                }
            }
            Button {
                title: "Tune-"
                enabled: iscp.connected
                onClicked: {
                    iscp.tuneDown(QISCP.Zone2);
                }
            }

            Button {
                title: "Preset+"
                enabled: iscp.connected
                onClicked: {
                    iscp.presetUp(QISCP.Zone2);
                }
            }
            Button {
                title: "Preset-"
                enabled: iscp.connected
                onClicked: {
                    iscp.presetDown(QISCP.Zone2);
                }
            }
        }


        /* BD/DVD */
        Row {
            Text {
                text: "BD/DVD"
            }
            Button {
                title: "Power"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Power) }
            }
            Button {
                title: "Menu"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Menu) }
            }
            Button {
                title: "Select"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Select) }
            }
            Button {
                title: "Play"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Play) }
            }
            Button {
                title: "Stop"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Stop) }
            }
            Button {
                title: "Pause"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Pause) }
            }
            Button {
                title: "|<<"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.SkipReverse) }
            }
            Button {
                title: ">>|"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.SkipForward) }
            }
            Button {
                title: "T<<"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.TrackDown) }
            }
            Button {
                title: ">>T"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.TrackUp) }
            }
        }

        Row {
            id: tvCmds
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
        }
    }

    QISCP {
        id: iscp
        discoveryTimeout: 5000

        onDevicesDiscovered: {
            console.debug("Devices found")
            var devices=iscp.getDevices();
            hostsModel.clear();
            for (var i=0;i<devices.length;i++) {
                hostsModel.append(devices[i]);
            }

            // If we find only one device, then just connect to it
            if (devices.length==1) {
                connectToDevice(devices[0]);
            }

            inputsModel.clear();
            var inputs=iscp.getStaticInputs();
            for (var i=0;i<inputs.length;i++) {
                inputsModel.append(inputs[i]);
            }
        }

        onZonesList: {
            var zones=iscp.getZones();

            console.debug("Got zones list:"+zones.length)
        }

        onInputsList: {
            var inputs=iscp.getInputs();

            console.debug("Got inputs list:"+inputs.length)

            if (inputs.length==0)
                return;

            inputsModel.clear();
            for (var i=0;i<inputs.length;i++) {
                var input=inputs[i];
                if (input.value==1)
                    inputsModel.append(input);
            }
        }

        onPresetsList: {
            presetsModel.clear();
            var p=iscp.getPresets();

            console.debug("*** Got presets:"+p.length);

            for (var i=0;i<p.length;i++) {
                console.debug("Add: "+p[i].freq);
                presetsModel.append(p[i]);
            }
        }

    }

    function connectToDevice(device) {
        console.debug("Connecting to host: "+device.ip);
        iscp.host=device.ip;
        iscp.port=device.port;
        iscp.connectToHost();
    }
}
