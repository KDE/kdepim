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

#include <iostream>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>

#include <Akonadi/Item>
#include <KCal/Event>
#include <KCal/Todo>

#include "../korganizereditorconfig.h"
#include "eventortododialog.h"

using namespace IncidenceEditors;
using namespace IncidenceEditorsNG;

int main( int argc, char **argv )
{
  KAboutData about( "IncidenceEditorNGApp",
                    "korganizer",
                    ki18n( "KOrganizer" ),
                    "0.1",
                    ki18n( "KDE application to run the KOrganizer incidenceeditor." ),
                    KAboutData::License_LGPL,
                    ki18n( "(C) 2010 Bertjan Broeksema" ),
                    ki18n( "Run the KOrganizer incidenceeditor ng." ),
                    "http://kdepim.kde.org",
                    "kdepim@kde.org" );
  about.addAuthor( ki18n( "Bertjan Broeksema" ), ki18n( "Author" ), "b.broeksema@home.nl" );

  KCmdLineOptions options;
  options.add("new-event", ki18n("Creates a new event"));
  options.add("new-todo", ki18n("Creates a new todo"));
  options.add("+item", ki18n("Loads an existing item, or returns without doing anything when the item is not an event or todo."));

  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs::init( argc, argv, &about );
  KApplication app;

  KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
  Akonadi::Item item( -1 );
  if ( args->isSet( "new-event" ) ) {
    std::cout << "Creating new event..." << std::endl;
    item.setPayload<KCal::Event::Ptr>( KCal::Event::Ptr( new KCal::Event ) );
  } else if ( args->isSet( "new-todo" ) ) {
    std::cout << "Creating new todo..." << std::endl;
    item.setPayload<KCal::Todo::Ptr>( KCal::Todo::Ptr( new KCal::Todo ) );
  } else if ( args->count() == 1 ) {
    qint64 id = -1;
    if ( argc == 2 ) {
      bool ok = false;
      id = QString( argv[1] ).toLongLong( &ok );
      if ( !ok ) {
        std::cerr << "Invalid akonadi item id given." << std::endl;
        return 1;
      }

      item.setId( id );
      std::cout << "Trying to load Akonadi Item " << QString::number( id ).toLatin1().data();
      std::cout << "..." << std::endl;
    } else {
      std::cerr << "Invalid argument count." << std::endl << std::endl;
      return 1;
    }
  } else {
    std::cerr << "Invalid usage." << std::endl << std::endl;
    return 1;
  }

  EditorConfig::setEditorConfig( new KOrganizerEditorConfig );

  EventOrTodoDialog *dialog = new EventOrTodoDialog;
  dialog->resize( QSize( 600, 600 ).expandedTo( dialog->minimumSizeHint() ) );
  dialog->load( item );

  return app.exec();
}
