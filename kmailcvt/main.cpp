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


#include <kaboutdata.h>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QApplication>
#include "kmailcvt.h"
#include "kdepim-version.h"
#include "pimcommon/util/kdelibs4configmigrator.h"

int main(int argc, char *argv[])
{
    PimCommon::Kdelibs4ConfigMigrator migrate(QLatin1String("kmailcvt"));
    migrate.setConfigFiles(QStringList() << QLatin1String("kmailcvtrc"));
    migrate.migrate();

    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("kmailcvt");

    KAboutData aboutData( QStringLiteral("kmailcvt"), i18n("KMailCVT"),
                          QStringLiteral(KDEPIM_VERSION), i18n("Mail Import Tool"), KAboutLicense::GPL_V2,
                          i18n("Copyright © 2000–2014 KMailCVT authors"));

    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Maintainer & New filter & cleanups"), QLatin1String("montel@kde.org"));
    aboutData.addAuthor(i18n("Hans Dijkema"),i18n("Original author"), QLatin1String("kmailcvt@hum.org"));
    aboutData.addAuthor(i18n("Danny Kukawka"), i18n("Previous Maintainer & New filters"), QLatin1String("danny.kukawka@web.de"));
    aboutData.addAuthor(i18n("Laurence Anderson"), i18n("New GUI & cleanups"), QLatin1String("l.d.anderson@warwick.ac.uk"));
    aboutData.addCredit(i18n("Daniel Molkentin"), i18n("New GUI & cleanups"), QLatin1String("molkentin@kde.org"));
    aboutData.addCredit(i18n("Matthew James Leach"), i18n("Port to Akonadi"), QLatin1String("matthew@theleachfamily.co.uk"));

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KMailCVT *kmailcvt = new KMailCVT();
    kmailcvt->show();
    int ret = app.exec();
    delete kmailcvt;
    return ret;
}
