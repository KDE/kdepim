/*
  This file is part of KOrganizer.

  Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>

#include "eventortododialog.h"

int main( int argc, char **argv )
{
  KAboutData about( "IncidenceEditorNGApp",
                    "korganizer",
                    ki18n( "IncidenceEditorApp" ),
                    "0.1",
                    ki18n( "KDE application to run the KOrganizer incidenceeditor." ),
                    KAboutData::License_LGPL,
                    ki18n( "(C) 2010 Bertjan Broeksema" ),
                    ki18n( "Run the KOrganizer incidenceeditor ng." ),
                    "http://kdepim.kde.org",
                    "kdepim@kde.org" );
  about.addAuthor( ki18n( "Bertjan Broeksema" ), ki18n( "Author" ), "b.broeksema@home.nl" );
  KCmdLineArgs::init( argc, argv, &about );
  KApplication app;

  EventOrTodoDialog *dialog = new EventOrTodoDialog;
  dialog->resize( QSize( 800, 600 ).expandedTo( dialog->minimumSizeHint() ) );
  dialog->show();

  return app.exec();
}
