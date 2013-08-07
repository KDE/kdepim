/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
#include "contacteditormainwindow.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

int main( int argc, char **argv )
{
    KAboutData aboutData( "contactthemeeditor", 0, ki18n("Contact Theme Editor"),
      KDEPIM_VERSION, ki18n("Contact Theme Editor"), KAboutData::License_GPL_V2,
      ki18n("Copyright © 2013 contactthemeeditor authors"));
    aboutData.addAuthor(ki18n("Laurent Montel"), ki18n("Maintainer"), "montel@kde.org");
    aboutData.setProgramIconName(QLatin1String("kaddressbook"));
    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KApplication app;
    ContactEditorMainWindow *mw = new ContactEditorMainWindow();
    mw->show();
    app.exec();
}
