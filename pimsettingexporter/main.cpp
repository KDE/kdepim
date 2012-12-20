/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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


#include "backupmailwindow.h"

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <KUniqueApplication>
#include <KDebug>
#include "kdepim-version.h"

int main(int argc, char *argv[])
{
  KLocale::setMainCatalog("pimsettingexporter");

  KAboutData aboutData( "pimsettingexporter", 0, ki18n("PIM Setting Exporter"),
    KDEPIM_VERSION, ki18n("PIM Setting Exporter"), KAboutData::License_GPL_V2,
    ki18n("Copyright © 2012 pimsettingexporter authors"));
  aboutData.addAuthor(ki18n("Laurent Montel"), ki18n("Maintainer"), "montel@kde.org");
  //aboutData.setProgramIconName("kontact-import-wizard");
  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KUniqueApplication::addCmdLineOptions();

  if (!KUniqueApplication::start())
  {
    kDebug() << "pimsettingexporter is already running!";
    return (0);
  }
  KUniqueApplication a;
  BackupMailWindow *backupMailWin = new BackupMailWindow();
  a.setTopWidget(backupMailWin);
  backupMailWin->show();

  return a.exec();
}
