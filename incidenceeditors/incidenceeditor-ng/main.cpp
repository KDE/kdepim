/*
    Copyright (C) 2010  Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
  about.addAuthor( ki18n( "Bertjan Broeksema" ), ki18n( "Author" ), "broeksema@kde.org" );
  about.setProgramIconName( "korganizer" );

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

  EventOrTodoDialog dialog;
  dialog.resize( QSize( 600, 500 ).expandedTo( dialog.minimumSizeHint() ) );
  dialog.load( item );
  dialog.show();

  return app.exec();
}
