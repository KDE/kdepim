/*
  Copyright (c) 2013-2016 Montel Laurent <montel@kde.org>

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
#include "storageservicemanagermainwindow.h"
#include <kaboutdata.h>
#include <Kdelibs4ConfigMigrator>

#include <qcommandlineparser.h>
#include <qcommandlineoption.h>
#include <KDBusService>
#include <KLocalizedString>
#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("storageservicemanager");

    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    Kdelibs4ConfigMigrator migrate(QStringLiteral("storageservice"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("storageservicerc") << QStringLiteral("storageservicemanager.notifyrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("storageserviceui.rc"));
    migrate.migrate();

    KAboutData aboutData(QStringLiteral("storageservicemanager"),
                         i18n("Storage Service Manager"),
                         QStringLiteral(KDEPIM_VERSION),
                         i18n("Storage Service Manager"),
                         KAboutLicense::GPL_V2,
                         i18n("Copyright © 2013-2016 storageservicemanager authors"));
    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));

    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kmail")));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KDBusService service(KDBusService::Unique);

    StorageServiceManagerMainWindow *mw = new StorageServiceManagerMainWindow();
    mw->show();
    return app.exec();
}
