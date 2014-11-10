/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "mainwindow.h"

#include <KAboutData>

#include <KLocalizedString>
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

static const char description[] = I18N_NOOP("A test app for embedding calendarviews");

int main(int argc, char **argv)
{
    KAboutData about(QStringLiteral("viewerapp"), i18n("ViewerApp"), QLatin1String("0.1"), i18n(description),
                     KAboutLicense::GPL,
                     i18n("Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net"));
    about.addAuthor(i18n("Kevin Krammer"), QString(), QStringLiteral("krake@kdab.com"));

    QCommandLineParser parser;
    QApplication app(argc, argv);
    parser.addVersionOption();
    parser.addHelpOption();
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("+[viewname]"), i18n("Optional list of view names to instantiate")));

    QStringList viewNames;
    for (int i = 0; i < parser.positionalArguments().count(); ++i) {
        viewNames << parser.positionalArguments().at(i).toLower();
    }

    MainWindow *widget = new MainWindow(viewNames) ;

    widget->show();

    return app.exec();
}

