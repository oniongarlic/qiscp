#include "qiscp.h"
#include "iscpmsg.h"

#include <QtGlobal>
#include <QNetworkAddressEntry>
#include <QStringList>
#include <QSettings>

static QString getPaddedInt(int n, int padding=5) {
    QString s=QString("%1").arg(n, padding, 10, QChar('0')).toUpper();
    return s;
}

static QString getHex(int n, int padding=2) {
    QString s=QString("%1").arg(n, padding, 16, QChar('0')).toUpper();
    return s;
}

static QString getHexWithPrefix(int n) {
    QString s=QString("%1").arg(n, 2, 16, n<0 ? QChar('-') : QChar('+')).toUpper();
    return s;
}

qiscp::qiscp(QObject *parent) :
    QObject(parent),
    m_port(ISCP_PORT),
    m_discoveryTimeout(2000),
    m_discovering(false),
    m_connected(false),
    m_debug(false),
    m_power(false),
    m_z2Power(false),
    m_z3Power(false),
    m_z4Power(false),
    m_poweredZones(0),
    m_masterMuted(false),
    m_masterVolume(0),
    m_maxvolume(20),
    m_zonesAvailable(Zone1),
    m_masterTunerFreq(0),
    m_timeRef(0,0),
    m_hasArtwork(false),
    m_discovered(0)
{
    m_socket=new QTcpSocket(this);
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readISCP()));
    connect(m_socket, SIGNAL(connected()), this, SLOT(tcpConnected()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(tcpDisconnected()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpError(QAbstractSocket::SocketError)));

    m_broadcast=new QUdpSocket(this);
    m_broadcast->bind(QHostAddress::Any, ISCP_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(m_broadcast, SIGNAL(readyRead()), this, SLOT(readBroadcastDatagram()));

    m_buffer.resize(1024);

    m_cmdtimer.setInterval(100);
    connect(&m_cmdtimer, SIGNAL(timeout()), this, SLOT(handleCommandQueue()));

    // Command to ID mapping
    // Known commands
    m_commands.insert("PWR", ISCPCommands::MasterPower);
    m_commands.insert("MVL", ISCPCommands::MasterVolume);
    m_commands.insert("AMT", ISCPCommands::MasterMute);
    m_commands.insert("SLI", ISCPCommands::MasterInput);
    m_commands.insert("SLP", ISCPCommands::SleepTimer);

    /**
     * Tuner is a shared resource we don't need to handle it for every zone,
     * but control is still separate/zone so we still need to handle
     * the zone specific tuner information messages even if there is only one current
     * setting. So what we do is just take them as master tuner information
    */
    m_commands.insert("TUN", ISCPCommands::MasterTuner);
    m_commands.insert("TUZ", ISCPCommands::MasterTuner);
    m_commands.insert("TU3", ISCPCommands::MasterTuner);
    m_commands.insert("TU4", ISCPCommands::MasterTuner);

    m_commands.insert("PRS", ISCPCommands::MasterTunerPreset);
    m_commands.insert("PRZ", ISCPCommands::MasterTunerPreset);
    m_commands.insert("PR3", ISCPCommands::MasterTunerPreset);
    m_commands.insert("PR4", ISCPCommands::MasterTunerPreset);

    // Levels: Treble/Base, Center and Subwoofer
    m_commands.insert("TFR", ISCPCommands::MasterTone);
    m_commands.insert("CTL", ISCPCommands::CenterLevel);
    m_commands.insert("SWL", ISCPCommands::SubwooferLevel);

    // USB/Network
    m_commands.insert("NAL", ISCPCommands::CurrentAlbum);
    m_commands.insert("NAT", ISCPCommands::CurrentArtist);
    m_commands.insert("NTI", ISCPCommands::CurrentTitle);
    m_commands.insert("NRT", ISCPCommands::CurrentTrack);
    m_commands.insert("NTM", ISCPCommands::ElapsedTime);
    m_commands.insert("NST", ISCPCommands::PlayStatus);
    m_commands.insert("NJA", ISCPCommands::Artwork);
    m_commands.insert("NTR", ISCPCommands::TrackInfo);
    m_commands.insert("NRI", ISCPCommands::DeviceInformation);
    m_commands.insert("NDS", ISCPCommands::DeviceStatus);
    m_commands.insert("NPR", ISCPCommands::NetworkRadioPreset);

    // Menu
    m_commands.insert("NMS", ISCPCommands::MenuStatus);    
    m_commands.insert("NLS", ISCPCommands::MenuList);
    m_commands.insert("NLT", ISCPCommands::MenuListTitle);


    // XXX: Needs feedback, but should work
    m_commands.insert("AAL", ISCPCommands::CurrentAlbum);
    m_commands.insert("AAT", ISCPCommands::CurrentArtist);
    m_commands.insert("ATI", ISCPCommands::CurrentTitle);
    m_commands.insert("ATM", ISCPCommands::ElapsedTime);
    m_commands.insert("AST", ISCPCommands::PlayStatus);

    // Misc
    m_commands.insert("CEC", ISCPCommands::CEC);
    m_commands.insert("HAO", ISCPCommands::HDMIAudio);
    m_commands.insert("HAS", ISCPCommands::HDMISubAudio);

    // Triggers
    m_commands.insert("TGA", ISCPCommands::TriggerA);
    m_commands.insert("TGB", ISCPCommands::TriggerB);
    m_commands.insert("TGC", ISCPCommands::TriggerC);

    // Information
    m_commands.insert("IFV", ISCPCommands::VideoInformation);
    m_commands.insert("IFA", ISCPCommands::AudioInformation);

    // Various audio mode settings
    m_commands.insert("LMD", ISCPCommands::ListeningMode);
    m_commands.insert("LTN", ISCPCommands::LateNightMode);
    m_commands.insert("MOT", ISCPCommands::MusicOptimizer);
    m_commands.insert("PMB", ISCPCommands::PhaseMatchingBass);
    m_commands.insert("ADY", ISCPCommands::Audyssey2EQ);
    m_commands.insert("ADQ", ISCPCommands::AudysseyDynamicEQ);
    m_commands.insert("ADV", ISCPCommands::AudysseyDynamicVolume);

    // Zone 2
    m_commands.insert("ZPW", ISCPCommands::Zone2Power);
    m_commands.insert("ZVL", ISCPCommands::Zone2Volume);
    m_commands.insert("ZMT", ISCPCommands::Zone2Mute);
    m_commands.insert("SLZ", ISCPCommands::Zone2Input);
    m_commands.insert("ZTN", ISCPCommands::Zone2Tone);

    // Zone 3
    m_commands.insert("PW3", ISCPCommands::Zone3Power);
    m_commands.insert("VL3", ISCPCommands::Zone3Volume);
    m_commands.insert("MT3", ISCPCommands::Zone3Mute);
    m_commands.insert("SL3", ISCPCommands::Zone3Input);

    // Zone 4
    m_commands.insert("PW4", ISCPCommands::Zone4Power);
    m_commands.insert("VL4", ISCPCommands::Zone4Volume);
    m_commands.insert("MT4", ISCPCommands::Zone4Mute);
    m_commands.insert("SL4", ISCPCommands::Zone4Input);

    // Input ID to Input name mapping
    // Video
    m_inputs.insert(qiscpInputs::Video1, "Video 1");
    m_inputs.insert(qiscpInputs::Video2, "Video 2");
    m_inputs.insert(qiscpInputs::Video3, "Video 3");
    m_inputs.insert(qiscpInputs::Video4, "Video 4");
    m_inputs.insert(qiscpInputs::Video5, "Video 5");
    m_inputs.insert(qiscpInputs::Video6, "Video 6");
    m_inputs.insert(qiscpInputs::Video7, "Video 7");

    // Extras
    m_inputs.insert(qiscpInputs::Extra1, "Extra 1");
    m_inputs.insert(qiscpInputs::Extra2, "Extra 2");
    m_inputs.insert(qiscpInputs::Extra3, "Extra 3");

    // DVD
    m_inputs.insert(qiscpInputs::DVD, "BD/DVD");

    // Tape
    m_inputs.insert(qiscpInputs::Tape1, "Tape 1");
    m_inputs.insert(qiscpInputs::Tape2, "Tape 2");

    m_inputs.insert(qiscpInputs::CD, "TV/CD");
    m_inputs.insert(qiscpInputs::Phono, "Phono");

    // Blueooth
    m_inputs.insert(qiscpInputs::Bluetooth, "Bluetooth");

    // Radio
    m_inputs.insert(qiscpInputs::FM, "FM");
    m_inputs.insert(qiscpInputs::AM, "AM");
    m_inputs.insert(qiscpInputs::Tuner, "Tuner");

    // Network
    m_inputs.insert(qiscpInputs::MusicServer, "Music Server");
    m_inputs.insert(qiscpInputs::InternetRadio, "Internet Radio");
    m_inputs.insert(qiscpInputs::USBFront, "USB Front");
    m_inputs.insert(qiscpInputs::USBBack, "USB Back");
    m_inputs.insert(qiscpInputs::Network, "Network");
    m_inputs.insert(qiscpInputs::AirPlay, "AirPlay");

    // Zone 1 as source
    m_inputs.insert(qiscpInputs::Source, "Source");

    // Setup discovery timeout timer
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(deviceDiscoveryTimeout()));
}

