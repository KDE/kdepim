/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include <k4aboutdata.h>
#include <klocale.h>
#include <KApplication>
#include <KFileDialog>
#include "mboxmainwindow.h"

#include <KLocalizedString>
#include <QFileDialog>
#include "kdepim-version.h"

int main(int argc, char *argv[])
{
    KLocalizedString::setApplicationDomain("mboximporter");
    K4AboutData aboutData( "mboximporter", 0, ki18n("mbox importer"),
                          KDEPIM_VERSION, ki18n("MBox importer tool"), K4AboutData::License_GPL_V2,
                          ki18n("Copyright © 2013 MBoxImporter authors"));
    aboutData.addAuthor(ki18n("Laurent Montel"), ki18n("Maintainer"), "montel@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutData );

    QString fileName;
    KCmdLineOptions option;
    option.add("+[url]", ki18n("URL of mbox to be imported"));
    KCmdLineArgs::addCmdLineOptions(option);

    KApplication a;
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count()) {
        fileName = args->url(0).path();
        args->clear();
    } else {
        fileName = QFileDialog::getOpenFileName(); 
    }
    if (fileName.isEmpty()) {
        return 0;
    }
    MBoxMainWindow *w = new MBoxMainWindow(fileName);
    w->show();
    int ret = a.exec();
    delete w;
    return ret;
}
