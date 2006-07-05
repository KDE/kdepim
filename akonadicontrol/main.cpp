
#include <QCoreApplication>
#include "akonadicontrol.h"

int main( int argc, char** argv ) {
    QCoreApplication app( argc, argv );
    AkonadiControl ac;

    return app.exec();
}

