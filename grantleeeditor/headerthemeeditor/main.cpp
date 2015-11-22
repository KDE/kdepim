/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
#include <QCommandLineParser>
#include <kaboutdata.h>
#include <KLocalizedString>
#include <Kdelibs4ConfigMigrator>

#include <KDBusService>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    Kdelibs4ConfigMigrator migrate(QStringLiteral("headerthemeeditor"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("headerthemeeditorrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("headerthemeeditorui.rc"));
    migrate.migrate();
    KLocalizedString::setApplicationDomain("headerthemeeditor");
    KAboutData aboutData(QStringLiteral("headerthemeeditor"),
                         i18n("Header Theme Editor"),
                         QStringLiteral(KDEPIM_VERSION),
                         i18n("Messageviewer Header Theme Editor"),
                         KAboutLicense::GPL_V2,
                         i18n("Copyright © 2013-2015 headerthemeeditor authors"));
    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kmail")));
    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    aboutData.setProductName(QByteArray("headerthemeeditor"));

    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    KDBusService service;

    ThemeEditorMainWindow *mw = new ThemeEditorMainWindow();
    mw->show();
    const int ret = app.exec();
    delete mw;
    return ret;
}
