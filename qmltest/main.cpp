#include "qtquick1applicationviewer.h"
#include <QApplication>
#include <QDeclarativeComponent>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>

#include "qiscp.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QtQuick1ApplicationViewer viewer;

    qmlRegisterType<qiscp>("org.tal.qiscp", 1, 0, "QISCP");

    viewer.addImportPath(QLatin1String("modules"));
    viewer.setOrientation(QtQuick1ApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/test/main.qml"));
    viewer.showExpanded();

    return app.exec();
}
