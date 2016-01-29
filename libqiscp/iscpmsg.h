#ifndef ISCPMSG_H
#define ISCPMSG_H

#include <QObject>
#include <QByteArray>
#include <QVariantList>

#define ISCP_HEADER_SIZE (16)

class ISCPMsg : public QObject
{
    Q_OBJECT
public:
    explicit ISCPMsg(QObject *parent = 0);
    // explicit ISCPMsg(QString cmd, QString param, QObject *parent = 0);
    QString toString() const;

    bool setCommand(const QString &cmd, const QString &param, char destination='1');
    void setParameter(const QString &param);

    QString getCommand() const;
    QString getParamter() const;
    int getIntValue() const;
    int getTunerValue() const;

    QByteArray &bytes(void);
    bool fromData(QByteArray *data);

    QByteArray getData(void) const;

    bool getBooleanValue() const;
    QVariantList getListValue(const QString &sep) const;
signals:

public slots:

private:
    qint8 m_version;
    char m_destination;
    QString m_cmd;
    QString m_param;
    QByteArray m_data;
};

#endif // ISCPMSG_H
