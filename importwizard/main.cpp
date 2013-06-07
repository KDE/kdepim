/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <KUniqueApplication>

#include "importwizard.h"

#include "kdepim-version.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    KLocale::setMainCatalog("importwizard");

    //FIXME: "wizards" are "assistents" in new KDE slang
    KAboutData aboutData( "importwizard", 0, ki18n("importwizard"),
                          KDEPIM_VERSION, ki18n("PIM Import Tool"), KAboutData::License_GPL_V2,
                          ki18n("Copyright © 2012-2013 importwizard authors"));
    aboutData.addAuthor(ki18n("Laurent Montel"), ki18n("Maintainer"), "montel@kde.org");
    aboutData.setProgramIconName(QLatin1String("kontact-import-wizard"));
    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
    KUniqueApplication::addCmdLineOptions();

    if ( !KUniqueApplication::start() ) {
        fprintf( stderr, "importwizard is already running!\n" );
        exit( 0 );
    }
    KUniqueApplication a;

    ImportWizard *wizard = new ImportWizard();
    a.setTopWidget(wizard);
    wizard->show();
    int ret = a.exec();
    delete wizard;
    return ret;
}
