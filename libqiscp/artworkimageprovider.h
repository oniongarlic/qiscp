#ifndef ARTWORKIMAGEPROVIDER_H
#define ARTWORKIMAGEPROVIDER_H

#include <QDeclarativeImageProvider>

class ArtworkImageProvider : public QDeclarativeImageProvider
{
    // Q_OBJECT
public:
    explicit ArtworkImageProvider();

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
signals:

public slots:

};

#endif // ARTWORKIMAGEPROVIDER_H