qiscp::~qiscp()
{
    close();
}

void qiscp::connectToHost() {    
    if (m_socket->isOpen()) {
        m_socket->abort();
    }
    m_socket->connectToHost(m_host, m_port);
}

/**
 * @brief qiscp::queueCommand
 * @param cmd
 * @param param
 */
void qiscp::queueCommand(const QString &cmd, const QString &param) {
    ISCPMsg *message=new ISCPMsg();
    message->setCommand(cmd, param);
    m_cmdqueue.append(message);
    if (!m_cmdtimer.isActive())
        m_cmdtimer.start();
}

bool qiscp::writeCommand(const QString &cmd, const QString &param) {
    ISCPMsg message;
    message.setCommand(cmd, param);

    return writeCommand(&message);
}

bool qiscp::writeCommand(const QString &cmd, const char *param)
{
    QString tmp(param);

    return writeCommand(cmd, tmp);
}

bool qiscp::writeCommand(const QString &cmd, bool param)
{
    return writeCommand(cmd, param ? "01" : "00");
}

bool qiscp::writeCommand(ISCPMsg *message) {
    if (!m_socket->isOpen()) {
        qWarning("Not connected");
        return false;
    }

    qDebug() << "WCMD: [" << message->getCommand() << "]:[" << message->getParamter() << "]";

    int r=m_socket->write(message->bytes());
    debugLogWrite(toNetwork,message);
    return r==-1 ? false : true;
}

void qiscp::handleCommandQueue() {
    if (m_cmdqueue.isEmpty()) {
        m_cmdtimer.stop();
        return;
    }

    ISCPMsg *msg=m_cmdqueue.takeFirst();
    writeCommand(msg);
    delete msg;
}

/**
 * @brief qiscp::discoverDevices
 *
 * Goes trough all interfaces and all IPv4 address the host has and send the discovery UDP packet
 * to all broadcast addresses.
 *
 */
