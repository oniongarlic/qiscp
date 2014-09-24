#include <QDataStream>
#include <QDebug>
#include <QTime>
#include "iscpmsg.h"

#define ISCP_MAGIC "ISCP"

ISCPMsg::ISCPMsg(QObject *parent) :
    QObject(parent),
    m_version(1),
    m_destination('1')
{
}

bool ISCPMsg::setCommand(QString cmd, QString param, char destination) {
    if (cmd.size()>3)
        return false;
    m_cmd=cmd;
    m_param=param;
    m_destination=destination;

    return true;
}

void ISCPMsg::setParameter(QString param) {
    m_param=param;
}

/**
 * @brief ISCPMsg::toString
 */
QString ISCPMsg::toString() const {
    QString tmp;

    tmp.append(m_cmd);
    tmp.append(":");
    tmp.append(m_param);

    return tmp;
}

/**
 * @brief ISCPMsg::fromData
 * @param data
 * @return
 */
bool ISCPMsg::fromData(QByteArray *data) {
    qint32 i32, dsize;
    qint8 i8;
    char c8;

    if (data->startsWith(ISCP_MAGIC)==false) {
        qWarning("Invalid packet, signature is missing");
        return false;
    }

    QDataStream ds(data, QIODevice::ReadOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds.device()->seek(4);

    ds >> i32;
    if (i32!=ISCP_HEADER_SIZE) {
        qWarning() << "Invalid header size: " << i32;
        return false;
    }

    ds >> dsize;
    if (data->size()<dsize+ISCP_HEADER_SIZE) {
        qWarning("Invalid data size, larger than data packet");
        qDebug() << "Expected: " << data->size();
        qDebug() << "Got     : " << dsize+ISCP_HEADER_SIZE;
        return false;
    }

    // Version
    ds >> i8;
    if (i8!=0x01)
        return false;

    // Reserved padding
    ds >> i8;
    if (i8!=0x00)
        return false;
    ds >> i8;
    if (i8!=0x00)
        return false;
    ds >> i8;
    if (i8!=0x00)
        return false;

    // Read message data into m_data
    m_data.resize(dsize);
    ds.readRawData(m_data.data(), dsize);
    m_data.append((char)0);

    // Check data
    c8=m_data.at(0);
    if (c8!='!') {
        qWarning("Invalid message data");
        return false;
    }
    c8=m_data.at(1);
    if (c8!='1') {
        qWarning("Unknown device category");
        return false;
    }

    // Get the message command part into m_cmd
    QString msg=QString::fromUtf8(m_data.data(), dsize);
    m_cmd=msg.mid(2,3);

    // The rest depends on the command, read it into param, higher up can then decide what to do
    int eofidx=msg.indexOf(0x1A, 5);
    qDebug() << "EOF at: " << eofidx;
    m_param=msg.mid(5,eofidx-5);

    qDebug() << QTime::currentTime();
    qDebug() << "Datamsg sz: " << msg.size();
    qDebug() << "Command sz: " << dsize;
    qDebug() << "Command is: " << m_cmd;
    qDebug() << "Command pa: " << m_param;
    qDebug("-------------------------------------------------------");
    qDebug() << "Full ISCP Message is: " << msg;
    qDebug("-------------------------------------------------------");

    // Remove the data we used.
    data->remove(0, ISCP_HEADER_SIZE+dsize);

    return true;
}

QByteArray ISCPMsg::getData(void) const {
    return m_data;
}

QString ISCPMsg::getCommand() const {
    return m_cmd;
}

QString ISCPMsg::getParamter() const {
    return m_param;
}

/**
 * @brief ISCPMsg::getIntValue
 * @return Command paramter hex value converted to int
 */
int ISCPMsg::getIntValue() const {
    return getParamter().mid(0,2).toInt(NULL, 16);
}

/**
 * @brief ISCPMsg::getTunerValue
 * @return
 */
int ISCPMsg::getTunerValue() const {
    return getParamter().mid(0,5).toInt(NULL, 10);
}


/**
 * @brief Get the raw ISCP packet bytes
 * @return
 */
QByteArray& ISCPMsg::bytes(void) {
    m_data.clear();

    QByteArray rawcmd;
    rawcmd.append("!");
    rawcmd.append(m_destination);
    rawcmd.append(m_cmd.toLatin1());
    rawcmd.append(m_param.toLatin1());

    qint32 dsize=rawcmd.size()+1;

    QDataStream ds(&m_data, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds.writeRawData(ISCP_MAGIC, 4);
    ds << qint32(ISCP_HEADER_SIZE);
    ds << (qint32) dsize;
    ds << (qint8) 0x01;
    ds << (qint8) 0x00;
    ds << (qint8) 0x00;
    ds << (qint8) 0x00;
    ds.writeRawData(rawcmd.begin(), rawcmd.size());
    ds << (qint8) 0x0A;

    return m_data;
}
