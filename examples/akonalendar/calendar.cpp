/*
    This file is part of Akonadi.

    Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>

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



#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>

#include "mainwindow.h"

int main(  int argc,  char **argv )
{
    KAboutData aboutData( QLatin1String("akonalendar"), i18n( "Akonalendar" ), QLatin1String("0.1"));
    aboutData.setShortDescription( i18n( "A calendar demo application" ) );
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    MainWindow window;
    window.show();

    return app.exec();
}
