/*-
 * Copyright 2009 KDAB and Guillermo A. Amaral B. gamaral@amaral.com.mx
 */

#include <QApplication>

#include "noteserver.h"

int
main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    NoteServer ns;

    ns.listen(QHostAddress::LocalHost, 1234);

    return (app.exec());   
}

