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
#include <KApplication>

#include "kmailcvt.h"
#include "kdepim-version.h"

int main(int argc, char *argv[])
{
    KLocale::setMainCatalog("kmailcvt");

    KAboutData aboutData( "kmailcvt", 0, ki18n("KMailCVT"),
                          KDEPIM_VERSION, ki18n("Mail Import Tool"), KAboutData::License_GPL_V2,
                          ki18n("Copyright © 2000–2014 KMailCVT authors"));
    aboutData.addAuthor(ki18n("Laurent Montel"), ki18n("Maintainer & New filter & cleanups"), "montel@kde.org");
    aboutData.addAuthor(ki18n("Hans Dijkema"),ki18n("Original author"), "kmailcvt@hum.org");
    aboutData.addAuthor(ki18n("Danny Kukawka"), ki18n("Previous Maintainer & New filters"), "danny.kukawka@web.de");
    aboutData.addAuthor(ki18n("Laurence Anderson"), ki18n("New GUI & cleanups"), "l.d.anderson@warwick.ac.uk");
    aboutData.addCredit(ki18n("Daniel Molkentin"), ki18n("New GUI & cleanups"), "molkentin@kde.org");
    aboutData.addCredit(ki18n("Matthew James Leach"), ki18n("Port to Akonadi"), "matthew@theleachfamily.co.uk");

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KApplication a;
    KMailCVT *kmailcvt = new KMailCVT();
    a.setTopWidget(kmailcvt);
    kmailcvt->show();
    int ret = a.exec();
    delete kmailcvt;
    return ret;
}
