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

#include <qapplication.h>
#include <kaboutdata.h>
#include <KLocalizedString>

#include <KDBusService>

int main( int argc, char **argv )
{
    KLocalizedString::setApplicationDomain("headerthemeeditor");
    QApplication app(argc, argv);
    KAboutData aboutData( QStringLiteral("headerthemeeditor"), 
                          i18n("Header Theme Editor"), 
                          QLatin1String(KDEPIM_VERSION),
                          i18n("Messageviewer Header Theme Editor"),
                          KAboutLicense::GPL_V2,
                          i18n("Copyright © 2013, 2014 headerthemeeditor authors"));
    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));
    aboutData.setProgramIconName(QLatin1String("kmail"));
    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setProductName(QByteArray("headerthemeeditor"));

    KAboutData::setApplicationData(aboutData);

    app.setApplicationName(aboutData.componentName());
    app.setApplicationDisplayName(aboutData.displayName());
    app.setOrganizationDomain(aboutData.organizationDomain());
    app.setApplicationVersion(aboutData.version());

    KDBusService service();

    ThemeEditorMainWindow *mw = new ThemeEditorMainWindow();
    mw->show();
    app.exec();
}
