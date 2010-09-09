/*
* This file is part of Akonadi
*
* Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/

#include <kdeclarativeapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <incidenceeditor-ng/korganizereditorconfig.h>

#include "mainview.h"

using namespace IncidenceEditorNG;

int main( int argc, char **argv )
{
  const QByteArray& ba = QByteArray( "tasks-mobile" );
  const KLocalizedString name = ki18n( "Tasks Mobile" );

  // FIXME we don't build incidenceeditor on WinCE, let's hope we don't have the crash there
#ifndef Q_OS_WINCE
  // NOTE: This is necessary to avoid a crash, but will result in an empty config.
  //       To make this really configurable do something like KOrganizerEditorConfig
  //       in incidinceeditors/groupwareintegration.cpp
  EditorConfig::setEditorConfig( new KOrganizerEditorConfig );
#endif

  KAboutData aboutData( ba, ba, name, ba, name );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KDeclarativeApplication::initCmdLine();
  KDeclarativeApplication app;

  MainView view;
  view.show();

  return app.exec();
}

