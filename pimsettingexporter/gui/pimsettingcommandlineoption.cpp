/*
   Copyright (C) 2015-2016 Montel Laurent <montel@kde.org>

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
#include "pimsettingcommandlineoption.h"
#include "pimsettingexporterwindow.h"
#include "pimsettingexportgui_debug.h"
#include "pimsettingexporter-version.h"
#include <KLocalizedString>
#include <KAboutData>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>

PimSettingCommandLineOption::PimSettingCommandLineOption(QObject *parent)
    : QObject(parent),
      mExporterWindow(Q_NULLPTR)
{
}

PimSettingCommandLineOption::~PimSettingCommandLineOption()
{
}

void PimSettingCommandLineOption::slotActivateRequested(const QStringList &arguments, const QString &workingDirectory)
{
    Q_UNUSED(workingDirectory);
    if (!arguments.isEmpty()) {
        if (mParser.parse(arguments)) {
            handleCommandLine();
        } else {
            qCDebug(PIMSETTINGEXPORTERGUI_LOG) << " Impossible to parse argument ";
        }
    }
}

void PimSettingCommandLineOption::createParser(const QApplication &app)
{
    KAboutData aboutData(QStringLiteral("pimsettingexporter"), i18n("PIM Setting Exporter"),
                         QStringLiteral(PIMSETTINGEXPORTER_VERSION), i18n("PIM Setting Exporter"), KAboutLicense::GPL_V2,
                         i18n("Copyright © 2012-2016 pimsettingexporter authors"));
    aboutData.addAuthor(i18n("Laurent Montel"), i18n("Maintainer"), QStringLiteral("montel@kde.org"));
    mParser.addVersionOption();
    mParser.addHelpOption();
    mParser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("template"), i18n("Template file uses to define what data, settings to import or export"), QStringLiteral("file")));
    mParser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("import"), i18n("Import the given file")));
    mParser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("export"), i18n("Export the given file")));
    mParser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("+[url]"), i18n("File or url. The user will be asked whether to import or export.")));

    KAboutData::setApplicationData(aboutData);

    aboutData.setupCommandLine(&mParser);
    mParser.process(app);
    aboutData.processCommandLine(&mParser);
}

void PimSettingCommandLineOption::setExportWindow(PimSettingExporterWindow *exporterWindow)
{
    mExporterWindow = exporterWindow;
}

void PimSettingCommandLineOption::handleCommandLine()
{
    if (mExporterWindow) {
        mExporterWindow->handleCommandLine(mParser);
    }
}

