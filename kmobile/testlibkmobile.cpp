/*
 * Test program for the KDE KMobile Library
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

    QByteArray data;
    QDataStream ds(data, IO_WriteOnly);
//     ds << QString("a");

    QCString replyType;
    QByteArray replyData;
    ok = client->call("kmobile", "kmobileIface", "deviceNames()", data, replyType, replyData);

    QDataStream reply(replyData, IO_ReadOnly); 
    QStringList deviceNames; 
    reply >> deviceNames;

    kdDebug() << QString("%1\n").arg(ok?"Ok":"Failure");
    kdDebug() << QString("Number of currently registered drivers: %1\n").arg(deviceNames.count());
    for (int i=0; i<deviceNames.count(); i++)
      kdDebug() << QString("Device %1: %2\n").arg(i+1).arg(deviceNames[i]);

  //  return app.exec();
}
