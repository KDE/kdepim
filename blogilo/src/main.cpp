/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "mainwindow.h"
#include "global.h"
#include "constants.h"
#include "blogilomigrateapplication.h"

#include <QApplication>
#include <KCrash>
#include <KLocalizedString>
#include <kaboutdata.h>
#include <QCommandLineParser>
#include <kdbusservice.h>
#include <kdelibs4configmigrator.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    app.setAttribute(Qt::AA_EnableHighDpiScaling);

    BlogiloMigrateApplication migrate;
    migrate.migrate();

    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("blogilo")));
    KLocalizedString::setApplicationDomain("blogilo");
    KAboutData about(QStringLiteral("blogilo"), i18n("Blogilo"), VERSION, i18n("A KDE Blogging Client"),
                     KAboutLicense::GPL_V2, i18n("Copyright © 2008–2016 Blogilo authors"));
    about.addAuthor(i18n("Mehrdad Momeny"), i18n("Core Developer"), QStringLiteral("mehrdad.momeny@gmail.com"));
    about.addAuthor(i18n("Golnaz Nilieh"), i18n("Core Developer"), QStringLiteral("g382nilieh@gmail.com"));
    about.addAuthor(i18n("Laurent Montel"), i18n("Core Developer"), QStringLiteral("montel@kde.org"));
    about.addCredit(i18n("Roozbeh Shafiee"), i18n("Icon designer"), QStringLiteral("roozbeh@roozbehonline.com"));
    about.addCredit(i18n("Sajjad Baroodkoo"), i18n("Icon designer"), QStringLiteral("sajjad@graphit.ir"));

    about.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"),
                        i18nc("EMAIL OF TRANSLATORS", "Your emails"));
    KCrash::initialize();
    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    parser.addVersionOption();
    parser.addHelpOption();
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    global_init();

    KDBusService service(KDBusService::Unique);

    MainWindow *bilbo = new MainWindow;

    bilbo->show();
    int r = app.exec();

    global_end();
    return r;
}

