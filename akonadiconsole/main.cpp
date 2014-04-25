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

#include <k4aboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include "instanceselector.h"

#include <stdlib.h>

int main( int argc, char **argv )
{
  K4AboutData aboutData( "akonadiconsole", 0,
                        ki18n( "Akonadi Console" ),
                        "0.99",
                        ki18n( "The Management and Debugging Console for Akonadi" ),
                        K4AboutData::License_GPL,
                        ki18n( "(c) 2006-2008 the Akonadi developer" ),
                        KLocalizedString(),
                        "http://pim.kde.org/akonadi/" );
  aboutData.setProgramIconName( "akonadi" );
  aboutData.addAuthor( ki18n( "Tobias König" ), ki18n( "Author" ), "tokoe@kde.org" );
  aboutData.addAuthor( ki18n( "Volker Krause" ),  ki18n( "Author" ), "vkrause@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineOptions options;
  options.add( "remote <server>", ki18n("Connect to an Akonadi remote debugging server"));
  KCmdLineArgs::addCmdLineOptions( options );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if ( args->isSet( "remote" ) ) {
    const QString akonadiAddr = QString::fromLatin1( "tcp:host=%1,port=31415" ).arg( args->getOption( "remote" ) );
    const QString dbusAddr = QString::fromLatin1( "tcp:host=%1,port=31416" ).arg( args->getOption( "remote" ) );
    setenv( "AKONADI_SERVER_ADDRESS", akonadiAddr.toLatin1(), 1 );
    setenv( "DBUS_SESSION_BUS_ADDRESS", dbusAddr.toLatin1(), 1 );
  }

  KApplication app;

  InstanceSelector instanceSelector( args->isSet( "remote" ) ? args->getOption( "remote" ) : QString() );

  return app.exec();
}
