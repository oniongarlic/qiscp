import QtQuick 1.1
import org.tal.qiscp 1.0

Rectangle {
    id: root
    width: 1024
    height: 480

    Row {
        width: parent.width
        anchors.top: parent.top
        anchors.bottom: controls.top

        Column {
            width: parent.width/3
            height: parent.height
            DeviceList {
                id: hosts
                height: parent.height/2;
                width: parent.width;
                model: hostsModel
                onDeviceSelected: {
                    connectToDevice(device);
                }
                visible: !iscp.discovering
            }
            Rectangle {
                id: discovering
                width: parent.width
                height: parent.height/2
                visible: iscp.discovering
                Text {
                    id: t
                    text: "Searching for devices. Found: "+iscp.discovered;
                    font.pixelSize: 16
                    anchors.centerIn: parent
                }
                Button {
                    title: "Cancel search"
                    anchors.top: t.bottom
                    onClicked: iscp.discoverHostsCancel()
                }
            }

            Image {
                id: albumArtwork
                width: 128
                height: 128
                visible: iscp.hasArtwork
                cache: false
                opacity: status==Image.Ready ? 1.0 : 0.0
                asynchronous: true;
                Behavior on opacity { NumberAnimation { duration: 800 } }
            }

            Text {
                text: iscp.currentArtist;
                font.bold: true
                font.pointSize: 12
            }
            Text {
                text: iscp.currentAlbum;
                font.pointSize: 11
            }
            Text {
                text: iscp.currentTitle;
                font.italic: true
                font.pointSize: 12
            }

            Text {
                text: "POS:"+iscp.currentTrackPosition+"/"+iscp.currentTrackLength;
                font.italic: true
                font.pointSize: 11
            }
            Text {
                text: "Track:"+iscp.currentTrack+"/"+iscp.currentTracks;
                font.pointSize: 10
            }
            Text {
                text: "Status:"+iscp.playMode + ":"+iscp.repeatMode+":"+iscp.shuffleMode;
                font.pointSize: 10
            }
        }

        InputList {
            id: inputs
            enabled: iscp.connected
            height: parent.height;
            width: parent.width/5
            clip: true;
            model: inputsModel;
            onInputSelected: {
                iscp.setZoneInput(QISCP.Zone1, input);
            }
            currentInput: iscp.masterInput;
        }

        TunerPresetList {
            id: presets
            height: parent.height;
            width: parent.width/5
            model: presetsModel
            visible: ISCPInputs.isTuner(iscp.masterInput)
            onPresetSelected: {
                iscp.tunePreset(preset.preset_id);
            }
            currentPreset: iscp.masterTunerPreset
        }

        NetworkServicesList {
            id: services
            model: networksModel;
            height: parent.height;
            width: parent.width/5
            visible: iscp.masterInput==0x2B;
            onServiceSelected: {
                console.debug("*** Network service: "+service.service_id);
                iscp.setNetworkService(service.service_id);
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

    ListModel {
        id: networksModel
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
                visible: ISCPInputs.isTuner(iscp.masterInput)
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
            Text {
               text: "ST: "+iscp.sleepTimer
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
                    iscp.disconnectFromHost();
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
            Button {
                title: iscp.phaseMatchingBass ? "PMB Off" : "PMB On";
                enabled: iscp.connected
                onClicked: {
                    iscp.setPhaseMatchingBass(!iscp.phaseMatchingBass);
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
                title: iscp.zone2power ? "Powered on" : "Turned Off"
                onClicked: {
                    console.debug("Zone 2 power state is: "+iscp.zone2power)
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

        Row {
            width: parent.width
            spacing: 32
            Text {
                text: "Z-Power:"
            }
            Text {
                text: iscp.poweredZones & QISCP.Zone1 ? "Z1-On" : "Z1-Off"
            }
            Text {
                text: iscp.poweredZones & QISCP.Zone2 ? "Z2-On" : "Z2-Off"
            }
            Text {
                text: iscp.poweredZones & QISCP.Zone3 ? "Z3-On" : "Z3-Off"
            }
            Text {
                text: iscp.poweredZones & QISCP.Zone4 ? "Z4-On" : "Z4-Off"
            }
        }

        /* Commands */
        Row {
            Text {
                text: "Commands"
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
            Button {
                title: "S30"
                enabled: iscp.connected
                onClicked: { iscp.seekTo(30); }
            }
            Button {
                title: "S160"
                enabled: iscp.connected
                onClicked: { iscp.seekTo(160); }
            }
            Button {
                title: "S830"
                enabled: iscp.connected
                onClicked: { iscp.seekTo(830); }
            }

            Button {
                title: "RANDOM: "+iscp.shuffleMode
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Random) }
            }
            Button {
                title: "REPEAT: "+iscp.repeatMode
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Repeat) }
            }
        }

        Row {
            Button {
                title: "LEFT"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Left) }
            }
            Button {
                title: "RIGHT"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Right) }
            }
            Button {
                title: "UP"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Up) }
            }
            Button {
                title: "DOWN"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Down) }
            }
            Button {
                title: "RETURN"
                enabled: iscp.connected
                onClicked: { iscp.command(QISCP.Return) }
            }
        }

        TVControls {
            iscp: iscp
        }
    }

    Row {
        Text {
            id: audioInfo
        }
        Text {
            id: videInfo
        }
    }

    QISCP {
        id: iscp
        discoveryTimeout: 5000

        onPoweredZonesChanged: console.debug("*** PZONES: "+poweredZones)

        onDevicesDiscovered: {            
            var devices=iscp.getDevices();

            console.debug("Devices found: "+devices.length)

            if (devices.length==0)
                return;

            hostsModel.clear();
            for (var i=0;i<devices.length;i++) {
                hostsModel.append(devices[i]);
            }

            // If we find only one device, then just connect to it
            if (devices.length==1) {
                connectToDevice(devices[0]);
            }            
        }

        onDiscoveredHost: {
            console.debug("New device: "+mac)
            console.debug(device.ip)
            console.debug(device.model)
        }

        onConnectedToHost: {
            iscp.debugLog("/tmp/iscp.log", true);

            inputsModel.clear();
            var inputs=iscp.getStaticInputs();
            for (var i=0;i<inputs.length;i++) {
                inputsModel.append(inputs[i]);
            }
        }

        onDisconnectedFromHost: {
            iscp.debugLog("", false);
        }

        onVideoInfoChanged: {
            console.debug("Got Video info:")
            console.debug(arg[0])
            console.debug(arg[1])
            console.debug(arg[2])
            console.debug(arg[3])
            console.debug(arg[4])
            console.debug(arg[5])
        }

        onAudioInfoChanged: {
            console.debug("Got audio info:")
            console.debug(arg[0])
            console.debug(arg[1])
            console.debug(arg[2])
            console.debug(arg[3])
            console.debug(arg[4])
            console.debug(arg[5])
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

        onNetworkList: {
            networksModel.clear();

            var p=iscp.getNetworkSources()

            console.debug("*** Got network sources:"+p.length);

            for (var i=0;i<p.length;i++) {
                console.debug("Add: "+p[i].name);
                networksModel.append(p[i]);
            }
        }

        onCurrentArtworkChanged: {
            albumArtwork.source="";
            if (hasArtwork) {
                 // XXX just for testing!!
                iscp.saveArtwork("/tmp/artwork.png")
                albumArtwork.source="/tmp/artwork.png";
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
