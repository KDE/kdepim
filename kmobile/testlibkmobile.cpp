/*
 * Copyright (C) 2003 Helge Deller <deller@kde.org>
 */

#include <kapplication.h>
#include <dcopclient.h>
#include <qdatastream.h>
#include <qstring.h>
#include <kdebug.h>

int main(int argc, char **argv)
{
    bool ok;

    KApplication app(argc, argv, "kmobile_client", false);

    // get our DCOP client and attach so that we may use it
    DCOPClient *client = app.dcopClient();
    client->attach();

    // do a 'send' for now
    QByteArray data;
    QDataStream ds(data, IO_WriteOnly);
//     ds << QString("a");

    QCString replyType;
    QByteArray replyData;
    ok = client->call("kmobile", "kmobileIface", "deviceNames()", data, replyType, replyData);

    QDataStream reply(replyData, IO_ReadOnly); 
    QStringList ret; 
    reply >> ret;

    kdDebug() << QString("Ergebnis=%1\n").arg(ok);
    kdDebug() << QString("Ergebnis-Liste=%1\n").arg(ret[0]);

  //  return app.exec();
}
