#include "qtquick1applicationviewer.h"
#include <QApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include "artworkimageprovider.h"

#include "qiscp.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QtQuick1ApplicationViewer viewer;

    qmlRegisterType<qiscp>("org.tal.qiscp", 1, 0, "QISCP");
    qmlRegisterUncreatableType<qiscpInputs>("org.tal.qiscp", 1, 0, "ISCPInputs", "ISCPInputs can not be created");

    viewer.engine()->addImageProvider(QLatin1String("artwork"), new ArtworkImageProvider());

    viewer.addImportPath(QLatin1String("modules"));
    viewer.setOrientation(QtQuick1ApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/test/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
