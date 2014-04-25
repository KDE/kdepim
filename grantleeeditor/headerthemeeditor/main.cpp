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

#include "themeeditormainwindow.h"

#include "kdepim-version.h"

#include <kapplication.h>
#include <k4aboutdata.h>
#include <kcmdlineargs.h>
#include <KGlobal>

int main( int argc, char **argv )
{
    K4AboutData aboutData( "headerthemeeditor", 0, ki18n("Messageviewer Header Theme Editor"),
      KDEPIM_VERSION, ki18n("Header Theme Editor"), K4AboutData::License_GPL_V2,
      ki18n("Copyright © 2013, 2014 headerthemeeditor authors"));
    aboutData.addAuthor(ki18n("Laurent Montel"), ki18n("Maintainer"), "montel@kde.org");
    aboutData.setProgramIconName(QLatin1String("kmail"));
    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KApplication app;
    KGlobal::locale()->insertCatalog(QLatin1String("libpimcommon"));
    ThemeEditorMainWindow *mw = new ThemeEditorMainWindow();
    mw->show();
    app.exec();
}
