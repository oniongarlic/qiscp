#include "deviceinforparser.h"
#include <QDebug>

/**
 * @brief DeviceInforParser::DeviceInforParser
 * @param parent
 *
 * Parse the XML response to the ISCP NRI command
 * <response status="ok">
  <device id="TX-NR626">
    <brand>ONKYO</brand>
    <category>AV Receiver</category>
    <year>2013</year>
    <model>TX-NR626</model>
    <destination>xx</destination>
    <firmwareversion>1010-3110-0000-0100</firmwareversion>
    <netservicelist count="15">
    ...    <netservice id="11" value="1" name="" />
    </netservicelist>
    ...
 *
 */

DeviceInforParser::DeviceInforParser(QString xml, QObject *parent) :
    QObject(parent)
{
    m_xml.addData(xml);    
    m_ok=readResponse();
}

bool DeviceInforParser::isOk() {
    return m_ok;
}

QVariantMap DeviceInforParser::getDevice() const
{
    return m_device;
}

QVariantList DeviceInforParser::getPresets() const
{
    return m_presets;
}

QVariantList DeviceInforParser::getNetservices() const
{
    return m_netservices;
}

QVariantList DeviceInforParser::getZones() const
{
    return m_zones;
}

QVariantList DeviceInforParser::getSelectors() const
{
    return m_selectors;
}

QVariantList DeviceInforParser::getControls() const
{
    return m_controls;
}

bool DeviceInforParser::readResponse() {
    if (m_xml.readNextStartElement()) {
        if (m_xml.name() == "response" && m_xml.attributes().value("status")=="ok") {
            return readDevice();
        }
        m_xml.raiseError("Response not found");
    }

    return !m_xml.error();
}

bool DeviceInforParser::readDevice() {
    if (m_xml.readNextStartElement()) {
        if (m_xml.name() == "device") {
            QString device_id=m_xml.attributes().value("status").toString();
            m_device.insert("id", device_id);
            qDebug() << device_id;
            return readDeviceData();
        }
        m_xml.raiseError("Device not found");
    }

    return !m_xml.error();
}

bool DeviceInforParser::readDeviceData() {

    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == "device");

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == "brand")
            m_device.insert("brand", m_xml.readElementText());
        else if (m_xml.name() == "category")
            m_device.insert("category", m_xml.readElementText());
        else if (m_xml.name() == "year")
            m_device.insert("year", m_xml.readElementText());
        else if (m_xml.name() == "model")
            m_device.insert("model", m_xml.readElementText());
        else if (m_xml.name() == "destination")
            m_device.insert("destination", m_xml.readElementText());
        else if (m_xml.name() == "firmwareversion")
            m_device.insert("firmwareversion", m_xml.readElementText());
        else if (m_xml.name() == "netservicelist")
            readNetservices();
        else if (m_xml.name() == "zonelist")
            readZones();
        else if (m_xml.name() == "selectorlist")
            readSelectors();
        else if (m_xml.name() == "presetlist")
            readPresets();
        else if (m_xml.name() == "controllist")
            readControls();
        else {
            qWarning() << "Unexpected XML element in DeviceData: " << m_xml.name();
            m_xml.skipCurrentElement();
        }
    }
    return !m_xml.error();
}

void DeviceInforParser::readNetservices() {

    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == "netservicelist");

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == "netservice") {
            QString id=m_xml.attributes().value("id").toString();
            int value=m_xml.attributes().value("value").toString().toInt();
            QString name=m_xml.attributes().value("name").toString();

            qDebug() << "NS: " << id << ":" << value << ":" << name;

            QVariantMap ns;
            ns.insert("id", id);
            ns.insert("value", value);
            ns.insert("name", name);

            m_netservices << ns;

            m_xml.skipCurrentElement();
        } else {
            qWarning() << "Unexpected XML element in netservices: " << m_xml.name();
            m_xml.skipCurrentElement();
        }
    }
}

void DeviceInforParser::readZones() {
    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == "zonelist");

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == "zone") {
            QString id=m_xml.attributes().value("id").toString();
            int value=m_xml.attributes().value("value").toString().toInt();
            int volmax=m_xml.attributes().value("volmax").toString().toInt();
            int volstep=m_xml.attributes().value("volstep").toString().toInt();
            QString name=m_xml.attributes().value("name").toString();

            QVariantMap ns;
            ns.insert("id", id);
            ns.insert("value", value);
            ns.insert("volmax", volmax);
            ns.insert("volstep", volstep);
            ns.insert("name", name);

            m_zones << ns;

            qDebug() << "Z: " << id << ":" << value << ":" << name;
            m_xml.skipCurrentElement();
        } else {
            qWarning() << "Unexpected XML element in zonelist: " << m_xml.name();
            m_xml.skipCurrentElement();
        }
    }
}

void DeviceInforParser::readControls() {
    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == "controllist");

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == "control") {
            QString id=m_xml.attributes().value("id").toString();
            int value=m_xml.attributes().value("value").toString().toInt();
            int zone=m_xml.attributes().value("zone").toString().toInt();

            QVariantMap ns;
            ns.insert("id", id);
            ns.insert("value", value);
            ns.insert("zone", zone);

            m_controls << ns;

            qDebug() << "C: " << id << ":" << value << ":" << zone;
            m_xml.skipCurrentElement();
        } else {
            qWarning() << "Unexpected XML element in controllist: " << m_xml.name();
            m_xml.skipCurrentElement();
        }
    }
}

void DeviceInforParser::readSelectors() {
    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == "selectorlist");

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == "selector") {
            int id=m_xml.attributes().value("id").toString().toInt(NULL, 16);
            int value=m_xml.attributes().value("value").toString().toInt();
            QString name=m_xml.attributes().value("name").toString();

            QVariantMap ns;
            ns.insert("input_id", id);
            ns.insert("value", value);
            ns.insert("name", name);

            m_selectors << ns;

            qDebug() << "S: " << id << ":" << value << ":" << name;
            m_xml.skipCurrentElement();
        } else {
            qWarning() << "Unexpected XML element in selectorlist: " << m_xml.name();
            m_xml.skipCurrentElement();
        }
    }
    qDebug() << "INPUTS: " << m_selectors;
}

void DeviceInforParser::readPresets() {
    Q_ASSERT(m_xml.isStartElement() && m_xml.name() == "presetlist");

    while (m_xml.readNextStartElement()) {
        if (m_xml.name() == "preset") {
            QString id=m_xml.attributes().value("id").toString();
            QString band=m_xml.attributes().value("band").toString();
            QString freq=m_xml.attributes().value("freq").toString(); // Parse into a int ?
            QString name=m_xml.attributes().value("name").toString();

            QVariantMap ns;
            ns.insert("preset_id", id);
            ns.insert("band", band);
            ns.insert("freq", freq);
            ns.insert("name", name);

            m_presets << ns;

            qDebug() << "P: " << id << ":" << freq << ":" << name;
            m_xml.skipCurrentElement();
        } else {
            qWarning() << "Unexpected XML element in presetlist: " << m_xml.name();
            m_xml.skipCurrentElement();
        }
    }
}
