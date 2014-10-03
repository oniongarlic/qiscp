#ifndef DEVICEINFORPARSER_H
#define DEVICEINFORPARSER_H

#include <QObject>
#include <QMap>
#include <QVariantList>
#include <QVariantMap>
#include <QXmlStreamReader>

class DeviceInforParser : public QObject
{
    Q_OBJECT
public:
    explicit DeviceInforParser(QString xml, QObject *parent = 0);

    Q_INVOKABLE bool isOk();
    Q_INVOKABLE QVariantMap getDevice() const;
    Q_INVOKABLE QVariantList getPresets() const;
    Q_INVOKABLE QVariantList getNetservices() const;
    Q_INVOKABLE QVariantList getZones() const;

signals:

public slots:

private:
    QXmlStreamReader m_xml;
    bool m_ok;

    bool readResponse();
    bool readDevice();
    bool readDeviceData();
    void readNetservices();
    void readZones();
    void readControls();
    void readSelectors();
    void readPresets();

    QVariantMap m_device;
    QVariantList m_presets;
    QVariantList m_zones;
    QVariantList m_controls;
    QVariantList m_selectors;
    QVariantList m_netservices;
};

#endif // DEVICEINFORPARSER_H
