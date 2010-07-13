/*
* This file is part of Akonadi
*
* Copyright 2010 Stephen Kelly <steveire@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/

#include <kdeclarativeapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <QDateTime>

#include "mainview.h"

int main( int argc, char **argv )
{
  kWarning() << "Starting main function" << QDateTime::currentDateTime();

  const QByteArray& ba = QByteArray( "kmail-mobile" );
  const KLocalizedString name = ki18n( "KMail Mobile" );
  KAboutData aboutData( ba, ba, name, ba, name );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineOptions options;
  options.add("timeit", ki18n("start timers for various parts of the application startup"));
  KCmdLineArgs::addCmdLineOptions(options);

  KDeclarativeApplication app;

  MainView view;
  view.show();

  return app.exec();
}
