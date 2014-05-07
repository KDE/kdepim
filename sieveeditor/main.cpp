/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/


#include "kdepim-version.h"
#include "sieveeditormainwindow.h"
#include "sieveeditor_options.h"
#include <k4aboutdata.h>
#include <kcmdlineargs.h>
#include <QDebug>
#include <KUniqueApplication>
#include <KGlobal>
#include <KLocalizedString>

int main( int argc, char **argv )
{
    K4AboutData aboutData( "sieveeditor", 0, ki18n("Sieve Editor"),
                          KDEPIM_VERSION, ki18n("Sieve Editor"), K4AboutData::License_GPL_V2,
                          ki18n("Copyright © 2013, 2014 sieveeditor authors"));
    aboutData.addAuthor(ki18n("Laurent Montel"), ki18n("Maintainer"), "montel@kde.org");
    aboutData.setProgramIconName(QLatin1String("kmail"));
    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options = sieveeditor_options();
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
    KUniqueApplication::addCmdLineOptions();
    if (!KUniqueApplication::start()) {
        qDebug() << "sieveeditor is already running!";
        return (0);
    }
    KUniqueApplication a;
    SieveEditorMainWindow *mw = new SieveEditorMainWindow();
    mw->show();
    a.exec();
}
