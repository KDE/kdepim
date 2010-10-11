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

#ifdef KCALCORE_SERIALIZER_PLUGIN_STATIC
#include <QtPlugin>

Q_IMPORT_PLUGIN(akonadi_serializer_kcalcore)
#endif

using namespace IncidenceEditorNG;

int main( int argc, char **argv )
{
  const QByteArray& ba = QByteArray( "tasks-mobile" );
  const KLocalizedString name = ki18n( "Tasks Mobile" );

  EditorConfig::setEditorConfig( new KOrganizerEditorConfig );

  KAboutData aboutData( ba, ba, name, ba, name );
  aboutData.setProductName( "KOrganizer Mobile/tasks" ); //has to match the bugzilla product name

  KCmdLineArgs::init( argc, argv, &aboutData );
  KDeclarativeApplication::initCmdLine();
  KDeclarativeApplication app;

  MainView view;
  view.show();

  return app.exec();
}

