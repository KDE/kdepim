/*
   Copyright (C) 2012-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "pimsettingexporterwindow.h"
#include "pimsettingcommandlineoption.h"

#include <KLocalizedString>
#include <KDBusService>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>
#include <kdelibs4configmigrator.h>
#include <KCrash>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("pimsettingexporter");

    KCrash::initialize();
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
    Kdelibs4ConfigMigrator migrate(QStringLiteral("pimsettingexporter"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("pimsettingexporterrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("pimsettingexporter.rc"));
    migrate.migrate();
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kontact")));

    PimSettingCommandLineOption parser;
    parser.createParser(app);

    KDBusService service(KDBusService::Unique);

    PimSettingExporterWindow *backupMailWin = new PimSettingExporterWindow();
    parser.setExportWindow(backupMailWin);
    QObject::connect(&service, &KDBusService::activateRequested,
                     &parser, &PimSettingCommandLineOption::slotActivateRequested);
    backupMailWin->show();
    parser.handleCommandLine();

    return app.exec();
}
