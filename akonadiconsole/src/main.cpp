/*
    This file is part of Akonadi.

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include <kaboutdata.h>
#include <qapplication.h>

#include "instanceselector.h"

#include <Kdelibs4ConfigMigrator>
#include <KDBusService>
#include <KCrash>
#include <stdlib.h>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <QtDBus/QDBusMetaType>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
    Kdelibs4ConfigMigrator migrate(QStringLiteral("akonadiconsole"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("akonadiconsolerc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("akonadiconsoleui.rc"));
    migrate.migrate();

    KAboutData aboutData(QStringLiteral("akonadiconsole"),
                         QStringLiteral("Akonadi Console"),
                         QStringLiteral("0.99"),
                         QStringLiteral("The Management and Debugging Console for Akonadi"),
                         KAboutLicense::GPL,
                         QStringLiteral("(c) 2006-2016 the Akonadi developer"), QString(),
                         QStringLiteral("https://community.kde.org/KDE_PIM/akonadi"));
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("akonadi")));
    aboutData.addAuthor(QStringLiteral("Tobias König"), QStringLiteral("Author"), QStringLiteral("tokoe@kde.org"));
    aboutData.addAuthor(QStringLiteral("Volker Krause"),  QStringLiteral("Author"), QStringLiteral("vkrause@kde.org"));
    KAboutData::setApplicationData(aboutData);

    KCrash::initialize();
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("remote"), QStringLiteral("Connect to an Akonadi remote debugging server"), QStringLiteral("server")));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    KDBusService service;

    qRegisterMetaType<QVector<QByteArray>>();
    qDBusRegisterMetaType<QVector<qint64>>();
    qDBusRegisterMetaType<QVector<QByteArray>>();

    if (parser.isSet(QStringLiteral("remote"))) {
        const QString akonadiAddr = QStringLiteral("tcp:host=%1,port=31415").arg(parser.value(QStringLiteral("remote")));
        const QString dbusAddr = QStringLiteral("tcp:host=%1,port=31416").arg(parser.value(QStringLiteral("remote")));
        setenv("AKONADI_SERVER_ADDRESS", akonadiAddr.toLatin1(), 1);
        setenv("DBUS_SESSION_BUS_ADDRESS", dbusAddr.toLatin1(), 1);
    }

    InstanceSelector instanceSelector(parser.isSet(QStringLiteral("remote")) ? parser.value(QStringLiteral("remote")) : QString());

    return app.exec();
}
