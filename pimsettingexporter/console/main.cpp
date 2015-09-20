/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "kdepim-version.h"
#include "pimsettingexporterconsole.h"
#include <kaboutdata.h>
#include <KLocalizedString>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCommandLineOption>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    KAboutData aboutData(QStringLiteral("pimsettingexporterconsole"), i18n("PIM Setting Exporter Console"),
                         QStringLiteral(KDEPIM_VERSION), i18n("PIM Setting Exporter Console"), KAboutLicense::GPL_V2,
                         i18n("Copyright © 2015 pimsettingexporter authors"));
    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("logfile"), i18n("File to log information to."), QStringLiteral("file")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("template"), i18n("Template file to define what data, settings to import or export."), QStringLiteral("file")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("import"), i18n("Import the given file."), QStringLiteral("file")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("export"), i18n("Export the given file."), QStringLiteral("file")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QString importFile;
    QString exportFile;
    if (parser.isSet(QStringLiteral("import"))) {
        importFile = parser.value(QStringLiteral("import"));
    } else if (parser.isSet(QStringLiteral("export"))) {
        exportFile = parser.value(QStringLiteral("export"));
    }
    if (importFile.isEmpty() && exportFile.isEmpty()) {
        return -1;
    }
    QString logFile;
    QString templateFile;
    if (parser.isSet(QStringLiteral("template"))) {
        templateFile = parser.value(QStringLiteral("template"));
    }
    if (parser.isSet(QStringLiteral("logfile"))) {
        logFile = parser.value(QStringLiteral("logfile"));
    }

    PimSettingExporterConsole *console = new PimSettingExporterConsole;
    if (!importFile.isEmpty()) {
        console->setMode(PimSettingExporterConsole::Import);
        console->setImportExportFileName(importFile);
    } else if (!exportFile.isEmpty()) {
        console->setMode(PimSettingExporterConsole::Export);
        console->setImportExportFileName(exportFile);
    }
    if (!logFile.isEmpty()) {
        console->setLogFileName(logFile);
    }
    if (!templateFile.isEmpty()) {
        console->setTemplateFileName(templateFile);
    }
    console->start();
    QObject::connect(console, &PimSettingExporterConsole::finished, &app, &QCoreApplication::quit);

    return app.exec();
}
