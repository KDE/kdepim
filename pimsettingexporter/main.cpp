/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "pimsettingexporterwindow.h"

#include <kaboutdata.h>
#include <KLocalizedString>
#include <KDBusService>
#include "pimsettingexport_debug.h"
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>
#include "kdepim-version.h"
#include <kdelibs4configmigrator.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Kdelibs4ConfigMigrator migrate(QStringLiteral("pimsettingexporter"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("pimsettingexporterrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("pimsettingexporter.rc"));
    migrate.migrate();
    KLocalizedString::setApplicationDomain("pimsettingexporter");

    KAboutData aboutData(QStringLiteral("pimsettingexporter"), i18n("PIM Setting Exporter"),
                         QStringLiteral(KDEPIM_VERSION), i18n("PIM Setting Exporter"), KAboutLicense::GPL_V2,
                         i18n("Copyright © 2012-2015 pimsettingexporter authors"));
    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kontact")));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("template"), i18n("Template file uses to define what data, settings to import or export"), QStringLiteral("file")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("import"), i18n("Import the given file")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("export"), i18n("Export the given file")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("+[url]"), i18n("File or url. The user will be asked whether to import or export.")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KDBusService service(KDBusService::Unique);

    PimSettingExporterWindow *backupMailWin = new PimSettingExporterWindow();
    QObject::connect(&service, SIGNAL(activateRequested(QStringList,QString)),
                     backupMailWin, SLOT(slotActivateRequested(QStringList,QString)));
    backupMailWin->show();
    backupMailWin->handleCommandLine(parser);

    return app.exec();
}
