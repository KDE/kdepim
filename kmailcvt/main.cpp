/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Wed Aug  2 11:23:04 CEST 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <dcopclient.h>

#include <unistd.h>

#include "kmailcvt.h"

static const char *description =
	I18N_NOOP("A little tool to convert mail boxes and address books to kmail format");
	
	
static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

static KApplication *app;

int main(int argc, char *argv[])
{
  KLocale::setMainCatalogue("kmailcvt");

  KAboutData aboutData( "kmailcvt2", I18N_NOOP("Kmailcvt2"),
    VERSION, description, KAboutData::License_GPL,
    "(c) 2000, Hans Dijkema");
  aboutData.addAuthor("Hans Dijkema",0, "kmailcvt@hum.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication a;

  app=&a;

  Kmailcvt2 *kmailcvt2 = new Kmailcvt2();
  a.setMainWidget(kmailcvt2);
  kmailcvt2->show();  

  DCOPClient *client=a.dcopClient();
  if (!client->attach()) {
    exit(1);
  }

  return a.exec();
}

void procEvents(void)
{
  app->processEvents(50);
}

int dcopAddMessage(QString folderName,QString fileName)
{
   const QByteArray kmData;
   QByteArray kmRes;
   QDataStream kmArg(kmData,IO_WriteOnly);
   const QCString kmApp("kmail"),kmIface("KMailIface"),
                  kmFunc("dcopAddMessage(QString,KURL)");
   //QCString type("int");
   QCString type;//("void");
   bool res;
   KURL message(fileName);


   kmArg << folderName;
   kmArg << message;

   DCOPClient *c=app->dcopClient();
   res=c->call(kmApp,kmIface,kmFunc,kmData,type,kmRes);
   if (!res) { 
     c->detach();
     if (!fork()) {
        if (execlp("kmail","kmail",NULL)==-1) {
          exit(0);
        }
     }
     sleep(5);
     c->attach();
     res=c->call(kmApp,kmIface,kmFunc,kmData,type,kmRes);
     if (!res) { return -3; }
   }

   QDataStream KmResult(kmRes,IO_ReadOnly);
   int result;
   KmResult >> result;
   //printf("res=%d %d\n",res,result);

return result; 
}

void dcopReload(void)
{
//const QByteArray kmData;
//QByteArray kmRes;
//const QCString kmApp("kmail"),kmIface("KMailIface"),
               //kmFunc("dcopReload()");
//QCString type;
   //DCOPClient *c=app->dcopClient();
   //c->call(kmApp,kmIface,kmFunc,kmData,type,kmRes);
}


