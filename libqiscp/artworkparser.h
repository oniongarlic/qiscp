#ifndef ARTWORKPARSER_H
#define ARTWORKPARSER_H

#include <QObject>
#include <QImage>

#include "iscpmsg.h"

class ArtworkParser : public QObject
{
    Q_OBJECT
public:
    explicit ArtworkParser(QObject *parent = 0);
    ~ArtworkParser();

    bool parseMessage(ISCPMsg *message);
    const QImage *get() const;
    bool save(QString file);
    bool complete() const;

signals:

public slots:


private:
    QByteArray m_artbuffer;
    QImage *m_artwork;
    bool m_hasArtwork;
    bool m_complete;
    bool set(QByteArray data);
};

#endif // ARTWORKPARSER_H
