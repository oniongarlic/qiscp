#include <QCoreApplication>

#include "qiscp.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qiscp iscp;

    iscp.discoverHosts();

    return a.exec();
}
