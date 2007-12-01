/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "mainwindow.h"

int main( int argc, char **argv )
{
  KAboutData aboutData( "scalixadmin", 0, ki18n("ScalixAdmin"), "1.0",
                        ki18n("Configuration Tool for Scalix Groupware Konnector"),
                        KAboutData::License_GPL,
                        ki18n("Copyright 2007, Tobias Koenig"), KLocalizedString(),
                        "http://www.kde.org" );

  aboutData.addAuthor( ki18n( "Tobias Koenig" ), ki18n( "Maintainer" ), "tokoe@kde.org", 0 );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( KCmdLineOptions() );

  KApplication app;

  KGlobal::locale()->insertCatalog( "scalixadmin" );

  MainWindow *window = new MainWindow;
  window->show();

  return app.exec();
}
