#include "qiscp.h"
#include "iscpmsg.h"

#include <QNetworkAddressEntry>
#include <QStringList>

static QString getPaddedInt(int n, int padding=5) {
    QString s=QString("%1").arg(n, padding, 10, QChar('0')).toUpper();
    return s;
}

static QString getHex(int n, int padding=2) {
    QString s=QString("%1").arg(n, padding, 16, QChar('0')).toUpper();
    return s;
}

qiscp::qiscp(QObject *parent) :
    QObject(parent),
    m_port(ISCP_PORT),
    m_discover_timeout(1000),
    m_masterMuted(false),
    m_masterVolume(0),
    m_maxvolume(20),
    m_masterTunerFreq(0)
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

    m_cmdtimer.setInterval(50);
    connect(&m_cmdtimer, SIGNAL(timeout()), this, SLOT(handleCommandQueue()));

    // Command to ID mapping
    // Known commands
    m_commands.insert("PWR", ISCPCommands::MasterPower);
    m_commands.insert("MVL", ISCPCommands::MasterVolume);
    m_commands.insert("AMT", ISCPCommands::MasterMute);
    m_commands.insert("SLI", ISCPCommands::MasterInput);
    m_commands.insert("SLP", ISCPCommands::SleepTimer);
    m_commands.insert("TUN", ISCPCommands::MasterTuner);

    m_commands.insert("NAL", ISCPCommands::CurrentAlbum);
    m_commands.insert("NAT", ISCPCommands::CurrentArtist);
    m_commands.insert("NTI", ISCPCommands::CurrentTitle);
    m_commands.insert("NRT", ISCPCommands::CurrentTrack);
    m_commands.insert("NTM", ISCPCommands::ElapsedTime);
    m_commands.insert("NST", ISCPCommands::PlayStatus);
    m_commands.insert("NRI", ISCPCommands::DeviceInformation);

    // XXX: Needs feedback, but should work
    m_commands.insert("AAL", ISCPCommands::CurrentAlbum);
    m_commands.insert("AAT", ISCPCommands::CurrentArtist);
    m_commands.insert("ATI", ISCPCommands::CurrentTitle);
    m_commands.insert("ATM", ISCPCommands::ElapsedTime);
    m_commands.insert("AST", ISCPCommands::PlayStatus);

    // Input ID to Input name mapping
    // Video
    m_inputs.insert(Inputs::Video1, "Video 1");
    m_inputs.insert(Inputs::Video2, "Video 2");
    m_inputs.insert(Inputs::Video3, "Video 3");
    m_inputs.insert(Inputs::Video4, "Video 4");
    m_inputs.insert(Inputs::Video5, "Video 5");
    m_inputs.insert(Inputs::Video6, "Video 6");
    m_inputs.insert(Inputs::Video7, "Video 7");

    // Extras
    m_inputs.insert(Inputs::Extra1, "Extra 1");
    m_inputs.insert(Inputs::Extra2, "Extra 2");
    m_inputs.insert(Inputs::Extra3, "Extra 3");

    // DVD
    m_inputs.insert(Inputs::DVD, "DVD");

    // Tape
    m_inputs.insert(Inputs::Tape1, "Tape 1");
    m_inputs.insert(Inputs::Tape2, "Tape 2");

    m_inputs.insert(Inputs::CD, "CD");
    m_inputs.insert(Inputs::Phono, "Phono");

    // Radio
    m_inputs.insert(Inputs::FM, "FM");
    m_inputs.insert(Inputs::AM, "AM");
    m_inputs.insert(Inputs::Tuner, "Tuner");

    // Network
    m_inputs.insert(Inputs::MusicServer, "Music Server");
    m_inputs.insert(Inputs::InternetRadio, "Internet Radio");
    m_inputs.insert(Inputs::USBFront, "USB Front");
    m_inputs.insert(Inputs::USBBack, "USB Back");
    m_inputs.insert(Inputs::Network, "Network");

    m_timer.setSingleShot(true);
    m_timer.setInterval(m_discover_timeout);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(discoveryTimeout()));
}

void qiscp::connectToHost() {
    qDebug() << "Connecting to: " << m_host << ":" << m_port;
    if (m_socket->isOpen()) {
        m_socket->abort();
    }
    m_socket->connectToHost(m_host, m_port);
}

void qiscp::queueCommand(QString cmd, QString param) {
    ISCPMsg *message=new ISCPMsg();
    message->setCommand(cmd, param);
    m_cmdqueue.append(message);
    if (!m_cmdtimer.isActive())
        m_cmdtimer.start();
}

bool qiscp::writeCommand(QString cmd, QString param) {
    ISCPMsg message;
    message.setCommand(cmd, param);

    return writeCommand(&message);
}

