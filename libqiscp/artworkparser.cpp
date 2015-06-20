#include "artworkparser.h"
#include "iscpmsg.h"

ArtworkParser::ArtworkParser(QObject *parent) :
    QObject(parent)
{
    m_artwork=new QImage();
}

ArtworkParser::~ArtworkParser()
{
    delete m_artwork;
}

bool ArtworkParser::parseMessage(ISCPMsg *message) {
    QString p=message->getParamter();
    int type=p.mid(0,1).toInt(NULL, 10);
    int marker=p.mid(1,1).toInt(NULL, 10);

    // XXX: Check that type is a valid value

    switch (marker) {
    case 0: // Start marker
        m_hasArtwork=false;
        m_complete=false;
        m_artbuffer.clear();
        m_artbuffer.append(p.mid(2));
        break;
    case 1: // Image data
        m_artbuffer.append(p.mid(2));
        break;
    case 2: // End of image data
        m_artbuffer.append(p.mid(2));
        set(QByteArray::fromHex(m_artbuffer));
        m_artbuffer.clear();
        m_complete=true;
        break;
    default:
        return false;
    }
    return true;
}

bool ArtworkParser::set(QByteArray data) {
    m_artwork=new QImage();

    m_hasArtwork=m_artwork->loadFromData(data);
    if (!m_hasArtwork) {
        qWarning("Failed to load artwork image data");
    }

    return m_hasArtwork;
}

const QImage *ArtworkParser::get() const
{
    return m_artwork;
}

bool ArtworkParser::save(QString file) {
    if (m_artwork->isNull())
        return false;

    return m_artwork->save(file, "PNG");
}

bool ArtworkParser::complete() const
{
    return m_complete;
}