void qiscp::discoverHosts(bool clear) {
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    // Create the disovery ISCP message, "!xECNQSTN"
    ISCPMsg msg;
    msg.setCommand("ECN", "QSTN", 'x');
    QByteArray packet=msg.bytes();

    m_discovered=0;
    emit discoveredChanged(m_discovered);

    m_discovering=true;
    emit discoveringChanged();

    m_timer.setInterval(m_discoveryTimeout);
    m_timer.stop();
    if (clear)
        m_devices.clear();
    m_timer.start();

    for (int i = 0; i < ifaces.size(); i++) {
        QList<QNetworkAddressEntry> addrs = ifaces[i].addressEntries();

        for (int j = 0; j < addrs.size(); j++) {
            if (addrs[j].ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;

            QHostAddress broadcast=addrs[j].broadcast();

            if (broadcast.isNull())
                continue;

            m_broadcast->writeDatagram(packet, broadcast, ISCP_PORT);
        }
    }
}

void qiscp::discoverHostsCancel() {
    if (m_discovering==false)
        return;
    m_timer.stop();
    deviceDiscoveryTimeout();
}

void qiscp::deviceDiscoveryTimeout() {        
    emit devicesDiscovered();

    m_discovering=false;
    emit discoveringChanged();

    if (m_discovered>0)
        cacheDiscoveredHosts();
}

bool qiscp::close() {
    if (m_socket->isOpen()) {
        m_socket->disconnectFromHost();
        return true;
    }
    return false;
}

void qiscp::tcpConnected() {    
    m_buffer.clear();
    m_connected=true;
    emit connectedChanged();
    emit connectedToHost();
    requestInitialState();
}

void qiscp::tcpDisconnected() {    
    clearAllTrackInformation();
    m_cmdtimer.stop();
    m_cmdqueue.clear();
    m_buffer.clear();
    m_connected=false;
    emit connectedChanged();
    emit disconnectedFromHost();
}

void qiscp::tcpError(QAbstractSocket::SocketError se) {
    m_buffer.clear();
    qWarning() << "TCP Error:" << se;
    emit connectionError(se);
    close();
}

void qiscp::readISCP() {
    QByteArray tmp;
    QDataStream ds(m_socket);
    int ba;

    ds.setByteOrder(QDataStream::BigEndian);
    ba=m_socket->bytesAvailable();

    // Hmm, this is a bit messy, as we check in here AND in the message parser for the signature
    // Read the first 4 bytes and check for signature (XXX: Read 4 bytes and compare that instead?)
    if (m_buffer.size()==0 && ba>4) {
        quint8 s;

        ds >> s;
        if ((char)s!='I')
            return;

        ds >> s;
        if ((char)s!='S')
            return;

        ds >> s;
        if ((char)s!='C')
            return;

        ds >> s;
        if ((char)s!='P')
            return;

        m_buffer.append("ISCP");
        ba-=4;
    }

    // Read in the data
    tmp.resize(ba);
    ds.readRawData(tmp.data(), ba);
    m_buffer.append(tmp, ba);
    tmp.clear();


    // Parse messages as long as we have data to parse
    while (m_buffer.size()>ISCP_HEADER_SIZE) {
        ISCPMsg msg;
        if (msg.fromData(&m_buffer)) {
            parseMessage(&msg);
            debugLogWrite(fromNetwork, &msg);
        } else {
            return;
        }
    }
}

/**
 * @brief qiscp::readBroadcastDatagram
 *
 * Read UDP broadcast packets from device discovery.
 *
 */
void qiscp::readBroadcastDatagram()
{
    if (m_discovering==false) {
        qWarning("Got device discovery packet when discovery not active, slow device?");
        return;
    }
    while (m_broadcast->hasPendingDatagrams()) {
        QByteArray datagram;
        QHostAddress sender;
        quint16 senderPort;

        datagram.resize(m_broadcast->pendingDatagramSize());
        m_broadcast->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        // As the only message we will ever get is either the query message itself
        // or the answer from a device we can safely ignore any packget that is the
        // size of the query message.
        if (datagram.size()==26)
            continue;

        ISCPMsg msg;
        if (!msg.fromData(&datagram))
                continue;

        QString cmd=msg.getCommand();
        if (cmd!="ECN")
            continue;

        QString param=msg.getParamter();
        QStringList di=param.split("/");

        if (di.count()!=4) {
            qWarning("Invalid device information");
            continue;
        }

        QVariantMap device;
        QString mac=di.at(3).mid(0,12);
        device.insert("ip", sender.toString());
        device.insert("model", di.at(0));
        device.insert("port",  di.at(1).toInt());
        device.insert("destination", di.at(2));
        device.insert("mac", mac);

        m_devices.insert(mac, device);
        emit discoveredHost(mac, device);

        m_discovered++;
        emit discoveredChanged(m_discovered);
    }
}

void qiscp::cacheDiscoveredHosts() {
    QSettings s;
    QMapIterator<QString, QVariant> i(m_devices);

    s.beginGroup("Devices");
    while (i.hasNext()) {
         i.next();

         s.beginGroup(i.key());
         QVariantMap device=i.value().toMap();
         QMapIterator<QString, QVariant> d(device);
         while (d.hasNext()) {
             d.next();
             s.setValue(d.key(), d.value().toString());
         }
         s.endGroup();
    }
    s.endGroup();
    s.sync();
}

void qiscp::loadCachedHosts() {

}

QVariant qiscp::videoInfo() const
{
    return m_video_info;
}

QVariant qiscp::audioInfo() const
{
    return m_audio_info;
}

bool qiscp::saveArtwork(QString file) {
    return m_artworkParser.save(file);
}

/**
 * @brief qiscp::parseMessage
 * @param message
 *
 * Analyzes the given ISCPMsg commands and parameter and decodes/parses into appropriate structures
 *
 */
void qiscp::parseMessage(ISCPMsg *message) {
    QString cmd=message->getCommand();

    if (!m_commands.contains(cmd)) {
        qWarning() << "Unknown CMD: " << cmd;
        return;
    }

    int cmdid=m_commands.value(cmd);

    switch (cmdid) {
    case ISCPCommands::MasterPower:
        m_power=message->getBooleanValue();
        if (m_power)
            m_poweredZones|=Zone1;
        else
            m_poweredZones&=!Zone1;
        emit powerChanged();
        emit poweredZonesChanged(m_poweredZones);
        break;
    case ISCPCommands::MasterMute:
        m_masterMuted=message->getBooleanValue();
        emit masterMutedChanged();
        break;
    case ISCPCommands::MasterVolume:
        m_masterVolume=message->getIntValue();
        emit masterVolumeChanged();
        break;
    case ISCPCommands::MasterTone: {
        QString p=message->getParamter();
        m_bassLevel=p.mid(1,2).toInt(NULL, 16);
        m_trebleLevel=p.mid(4,2).toInt(NULL, 16);

        emit bassLevelChanged();
        emit trebleLevelChanged();
    }
        break;
    case ISCPCommands::CenterLevel:
        m_centerLevel=message->getIntValue();
        emit centerLevelChanged();
        break;
    case ISCPCommands::SubwooferLevel:
        m_subwooferLevel=message->getIntValue();
        emit subwooferLevelChanged();
        break;
    case ISCPCommands::MasterInput:
        m_masterInput=message->getIntValue();
        emit masterInputChanged();
        switch (m_masterInput) {
        case qiscpInputs::FM:
        case qiscpInputs::AM:
        case qiscpInputs::Tuner:
            queueCommand("PRS", "QSTN");
            queueCommand("TUN", "QSTN");
            break;
        case qiscpInputs::Network:
        case qiscpInputs::InternetRadio:
        case qiscpInputs::MusicServer:
        case qiscpInputs::USBBack:
        case qiscpInputs::USBFront:
            requestNetworkPlayState();
            break;
        }
        requestInformationState();
        break;
    case ISCPCommands::MasterTuner:
        m_masterTunerFreq=message->getTunerValue();
        emit masterTunerFreqChanged();
        break;
    case ISCPCommands::MasterTunerPreset:
        m_masterTunerPreset=message->getIntValue();
        emit masterTunerPresetChanged(m_masterTunerPreset);
        break;
    case ISCPCommands::Artwork:
        if (m_artworkParser.parseMessage(message) && m_artworkParser.complete()) {
            m_hasArtwork=m_artworkParser.complete();
            emit hasArtworkChanged(m_hasArtwork);
            emit currentArtworkChanged();
        }
        break;
    case ISCPCommands::MusicOptimizer:
        m_musicOptimizer=message->getBooleanValue();
        emit musicOptimizerChanged();
        break;
    case ISCPCommands::PhaseMatchingBass:
        m_phaseMatchingBass=message->getBooleanValue();
        emit phaseMatchingBassChanged(m_phaseMatchingBass);
        break;
    case ISCPCommands::CEC:
        m_cec=message->getBooleanValue();
        emit cecChanged();
        break;
    case ISCPCommands::HDMIAudio:
        m_hdmiAudio=message->getBooleanValue();
        emit hdmiAudioChanged();
        break;
    case ISCPCommands::HDMISubAudio:
        m_hdmiSubAudio=message->getBooleanValue();
        emit hdmiSubAudioChanged();
        break;
    case ISCPCommands::ListeningMode:
        m_listeningmode=message->getIntValue();
        emit listeningModeChanged();
        break;
    case ISCPCommands::LateNightMode:
        m_latenight=message->getIntValue();
        emit lateNightModeChanged();
        break;
    case ISCPCommands::SleepTimer:
        m_sleepTimer=message->getIntValue();
        emit sleepTimerChanged(m_sleepTimer);
        break;
// Information
    case ISCPCommands::AudioInformation:
        m_audio_info=message->getListValue(",");
        qDebug() << "AudioInformation: " << m_audio_info;
        emit audioInfoChanged(m_audio_info);
        break;
    case ISCPCommands::VideoInformation:        
        m_video_info=message->getListValue(",");
        qDebug() << "VideoInformation: " << m_video_info;
        emit videoInfoChanged(m_video_info);
        break;
// Zone 2
    case ISCPCommands::Zone2Power:
        m_z2Power=message->getBooleanValue();
        emit zone2PowerChanged();
        if (m_z2Power==true) {
            requestZone2State();
        }

        if (m_z2Power)
            m_poweredZones|=Zone2;
        else
            m_poweredZones&=~Zone2;

        emit powerChanged();
        emit poweredZonesChanged(m_poweredZones);

        break;
    case ISCPCommands::Zone2Mute:
        m_z2Muted=message->getBooleanValue();
        emit zone2MutedChanged();
        break;
    case ISCPCommands::Zone2Volume:
        m_z2Volume=message->getIntValue();
        emit zone2VolumeChanged();
        break;
    case ISCPCommands::Zone2Input:
        m_z2Input=message->getIntValue();
        emit zone2InputChanged();
        switch (m_z2Input) {
        case qiscpInputs::FM:
        case qiscpInputs::AM:
        case qiscpInputs::Tuner:
            queueCommand("PRZ", "QSTN");
            queueCommand("TUZ", "QSTN");
            break;
        case qiscpInputs::Network:
        case qiscpInputs::InternetRadio:
        case qiscpInputs::MusicServer:
        case qiscpInputs::USBBack:
        case qiscpInputs::USBFront:
            requestNetworkPlayState();
            break;
        }
        break;
    case ISCPCommands::Zone2Tone: {
        QString p=message->getParamter();
        m_z2Bass=p.mid(1,2).toInt(NULL, 16);
        m_z2Treble=p.mid(4,2).toInt(NULL, 16);

        emit zone2BassLevelChanged();
        emit zone2TrebleLevelChanged();
    }
// Zone 3
    case ISCPCommands::Zone3Power:
        m_z3Power=message->getBooleanValue();

        if (m_z3Power==true) {
            requestZone3State();
        }
        if (m_z3Power)
            m_poweredZones|=Zone3;
        else
            m_poweredZones&=~Zone3;
        emit zone3PowerChanged();
        emit poweredZonesChanged(m_poweredZones);
        break;
    case ISCPCommands::Zone3Mute:
        m_z3Muted=message->getBooleanValue();
        emit zone3MutedChanged();
        break;
    case ISCPCommands::Zone3Volume:
        m_z3Volume=message->getIntValue();
        emit zone3VolumeChanged();
        break;
    case ISCPCommands::Zone3Input:
        m_z3Input=message->getIntValue();
        emit zone3InputChanged();
        switch (m_z3Input) {
        case qiscpInputs::FM:
        case qiscpInputs::AM:
        case qiscpInputs::Tuner:
            queueCommand("PR3", "QSTN");
            queueCommand("TU3", "QSTN");
            break;
        case qiscpInputs::Network:
        case qiscpInputs::InternetRadio:
        case qiscpInputs::MusicServer:
        case qiscpInputs::USBBack:
        case qiscpInputs::USBFront:
            requestNetworkPlayState();
            break;
        }
        break;
    case ISCPCommands::Zone3Tone: {
        QString p=message->getParamter();
        m_z3Bass=p.mid(1,2).toInt(NULL, 16);
        m_z3Treble=p.mid(4,2).toInt(NULL, 16);

        emit zone3BassLevelChanged();
        emit zone3TrebleLevelChanged();
    }
// Zone 4
    case ISCPCommands::Zone4Power:
        m_z4Power=message->getBooleanValue();        
        if (m_z4Power==true) {
            requestZone4State();
        }
        if (m_z4Power)
            m_poweredZones|=Zone4;
        else
            m_poweredZones&=~Zone4;
        emit zone4PowerChanged();
        emit poweredZonesChanged(m_poweredZones);
        break;
    case ISCPCommands::Zone4Mute:
        m_z4Muted=message->getBooleanValue();
        emit zone4MutedChanged();
        break;
    case ISCPCommands::Zone4Volume:
        m_z4Volume=message->getIntValue();
        emit zone4VolumeChanged();
        break;
    case ISCPCommands::Zone4Input:
        m_z4Input=message->getIntValue();
        emit zone4InputChanged();
        switch (m_z4Input) {
        case qiscpInputs::FM:
        case qiscpInputs::AM:
        case qiscpInputs::Tuner:
            queueCommand("PR4", "QSTN");
            queueCommand("TU4", "QSTN");
            break;
        case qiscpInputs::Network:
        case qiscpInputs::InternetRadio:
        case qiscpInputs::MusicServer:
        case qiscpInputs::USBBack:
        case qiscpInputs::USBFront:
            requestNetworkPlayState();
            break;
        }
        break;
// Network information
    case ISCPCommands::CurrentArtist:
        m_artist=message->getParamter();
        emit currentArtistChanged();
        break;
    case ISCPCommands::CurrentAlbum:
        m_album=message->getParamter();
        emit currentAlbumChanged();
        break;
    case ISCPCommands::CurrentTitle:
        m_title=message->getParamter();
        emit currentTitleChanged();
        break;
    case ISCPCommands::ElapsedTime:
        parseElapsedTime(message->getParamter());
        break;
    case ISCPCommands::PlayStatus:
         parsePlayStatus(message->getParamter());
         // XXX: Is this ok?
         if (m_playMode==Stopped)
             clearCurrentTrack();
        break;
    case ISCPCommands::TrackInfo:
        parseTrackInfo(message->getParamter());
        break;
    case ISCPCommands::MenuStatus:
        qWarning() << "Unhandled MS : " << message->getParamter();
        break;
    case ISCPCommands::MenuList:
        parseMenuItem(message->getParamter());
        break;
    case ISCPCommands::MenuListTitle:
        qWarning() << "Unhandled MLT: " << message->getParamter();
        break;
    case ISCPCommands::DeviceStatus:        
        parseDeviceStatus(message->getParamter());
        break;
    case ISCPCommands::NetworkRadioPreset:
        m_networkRadioPreset=message->getIntValue();
        emit networkRadioPresetChanged(m_networkRadioPreset);
        break;
// Audyssey
    case ISCPCommands::Audyssey2EQ:
        m_audyssey2EQ = (Audyssey2EQ)message->getIntValue();
        emit audyssey2EQChanged(m_audyssey2EQ);
        break;
    case ISCPCommands::AudysseyDynamicEQ:
        m_audysseyDynamicEQ = (AudysseyDynamicEQ)message->getIntValue();
        emit audysseyDynamicEQChanged(m_audysseyDynamicEQ);
        break;
    case ISCPCommands::AudysseyDynamicVolume:
        m_audysseyDynamicVolume = (AudysseyDynamicVolume)message->getIntValue();
        emit audysseyDynamicVolumeChanged(m_audysseyDynamicVolume);
        break;
// Device information
    case ISCPCommands::DeviceInformation:
        parseDeviceInformation(message->getParamter());
        break;
    default:
        qWarning() << "Known command not handled: " << cmd;
    }
}

void qiscp::parseTrackInfo(QString data) {
    QStringList tis=data.split("/");

    if (tis.size()<2) {
        qWarning("Invalid track info");
        setTracks(0);
        setTrack(0);
        return;
    }

    setTracks(tis.at(1).toInt(NULL, 10));
    setTrack(tis.at(0).toInt(NULL, 10));
}

void qiscp::parseMenuItem(QString data) {
    QChar type=data.at(0);
    QChar line=data.at(1);
    QChar param=data.at(2);
    QString msg=data.mid(3);

    qDebug() << "MI T:" << type << " L:" << line << " P:" << param;
    qDebug() << "MI D:" << msg;

    switch (type.toLatin1()) {
    case 'A':
    case 'U':
        m_menuitems.insert(line, msg);
        break;
    case 'C':
        if (param=='C') {
            m_menucursor=line.digitValue();
            qDebug() << "Curosor is at: " << m_menucursor;
        } else if (param=='P') {
            m_menuitems.clear();
        }
        break;
    default:
        qWarning("Unknown menu info type");
        break;
    }
}

void qiscp::parseDeviceInformation(QString data) {
    m_deviceinfoparser=new DeviceInforParser(data);

    if (m_deviceinfoparser->isOk()) {

        if (m_deviceinfoparser->isZoneAvailable(2))
            m_zonesAvailable|=Zone2;
        if (m_deviceinfoparser->isZoneAvailable(3))
            m_zonesAvailable|=Zone3;
        if (m_deviceinfoparser->isZoneAvailable(4))
            m_zonesAvailable|=Zone4;

        qDebug() << "*** Zones: " << m_zonesAvailable;

        m_tunerpresets=m_deviceinfoparser->getPresets();
        emit presetsList();

        m_networkservices=m_deviceinfoparser->getNetservices();
        emit networkList();

        m_zonesdata=m_deviceinfoparser->getZones();
        emit zonesList();

        m_inputsdata=m_deviceinfoparser->getSelectors();
        emit inputsList();
    } else {
        // XXX: signal that NRI wasn't available
    }

    delete m_deviceinfoparser;
}

void qiscp::parseDeviceStatus(QString data) {
    if (data.size()<3) {
        qWarning("Invalid device status");
        return;
    }

    const char n=data.at(0).toLatin1();
    const char f=data.at(1).toLatin1();
    const char r=data.at(2).toLatin1();

    // Network
    switch (n) {
    case '-':
        m_network_status=NoConnection;
        break;
    case 'E':
        m_network_status=Ethernet;
        break;
    case 'W':
        m_network_status=Wireless;
        break;
    }
    emit networkStatusChanged();

    // Front and Back USB
    m_usb_front=getUSBStatusEnum(f);
    emit usbFrontStatusChanged();

    m_usb_back=getUSBStatusEnum(r);
    emit usbBackStatusChanged();
}

qiscp::USBStatus qiscp::getUSBStatusEnum(const char u) {
    switch (u) {
    case '-':
        return NoDevice;
        break;
    case 'i':
        return iPodPhone;
        break;
    case 'M':
        return Memory;
        break;
    case 'W':
        return WirelessAdaptor;
        break;
    case 'B':
        return BluetoothAdaptor;
        break;
    case 'G':
        return GoogleUSB;
        break;
    default:
    case 'x':
        return Disabled;
        break;
    }
}

void qiscp::parsePlayStatus(QString data) {
    if (data.size()<3) {
        qWarning("Invalid play status");
        return;
    }

    const char ps=data.at(0).toLatin1();
    const char rs=data.at(1).toLatin1();
    const char ss=data.at(2).toLatin1();

    switch (ps) {
    case 'S':
        setPlayMode(Stopped);
        break;
    case 'P':
        setPlayMode(Playing);
        break;
    case 'p':
        setPlayMode(Paused);
        break;
    case 'F':
        setPlayMode(FastForwarding);
        break;
    case 'R':
        setPlayMode(FastReversing);
        break;
    case 'E':
        setPlayMode(EndOfFile);
        break;
    }

    switch (rs) {
    case '-':
        setRepeatMode(RepeatOff);
        break;
    case 'R':
        setRepeatMode(RepeatAll);
        break;
    case 'F':
        setRepeatMode(RepeatFolder);
        break;
    case '1':
        setRepeatMode(RepeatOne);
        break;
    case 'x':
        setRepeatMode(RepeatDisabled);
        break;
    }

    switch (ss) {
    case '-':
        setShuffleMode(ShuffleOff);
        break;
    case 'S':
        setShuffleMode(ShuffleAll);
        break;
    case 'A':
        setShuffleMode(ShuffleAlbum);
        break;
    case 'F':
        setShuffleMode(ShuffleFolder);
        break;
    case 'x':
        setShuffleMode(ShuffleDisabled);
        break;
    }

}

void qiscp::setTracks(quint16 tracks)
{
    if (m_tracks == tracks)
        return;

    m_tracks = tracks;
    emit currentTracksChanged(tracks);
}

void qiscp::setTrack(quint16 track)
{
    if (m_track == track)
        return;

    m_track = track;
    emit currentTrackChanged(track);
}

void qiscp::parseElapsedTime(QString et) {
    QStringList tmp=et.split("/");

    if (tmp.size()!=2) {
        qWarning("Invalid time stamp");
        return;
    }

    QTime tmptime;

    tmptime=QTime::fromString(tmp.at(1), "mm:ss");
    if (tmptime!=m_length) {
        m_length=tmptime;
        emit currentTrackLengthChanged();
    }

    tmptime=QTime::fromString(tmp.at(0), "mm:ss");
    if (tmptime!=m_position) {
        m_position=tmptime;
        emit currentTrackPositionChanged();
    }
}

/**
 * @brief qiscp::seekTo
 * @param position
 */
void qiscp::seekTo(int position)
{
    // mm limit is 00-99
    // ss limit is 00-59
    if (position>99*60+59)
        return;

    writeCommand("NTS", getElapsedTimeString(position));
}

/**
 * @brief qiscp::getElapsedTimeString
 * @param seconds
 * @return
 */
const QString qiscp::getElapsedTimeString(int seconds) const {
    int m=seconds/60;
    int s=seconds-m*60;

    return QString("%1:%2").arg(m,2, 10,  QLatin1Char('0')).arg(s, 2, 10,  QLatin1Char('0'));
}

/**
 * @brief qiscp::clearCurrentTrack
 *
 * Clears the information of the current track only
 *
 */
void qiscp::clearCurrentTrack() {
    m_position=QTime();
    m_length=QTime();
    m_artist.clear();
    m_album.clear();
    m_title.clear();
    m_hasArtwork=false;
    emit hasArtworkChanged(m_hasArtwork);
    emit currentTrackPositionChanged();
    emit currentTrackLengthChanged();
    emit currentTitleChanged();
    emit currentArtistChanged();
    emit currentAlbumChanged();    
}

/**
 * @brief qiscp::clearCurrentTrack
 *
 * Clears the all track information
 *
 */
void qiscp::clearAllTrackInformation() {
    setTracks(0);
    setTrack(0);
    setPlayMode(Stopped);
    setRepeatMode(RepeatOff);
    setShuffleMode(ShuffleOff);
}

/**
 * @brief qiscp::getDevices
 * @return a list of discovered devices on the network
 */
QVariantList qiscp::getDevices() const {
    QVariantList devices;
    QMapIterator<QString, QVariant> i(m_devices);
    while (i.hasNext()) {
        i.next();
        devices << i.value();
    }
    return devices;
}

/**
 * @brief qiscp::getInputs
 * @return
 */
QVariantList qiscp::getStaticInputs() const {
    QVariantList inputs;
    QMapIterator<int, QString> i(m_inputs);
    while (i.hasNext()) {
        QVariantMap input;
        i.next();

        input.insert("input_id", i.key());
        input.insert("name", i.value());

        inputs << input;
    }
    return inputs;
}

/**
 * @brief qiscp::getZones
 * @return
 *
 * Get list of Zones the connected device supports.
 *
 */
QVariantList qiscp::getZones() const {
    return m_zonesdata;
}

/**
 * @brief qiscp::getInputs
 * @return
 *
 * Get list of Inputs the connected device supports.
 *
 */
QVariantList qiscp::getInputs() const {
    return m_inputsdata;
}

/**
 * @brief qiscp::getNetworkSources
 * @return
 */
QVariantList qiscp::getNetworkSources() const {
    return m_networkservices;
}

/**
 * @brief qiscp::getControls
 * @return
 *
 * Get list of Audio controls the connected device supports.
 *
 */
QVariantList qiscp::getControls() const {
    return m_controls;
}

/*****************************************************************************/

/**
 * @brief qiscp::requestInitialState
 *
 * Called when connected is made, to get the current state of the receiver.
 *
 */
void qiscp::requestInitialState() {
    qDebug("*** Requesting initial state");
    queueCommand("PWR", "QSTN");
    queueCommand("MVL", "QSTN");
    queueCommand("AMT", "QSTN");
    queueCommand("SLP", "QSTN");
    queueCommand("SLI", "QSTN");
    queueCommand("NRI", "QSTN");
    queueCommand("TFR", "QSTN");
    queueCommand("CTL", "QSTN");
    queueCommand("SWL", "QSTN");
    // XXX: We should do this only if device supports the zones, but for now be stupid
    queueCommand("ZPW", "QSTN");
    // XXX: Funny, my device (master+z2 only) answers to PW3 with 00 but to PW4 with N/A so not reliable to probe it
    queueCommand("PW3", "QSTN");
    queueCommand("PW4", "QSTN");

    //
    queueCommand("CEC", "QSTN");
    queueCommand("MOT", "QSTN");
    queueCommand("PMB", "QSTN");
    queueCommand("LTN", "QSTN");
    queueCommand("LMD", "QSTN");
    queueCommand("HAO", "QSTN");
    // queueCommand("HAS", "QSTN");
    // queueCommand("HDO", "QSTN");
    // queueCommand("RAS", "QSTN");

    // Audyssey
    queueCommand("ADY", "QSTN");
    queueCommand("ADQ", "QSTN");
    queueCommand("ADV", "QSTN");
}

void qiscp::requestZone2State() {
    qDebug("*** Requesting Zone 2 state");
    queueCommand("ZVL", "QSTN");
    queueCommand("ZMT", "QSTN");
    queueCommand("ZTN", "QSTN");
    queueCommand("ZBL", "QSTN");
    queueCommand("SLZ", "QSTN");
}

void qiscp::requestZone3State() {
    qDebug("*** Requesting Zone 3 state");
    queueCommand("VL3", "QSTN");
    queueCommand("MT3", "QSTN");
    queueCommand("TN3", "QSTN");
    queueCommand("BL3", "QSTN");
    queueCommand("SL3", "QSTN");
}

void qiscp::requestZone4State() {
    qDebug("*** Requesting Zone 4 state");
    queueCommand("VL4", "QSTN");
    queueCommand("MT4", "QSTN");
    queueCommand("TN4", "QSTN");
    queueCommand("BL4", "QSTN");
    queueCommand("SL4", "QSTN");
}

void qiscp::requestNetworkPlayState() {
    qDebug("*** Requesting Network playback state");
    queueCommand("NST", "QSTN"); // Play Status
    queueCommand("NAT", "QSTN"); // Artist
    queueCommand("NAL", "QSTN"); // Album
    queueCommand("NTI", "QSTN"); // Title
    queueCommand("NTR", "QSTN"); // Track
    queueCommand("NJA", "QSTN"); // Artwork
    queueCommand("NDS", "QSTN"); // Device status
}

void qiscp::requestInformationState() {
    queueCommand("IFA", "QSTN");
    queueCommand("IFV", "QSTN");
}

/*************************************************************/

/**
 * @brief qiscp::debugLog
 * @param logfile
 * @param log
 * @return
 */
bool qiscp::debugLog(QString logfile, bool log)
{
    if (m_debuglog.isOpen() && log==false) {
        m_debuglog.flush();
        m_debuglog.close();
        return true;
    }

    m_debuglog.setFileName(logfile);
    if (log) {
        return m_debuglog.open(QIODevice::WriteOnly | QIODevice::Text);
    }
}

/**
 * @brief qiscp::debugLogWrite
 * @param data
 */
void qiscp::debugLogWrite(DebugLogDirection direction, const ISCPMsg *data) {
    if (m_debuglog.isOpen()) {
        QTextStream out(&m_debuglog);
        out << (direction==fromNetwork ? "I>> " : "O<<@") << QTime::currentTime().toString() << "][" << data->getCommand() << "]:[" << data->getParamter() << "]\n";
    }
}

/*************************************************************/

void qiscp::setPower(bool p) {
    writeCommand("PWR", p);
}

void qiscp::setZone2Power(bool p) {
    writeCommand("ZPW", p);
}

void qiscp::setZone3Power(bool p) {
    writeCommand("PW3", p);
}

void qiscp::setZone4Power(bool p) {
    writeCommand("PW4", p);
}

void qiscp::setMasterMuted(bool m) {
    writeCommand("AMT", m);
}

void qiscp::setZone2Muted(bool m) {
    writeCommand("ZMT", m);
}

void qiscp::setZone3Muted(bool m) {
    writeCommand("MT3", m);
}

void qiscp::setZone4Muted(bool m) {
    writeCommand("MT4", m);
}

void qiscp::setZoneInput(Zones zone, int t) {
    if (!m_inputs.contains(t))
        return;

    switch (zone) {
    case Zone1:
        // Reset tuner information
        m_masterTunerFreq=0;
        emit masterTunerFreqChanged();
        writeCommand("SLI", getHex(t));
        break;
    case Zone2:
        writeCommand("SLZ", getHex(t));
        break;
    case Zone3:
        writeCommand("SL3", getHex(t));
        break;
    case Zone4:
        writeCommand("SL4", getHex(t));
        break;
    }
}

void qiscp::setZoneMuted(Zone zone, bool m)
{
    switch (zone) {
    case Zone1:
        writeCommand("AMT", m);
        break;
    case Zone2:
        writeCommand("ZMT", m);
        break;
    case Zone3:
        writeCommand("MT3", m);
        break;
    case Zone4:
        writeCommand("MT4", m);
        break;
    }
}

void qiscp::setMasterInput(int t) {
    setZoneInput(Zone1, t);
}

void qiscp::setZone2Input(int t) {
    setZoneInput(Zone2, t);
}

void qiscp::setZone3Input(int t) {
    setZoneInput(Zone3, t);
}

void qiscp::setZone4Input(int t) {
    setZoneInput(Zone4, t);
}

void qiscp::setSleepTimer(int t) {
    if (t<=0) {
        writeCommand("SLP", "OFF");
    } else {
        if (t>90)
            t=90;
        writeCommand("SLP", getHex(t));
    }
}

void qiscp::setMasterVolume(quint8 vol) {
    // Make sure we don't try to set a too large volume directly
    if (vol>m_maxvolume)
        vol=m_maxvolume;
    writeCommand("MVL", getHex(vol));
}

void qiscp::setZoneVolume(Zones zone, quint8 vol) {
    // Make sure we don't try to set a too large volume directly
    if (vol>m_maxvolume)
        vol=m_maxvolume;
    switch (zone) {
    case Zone1:
        writeCommand("MVL", getHex(vol));
    break;
    case Zone2:
        writeCommand("ZVL", getHex(vol));
        break;
    case Zone3:
        writeCommand("VL3", getHex(vol));
        break;
    case Zone4:
        writeCommand("VL4", getHex(vol));
        break;
    }
}

void qiscp::setMaxDirectVolume(quint8 maxvol) {
    if (maxvol<1)
        maxvol=1;
    else if (maxvol>20)
        maxvol=20;

    if (m_maxvolume!=maxvol) {
        m_maxvolume=maxvol;
        emit maxDirectVolumeChanged();
    }
}

/**
 * @brief qiscp::volumeUp
 *
 * Master volume up
 *
 */
void qiscp::volumeUp(Zones zone) {
    switch (zone) {
    case Zone1:
        writeCommand("MVL", "UP");
    break;
    case Zone2:
        writeCommand("ZVL", "UP");
        break;
    case Zone3:
        writeCommand("VL3", "UP");
        break;
    case Zone4:
        writeCommand("VL4", "UP");
        break;
    }
}

/**
 * @brief qiscp::volumeDown
 *
 * Master volume down
 */
void qiscp::volumeDown(Zones zone) {
    switch (zone) {
    case Zone1:
        if (m_masterVolume==0)
            return;
        writeCommand("MVL", "DOWN");
        break;
    case Zone2:
        writeCommand("ZVL", "DOWN");
        break;
    case Zone3:
        writeCommand("VL3", "DOWN");
        break;
    case Zone4:
        writeCommand("VL4", "DOWN");
        break;
    }
}

QVariantList qiscp::getPresets() const {
    return m_tunerpresets;
}

void qiscp::tunerDisplayRDSToggle() {
    writeCommand("RDS", "UP");
}

void qiscp::presetRefresh()
{
    writeCommand("NRI", "QSTN");
}

void qiscp::tune(int t, Zones zone) {
    if (m_masterInput==qiscpInputs::FM) {
        // XXX: Check limits!
        if (t>10800)
            t=10800;
        if (t<8700)
            t=8700;
    } else if (m_masterInput==qiscpInputs::AM) {
        // XXX: Add limit check
    } else if (m_masterInput==qiscpInputs::Tuner) {
        // XXX: Add limit check
    } else {
        qWarning("Trying to tune when not radio input");
        return;
    }

    switch (zone) {
    case Zone1:
        writeCommand("TUN", getPaddedInt(t, 5));
        break;
    case Zone2:
        writeCommand("TUZ", getPaddedInt(t, 5));
        break;
    case Zone3:
        writeCommand("TU3", getPaddedInt(t, 5));
        break;
    case Zone4:
        writeCommand("TU4", getPaddedInt(t, 5));
        break;
    }
}

/**
 * @brief qiscp::tunePreset
 * @param t
 * @param zone
 *
 * Set AM/FM tuner to given preset memory location on given zone.
 *
 */
void qiscp::tunePreset(int t, Zones zone) {
    QString thex=getHex(t);
    switch (zone) {
    case Zone1:
        writeCommand("PRS", thex);
        break;
    case Zone2:
        writeCommand("PRZ", thex);
        break;
    case Zone3:
        writeCommand("PR3", thex);
        break;
    case Zone4:
        writeCommand("PR4", thex);
        break;
    }
}

/**
 * @brief qiscp::tuneStorePreset
 * @param ml
 *
 * Store currently tuned channel at memory location ml
 *
 */
bool qiscp::tuneStorePreset(int ml) {
    if (ml<1)
        return false;
    if (ml>40) // XXX: Somehow handle devices supporting only 30 locs
        return false;
    writeCommand("PRM", getHex(ml));
    return true;
}

/**
 * @brief qiscp::tuneUp
 * @param zone
 *
 * Autotune AM/FM tuner upwards on given zone
 *
 */
void qiscp::tuneUp(Zones zone) {
    switch (zone) {
    case Zone1:
        writeCommand("TUN", "UP");
        break;
    case Zone2:
        writeCommand("TUZ", "UP");
        break;
    case Zone3:
        writeCommand("TU3", "UP");
        break;
    case Zone4:
        writeCommand("TU4", "UP");
        break;
    }
}

/**
 * @brief qiscp::tuneDown
 * @param zone
 *
 * Autotune AM/FM tuner downwards on given zone
 *
 */
void qiscp::tuneDown(Zones zone) {
    switch (zone) {
    case Zone1:
        writeCommand("TUN", "DOWN");
        break;
    case Zone2:
        writeCommand("TUZ", "DOWN");
        break;
    case Zone3:
        writeCommand("TU3", "DOWN");
        break;
    case Zone4:
        writeCommand("TU4", "DOWN");
        break;
    }
}

void qiscp::presetUp(Zones zone) {
    switch (zone) {
    case Zone1:
        writeCommand("PRS", "UP");
        break;
    case Zone2:
        writeCommand("PRZ", "UP");
        break;
    case Zone3:
        writeCommand("PR3", "UP");
        break;
    case Zone4:
        writeCommand("PR4", "UP");
        break;
    }
}

void qiscp::presetDown(Zones zone) {
    switch (zone) {
    case Zone1:
        writeCommand("PRS", "DOWN");
        break;
    case Zone2:
        writeCommand("PRZ", "DOWN");
        break;
    case Zone3:
        writeCommand("PR3", "DOWN");
        break;
    case Zone4:
        writeCommand("PR4", "DOWN");
        break;
    }
}

void qiscp::bassLevelUp(Zones zone) {
    switch (zone) {
    case Zone1:
        writeCommand("TFR", "BUP");
        break;
    case Zone2:
        writeCommand("ZTN", "BUP");
        break;
    case Zone3:
        writeCommand("TN3", "BUP");
        break;
    case Zone4:
        qWarning("Zone 4 does not support tone control");
        break;
    }
}

void qiscp::bassLevelDown(Zones zone) {
    switch (zone) {
    case Zone1:
        writeCommand("TFR", "BDOWN");
        break;
    case Zone2:
        writeCommand("ZTN", "BDOWN");
        break;
    case Zone3:
        writeCommand("TN3", "BDOWN");
        break;
    case Zone4:
        qWarning("Zone 4 does not support tone control");
        break;
    }
}

void qiscp::trebleLevelUp(Zones zone) {
    switch (zone) {
    case Zone1:
        writeCommand("TFR", "TUP");
        break;
    case Zone2:
        writeCommand("ZTN", "TUP");
        break;
    case Zone3:
        writeCommand("TN3", "TUP");
        break;
    case Zone4:
        qWarning("Zone 4 does not support tone control");
        break;
    }
}

void qiscp::trebleLevelDown(Zones zone) {
    switch (zone) {
    case Zone1:
        writeCommand("TFR", "TDOWN");
        break;
    case Zone2:
        writeCommand("ZTN", "TDOWN");
        break;
    case Zone3:
        writeCommand("TN3", "TDOWN");
        break;
    case Zone4:
        qWarning("Zone 4 does not support tone control");
        break;
    }
}

/**
 * @brief qiscp::subwooferLevelUp
 *
 * Adjust master Subwoofer volume up
 *
 */
void qiscp::subwooferLevelUp() {
    writeCommand("SWL", "UP");
}


/**
 * @brief qiscp::subwooferLevelDown
 *
 * Adjust master Subwoofer volume down
 *
 */
void qiscp::subwooferLevelDown() {
    writeCommand("SWL", "DOWN");
}

/**
 * @brief qiscp::centerLevelUp
 *
 * Adjust master Center speaker volume up
 *
 */
void qiscp::centerLevelUp() {
    writeCommand("CTL", "UP");
}

/**
 * @brief qiscp::centerLevelDown
 *
 * Adjust master Center speak volume down
 *
 */
void qiscp::centerLevelDown() {
    writeCommand("CTL", "DOWN");
}

/**
 * @brief qiscp::setCEC
 * @param m
 *
 * Enable/Disable CEC device control trough HDMI
 *
 */
void qiscp::setCEC(bool m) {
    writeCommand("CEC", m);
}

void qiscp::setECO(EcoMode m) {
    writeCommand("ECO", m);
}

/**
 * @brief qiscp::setHDMIAudio
 * @param m
 *
 * Toggle Audio trough HDMI On/Off
 *
 */
void qiscp::setHDMIAudio(bool m) {
    writeCommand("HAO", m);
}

void qiscp::setHDMISubAudio(bool m) {
    writeCommand("HAS", m);
}

/**
 * @brief qiscp::setMusicOptimizer
 * @param m
 *
 * Toggle Compressed Music Optimizer On/Off
 *
 */
void qiscp::setMusicOptimizer(bool m)
{
    writeCommand("MOT", m);
}

/**
 * @brief qiscp::setPhaseMatcingBass
 * @param m
 */
void qiscp::setPhaseMatchingBass(bool m)
{
    writeCommand("PMB", m);
}
/**
 * @brief qiscp::setListeningMode
 * @param m
 *
 * Set listening mode
 *
 */
void qiscp::setListeningMode(ListeningModes m)
{
    writeCommand("LMD", getHex(m));
}

/**
 * @brief qiscp::setLateNightMode
 * @param m
 *
 * Set late night listening mode. Only available when input is xxx
 *
 */
void qiscp::setLateNightMode(LateNightModes m)
{
    writeCommand("LTN", getHex(m));
}

void qiscp::toggleAudyssey2EQ()
{
    writeCommand("ADY", "UP");
}

void qiscp::toggleAudysseyDynamicEQ()
{
    writeCommand("ADQ", "UP");
}

void qiscp::toggleAudysseyDynamicVolume()
{
    writeCommand("ADV", "UP");
}

void qiscp::toggleReEQ()
{
    writeCommand("RAS", "UP");
}

void qiscp::toggleCinemaFilter()
{
    writeCommand("RAS", "UP"); // Same command as ReEQ, depends on model what happens
}

void qiscp::toggleLateNightMode()
{
    writeCommand("LTN", "UP");
}

void qiscp::toggleListeningModeUp()
{
    writeCommand("LMD", "UP");
}

void qiscp::toggleListeningModeDown()
{
    writeCommand("LMD", "DOWN");
}

void qiscp::toggleListeningModeMusic()
{
    writeCommand("LMD", "MUSIC");
}

void qiscp::toggleListeningModeMovie()
{
    writeCommand("LMD", "MOVIE");
}

void qiscp::toggleListeningModeGame()
{
    writeCommand("LMD", "GAME");
}

/**
 * @brief qiscp::setBassLevel
 * @param level
 * @param zone
 *
 * Set bass level direclty. Range is from -10 to 10.
 *
 */
void qiscp::setBassLevel(qint8 level, Zones zone) {
    int l=qBound(-0xA, (int)level, 0xA);
    writeCommand("TFR", getHex(l));
}

/**
 * @brief qiscp::setTrebleLevel
 * @param level
 * @param zone
 *
 * Set treble level directly. Range is from -10 to 10.
 *
 */
void qiscp::setTrebleLevel(qint8 level, Zones zone) {
    int l=qBound(-0xA, (int)level, 0xA);
    writeCommand("TFR", getHex(l));
}

void qiscp::setCenterLevel(qint8 level) {
    int l=qBound(-0xC, (int)level, 0xC);
    writeCommand("CTL", getHex(l));
}

void qiscp::setSubwooferLevel(qint8 level) {
    int l=qBound(-0xF, (int)level, 0xC);
    writeCommand("SWL", getHex(l));
}

/**
 * @brief qiscp::bluetoothPairing
 *
 * Switch to Bluetooth input and enable device pairing.
 *
 */
void qiscp::bluetoothPairing() {
    writeCommand("NBT", "PAIRING");
}

/**
 * @brief qiscp::bluetoothClearPairing
 *
 * Clear bluetooth device pairing settings.
 *
 */
void qiscp::bluetoothClearPairing() {
    writeCommand("NBT", "CLEAR");
}

/**
 * @brief qiscp::baseCommand
 * @param c
 * @param cmd
 * @return
 *
 * These are the common shared commands for both DVD/BD and TV control trough RHID/HDMI/CEC
 *
 */
bool qiscp::baseCommand(QString c, Commands cmd) {
    switch (cmd) {
    case Power:
        writeCommand(c, "POWER");
        break;
    case PowerOn:
        writeCommand(c, "PWRON");
        break;
    case PowerOff:
        writeCommand(c, "PWROFF");
        break;
    case Play:
        writeCommand(c, "PLAY");
        break;
    case Stop:
        writeCommand(c, "STOP");
        break;
    case Pause:
        writeCommand(c, "PAUSE");
        break;
    case Enter:
        writeCommand(c, "ENTER");
        break;
    case Return:
        writeCommand(c, "RETURN");
        break;
    default:
        return false;
    }
    return true;
}

/**
 * @brief qiscp::keyCommand
 * @param c
 * @param cmd
 * @return true if valid command, false if not
 *
 * Send a key press command using given ISCP command.
 *
 */
bool qiscp::keyCommand(QString c, Commands cmd) {
switch (cmd) {
case Up:
    writeCommand(c, "UP");
    break;
case Down:
    writeCommand(c, "DOWN");
    break;
case Left:
    writeCommand(c, "LEFT");
    break;
case Right:
    writeCommand(c, "RIGHT");
    break;
case KeyA:
    writeCommand(c, "A");
    break;
case KeyB:
    writeCommand(c, "B");
    break;
case KeyC:
    writeCommand(c, "C");
    break;
case KeyD:
    writeCommand(c, "D");
    break;
case Key0:
    writeCommand(c, "0");
    break;
case Key1:
    writeCommand(c, "1");
    break;
case Key2:
    writeCommand(c, "2");
    break;
case Key3:
    writeCommand(c, "3");
    break;
case Key4:
    writeCommand(c, "4");
    break;
case Key5:
    writeCommand(c, "5");
    break;
case Key6:
    writeCommand(c, "6");
    break;
case Key7:
    writeCommand(c, "7");
    break;
case Key8:
    writeCommand(c, "8");
    break;
case Key9:
    writeCommand(c, "9");
    break;
default:
    return false;
}
return true;
}

/**
 * @brief qiscp::networkCommand
 * @param cmd
 *
 * Network service specific commands
 */
bool qiscp::networkCommand(Commands cmd) {
    const QString c="NTC";
    switch (cmd) {
    case Menu:
        writeCommand(c, "MENU");
        break;
    case TrackUp:
        writeCommand(c, "TRUP");
        break;
    case TrackDown:
        writeCommand(c, "TRDN");
        break;
    case Repeat:
        writeCommand(c, "REPEAT");
        break;
    case Random:
        writeCommand(c, "RANDOM");
        break;
    case Display:
        writeCommand(c, "DISPLAY");
        break;
    case Select:
        writeCommand(c, "SELECT");
        break;
    case FastForward:
        writeCommand(c, "FF");
        break;
    case FastReverse:
        writeCommand(c, "REW");
        break;
    default:
        if (keyCommand(c,cmd))
            return true;
        else if (baseCommand(c, cmd))
            return true;
        return false;
    }
    return true;
}

/**
 * @brief qiscp::tvCommand
 * @param cmd
 *
 * TV Control specific commands
 *
 */
bool qiscp::tvCommand(Commands cmd) {
    const QString c="CTV";

    switch (cmd) {
    case ChannelUp:
        writeCommand(c, "CHUP");
        break;
    case ChannelDown:
        writeCommand(c, "CHDN");
        break;
    case VolumeUp:
        writeCommand(c, "VLUP");
        break;
    case VolumeDown:
        writeCommand(c, "VLDN");
        break;
    case Mute:
        writeCommand(c, "MUTE");
        break;
    case Display:
        writeCommand(c, "DISP"); // different from network!
        break;
    case Input:
        writeCommand(c, "INPUT");
        break;
    case Clear:
        writeCommand(c, "CLEAR");
        break;
    case Setup:
        writeCommand(c, "SETUP");
        break;
    case Guide:
        writeCommand(c, "GUIDE");
        break;
    case Previous:
        writeCommand(c, "PREV");
        break;
    default:
        if (keyCommand(c,cmd))
            return true;
        else if (baseCommand(c, cmd))
            return true;
        return false;
    }
    return true;
}

bool qiscp::dvdCommand(Commands cmd) {
    const QString c="CDV";

    switch (cmd) {
    case FastForward:
        writeCommand(c, "FF");
        break;
    case FastReverse:
        writeCommand(c, "REW");
        break;
    case SkipForward:
        writeCommand(c, "SKIP.F");
        break;
    case SkipReverse:
        writeCommand(c, "SKIP.R");
        break;
    case Setup:
        writeCommand(c, "SETUP");
        break;
    case Menu:
        writeCommand(c, "MENU");
        break;
    case Top:
        writeCommand(c, "TOPMENU");
        break;
    case OpenClose:
        writeCommand(c, "OP/CL");
        break;
    case Angle:
        writeCommand(c, "ANGLE");
        break;
    case Display:
        writeCommand(c, "DISP"); // different from network!
        break;
    case Clear:
        writeCommand(c, "CLEAR");
        break;
    default:
        if (keyCommand(c,cmd))
            return true;
        else if (baseCommand(c, cmd))
            return true;
        return false;
    }
    return true;
}

/**
 * @brief qiscp::bdCommand
 * @param cmd
 *
 * Alias for dvdCommand();
 *
 */
bool qiscp::bdCommand(Commands cmd) {
    return dvdCommand(cmd);
}

/**
 * @brief qiscp::command
 * @param cmd
 *
 * Wrapper around the different commands, it maps them according to the currently selected input
 *
 */
bool qiscp::command(Commands cmd, Zones zone) {
switch (m_masterInput) {
case qiscpInputs::InternetRadio:
case qiscpInputs::Network:
case qiscpInputs::MusicServer:
case qiscpInputs::USBBack:
case qiscpInputs::USBFront:
    networkCommand(cmd);
    break;
case qiscpInputs::DVD:
    dvdCommand(cmd);
    break;
case qiscpInputs::CD: // CD/TV ?
    tvCommand(cmd);
    break;
}
}

void qiscp::setNetworkService(qiscp::NetworkService arg)
{
    QString p;
    m_networkService = arg;
    p=getHex(m_networkService, 2);
    writeCommand("NSV", p);
    emit networkServiceChanged(arg);
}

void qiscp::setDiscoveryTimeout(int arg)
{
    if (m_discoveryTimeout != arg && arg>=1000) {
        m_discoveryTimeout = arg;
        emit discoveryTimeoutChanged(arg);
    }
}

void qiscp::setPlayMode(qiscp::PlayModes arg)
{
    if (m_playMode == arg)
        return;

    m_playMode = arg;
    emit playModeChanged(arg);
}

void qiscp::setShuffleMode(qiscp::ShuffleModes arg)
{
    if (m_shuffleMode == arg)
        return;

    m_shuffleMode = arg;
    emit shuffleModeChanged(arg);
}

void qiscp::setRepeatMode(qiscp::RepeatModes arg)
{
    if (m_repeatMode == arg)
        return;

    m_repeatMode = arg;
    emit repeatModeChanged(arg);
}

void qiscp::setAudyssey2EQ(qiscp::Audyssey2EQ arg)
{
    writeCommand("ADY", getHex(arg));
}

void qiscp::setAudysseyDynamicEQ(qiscp::AudysseyDynamicEQ arg)
{
    writeCommand("ADQ", getHex(arg));
}

void qiscp::setAudysseyDynamicVolume(qiscp::AudysseyDynamicVolume arg)
{
    writeCommand("ADV", getHex(arg));
}
