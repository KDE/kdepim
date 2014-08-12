/*
    This file is part of Akonadi.

    Copyright (c) 2011 Stephen Kelly <steveire@gmail.com>

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

#include <KAboutData>

#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>


#include "mobile_mainview.h"

int main( int argc, char **argv )
{
  const QString ba( QLatin1String("coisceim-mobile") );
  const QString name = i18n( "Kontact Touch Trips" );

  KAboutData aboutData( ba, name, name );
  aboutData.setProductName( "Coisceim Mobile" );

    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

  MobileMainview view;
  view.show();

  return app.exec();
}

