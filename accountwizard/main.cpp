/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

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

#include "dialog.h"
#include "global.h"

#include <controlgui.h>

#include <kaboutdata.h>
#include <qapplication.h>
#include <QUrl>

#include <KDBusService>
#include <KLocalizedString>
#include <stdio.h>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QIcon>
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#if QT_VERSION >= 0x050600
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    KLocalizedString::setApplicationDomain("accountwizard");

    KAboutData aboutData(QStringLiteral("accountwizard"),
                         i18n("Account Assistant"),
                         QStringLiteral("0.2"),
                         i18n("Helps setting up PIM accounts"),
                         KAboutLicense::LGPL,
                         i18n("(c) 2009-2016 the Akonadi developers"),
                         QStringLiteral("http://pim.kde.org/akonadi/"));
    aboutData.addAuthor(i18n("Volker Krause"),  i18n("Author"), QStringLiteral("vkrause@kde.org"));
    aboutData.addAuthor(i18n("Laurent Montel"), QString(), QStringLiteral("montel@kde.org"));

    app.setOrganizationDomain(QStringLiteral("kde.org"));
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("akonadi")));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("type"), i18n("Only offer accounts that support the given type."), QStringLiteral("type")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("assistant"), i18n("Run the specified assistant."), QStringLiteral("assistant")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("package"), i18n("unpack fullpath on startup and launch that assistant"), QStringLiteral("fullpath")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    KDBusService service(KDBusService::Unique);

    Akonadi::ControlGui::start(0);

    const QString packageArgument = parser.value(QStringLiteral("package"));
    if (!packageArgument.isEmpty()) {
        Global::setAssistant(Global::unpackAssistant(QUrl::fromLocalFile(packageArgument)));
    } else {
        Global::setAssistant(parser.value(QStringLiteral("assistant")));
    }

    QString typeValue = parser.value(QStringLiteral("type"));
    if (!typeValue.isEmpty()) {
        Global::setTypeFilter(typeValue.split(QLatin1Char(',')));
    }

    Dialog dlg(0);
    dlg.show();

    return app.exec();
}