bool qiscp::writeCommand(ISCPMsg *message) {
    if (!m_socket->isOpen()) {
        qWarning("Not connected");
        return false;
    }

    qDebug() << "WCMD: [" << message->getCommand() << "]:[" << message->getParamter() << "]";

    int r=m_socket->write(message->bytes());
    return r==-1 ? false : true;
}

void qiscp::handleCommandQueue() {
    if (m_cmdqueue.isEmpty()) {
        m_cmdtimer.stop();
        return;
    }

    ISCPMsg *msg=m_cmdqueue.takeLast();
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
void qiscp::discoverHosts() {
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    // Create the disovery ISCP message, "!xECNQSTN"
    ISCPMsg msg;
    msg.setCommand("ECN", "QSTN", 'x');
    QByteArray packet=msg.bytes();

    qDebug() << "Sending discovery packget:";
    qDebug() << "Contents: " << packet.toHex();

    m_timer.stop();
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

void qiscp::discoveryTimeout() {
    qDebug("Device discovery done");
    qDebug() << m_devices;
    emit devicesDiscovered();
}

QVariantList qiscp::getDevices() const {
    return m_devices;
}

bool qiscp::close() {
    if (m_socket->isOpen()) {
        m_socket->disconnectFromHost();
        return true;
    }
    return false;
}

void qiscp::readISCP() {
    QByteArray tmp;
    QDataStream ds(m_socket);
    int ba;

    ds.setByteOrder(QDataStream::BigEndian);
    ba=m_socket->bytesAvailable();

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

        qDebug("Got response signature");
        m_buffer.append("ISCP");
        ba-=4;
    }

    // Read in the data
    tmp.resize(ba);
    ds.readRawData(tmp.data(), ba);
    qDebug() << "DS: " << tmp.size();
    qDebug() << "DATA: " << tmp.toHex();
    m_buffer.append(tmp, ba);
    tmp.clear();

    if (m_buffer.size()>ISCP_HEADER_SIZE) {
        ISCPMsg msg;
        if (msg.fromData(&m_buffer)) {            
            parseMessage(&msg);
            m_buffer.clear();
        } else {
             //m_buffer.clear();
            return;
        }
    }
}

void qiscp::parseMessage(ISCPMsg *message) {
    QString cmd=message->getCommand();

    if (!m_commands.contains(cmd)) {
        qWarning() << "Unknown CMD: " << cmd;
        return;
    }

    int cmdid=m_commands.value(cmd);
    int val;

    switch (cmdid) {
    case ISCPCommands::MasterPower:
        val=message->getIntValue();
        m_power=val==1 ? true : false;
        emit powerChanged();
        break;
    case ISCPCommands::MasterMute:
        val=message->getIntValue();
        m_masterMuted=val==1 ? true : false;
        emit masterMutedChanged();
        break;
    case ISCPCommands::MasterVolume:
        m_masterVolume=message->getIntValue();
        emit masterVolumeChanged();
        break;
    case ISCPCommands::MasterInput:
        m_masterInput=message->getIntValue();
        emit masterInputChanged();
        switch (m_masterInput) {
        case Inputs::FM:
        case Inputs::AM:
        case Inputs::Tuner:
            writeCommand("TUN", "QSTN");
            break;
        case Inputs::InternetRadio:
        case Inputs::MusicServer:
        case Inputs::USBBack:
        case Inputs::USBFront:
            requestNetworkPlayState();
            break;
        }
        break;
    case ISCPCommands::MasterTuner:
        m_masterTunerFreq=message->getTunerValue();
        emit masterTunerFreqChanged();
        break;
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
    case ISCPCommands::ElapsedTime: {
        QStringList tmp=message->getParamter().split("/");

        if (tmp.size()!=2) {
            qWarning("Invalid time stamp");
            return;
        }

        QString pos=tmp.at(0);
        QString len=tmp.at(1);

        m_length=QTime::fromString(len, "mm:ss");
        emit currentTrackLengthChanged();

        m_position=QTime::fromString(pos, "mm:ss");
        emit currentTrackPositionChanged();
    }
        break;
    case ISCPCommands::DeviceInformation: {
        m_deviceinfoparser=new DeviceInforParser(message->getParamter());

        if (m_deviceinfoparser->isOk()) {
            m_tunerpresets=m_deviceinfoparser->getPresets();
            emit presetsList();

            m_networkservices=m_deviceinfoparser->getNetservices();
            emit networkList();
        } else {
            // XXX: signal that NRI wasn't available
        }

        delete m_deviceinfoparser;
    }
        break;
    default:
        qWarning("Unhandled CMD!");
    }
}

void qiscp::tcpConnected() {
    qDebug("Connected");
    m_buffer.clear();
    m_connected=true;    
    emit connectedChanged();
    emit connectedToHost();
    requestInitialState();
}

/**
 * @brief qiscp::requestInitialState
 *
 * Called when connection is succesfull to get the current state for:
 * Power
 * Volume
 * Input
 *
 */
void qiscp::requestInitialState() {
    queueCommand("PWR", "QSTN");
    queueCommand("MVL", "QSTN");
    queueCommand("AMT", "QSTN");
    queueCommand("SLP", "QSTN");
    queueCommand("SLI", "QSTN");
    queueCommand("NRI", "QSTN");
}

void qiscp::requestNetworkPlayState() {
    queueCommand("NAT", "QSTN");
    queueCommand("NAL", "QSTN");
    queueCommand("NTI", "QSTN");
    queueCommand("NTR", "QSTN");
}

void qiscp::tcpDisconnected() {
    qDebug("DisConnected");
    m_buffer.clear();
    m_connected=false;
    emit connectedChanged();
    emit disconnectedFromHost();
}

void qiscp::tcpError(QAbstractSocket::SocketError se) {
    m_buffer.clear();
    qWarning() << "TCP Error:" << se;
}

void qiscp::readBroadcastDatagram()
{
    while (m_broadcast->hasPendingDatagrams()) {
        QByteArray datagram;
        QHostAddress sender;
        quint16 senderPort;

        datagram.resize(m_broadcast->pendingDatagramSize());
        m_broadcast->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        qDebug() << "Got UDP data from: " << sender;
        qDebug() << "Contents: " << datagram.toHex();

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

        device.insert("ip", sender.toString());
        device.insert("model", di.at(0));
        device.insert("port",  di.at(1).toInt());
        device.insert("destination", di.at(2));
        device.insert("mac", di.at(3).mid(0,12));

        m_devices << device;
    }
}

/**
 * @brief qiscp::getInputs
 * @return
 */
QVariantList qiscp::getInputs() const {
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

void qiscp::setPower(bool p) {
    writeCommand("PWR", p==true ? "01" : "00");
}

void qiscp::setMasterMuted(bool m) {
    writeCommand("AMT", m==true ? "01" : "00");
}

void qiscp::setMasterInput(int t) {
    // Check that input code is a valid one, if not just ignore it
    if (!m_inputs.contains(t))
        return;

    // Reset tuner information
    m_masterTunerFreq=0;
    emit masterTunerFreqChanged();

    writeCommand("SLI", getHex(t));
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

void qiscp::volumeUp() {
    writeCommand("MVL", "UP");
}

void qiscp::volumeDown() {
    writeCommand("MVL", "DOWN");
}

QVariantList qiscp::getPresets() const {
    return m_tunerpresets;
}

void qiscp::tune(int t) {
    if (m_masterInput==Inputs::FM) {
        // XXX: Check limits!
        if (t>10800)
            t=10800;
        if (t<8700)
            t=8700;
    } else if (m_masterInput==Inputs::AM) {
        // XXX: Add limit check
    } else if (m_masterInput==Inputs::Tuner) {
        // XXX: Add limit check
    } else {
        qWarning("Trying to tune when not radio input");
        return;
    }

    writeCommand("TUN", getPaddedInt(t, 5));
}

void qiscp::tunePreset(int t) {
    writeCommand("PRS", getHex(t));
}

void qiscp::tuneUp() {
    writeCommand("TUN", "UP");
}

void qiscp::tuneDown() {
    writeCommand("TUN", "DOWN");
}

void qiscp::presetUp() {
    writeCommand("PRS", "UP");
}

void qiscp::presetDown() {
    writeCommand("PRS", "DOWN");
}

void qiscp::bluetoothPairing() {
    writeCommand("NBT", "PAIRING");
}

void qiscp::bluetoothClearPairing() {
    writeCommand("NBT", "CLEAR");
}

void qiscp::networkCommand(Commands cmd) {
int t=cmd;

switch (cmd) {
case Play:
    writeCommand("NTC", "PLAY");
    break;
case Stop:
    writeCommand("NTC", "STOP");
    break;
case Pause:
    writeCommand("NTC", "PAUSE");
    break;
}
}

void qiscp::tvCommand(Commands cmd) {

}

void qiscp::dvdCommand(Commands cmd) {

}

void qiscp::bdCommand(Commands cmd) {

}

void qiscp::command(Commands cmd) {
switch (m_masterInput) {
case Inputs::InternetRadio:
case Inputs::Network:
case Inputs::MusicServer:
    networkCommand(cmd);
    break;
case Inputs::DVD:
    dvdCommand(cmd);
    break;
}
}
