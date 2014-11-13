#include "artworkimageprovider.h"

ArtworkImageProvider::ArtworkImageProvider() :
    QDeclarativeImageProvider(QDeclarativeImageProvider::Image)
{
}

QImage ArtworkImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize) {
    int width = 128;
    int height = 128;

    if (size)
        *size = QSize(width, height);

    QImage image(
                requestedSize.width() > 0 ? requestedSize.width() : width,
                requestedSize.height() > 0 ? requestedSize.height() : height,
                QImage::Format_RGB32);
    image.fill(QColor(id).rgba());

    return image;
}
