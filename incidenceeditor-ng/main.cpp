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

#include <k4aboutdata.h>
#include <KApplication>
#include <KCmdLineArgs>

#include <calendarsupport/kcalprefs.h>
#include <Akonadi/calendar/calendarsettings.h>

#include <Item>
#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <KCalCore/Journal>

#include "korganizereditorconfig.h"
#include "incidencedialog.h"
#include "incidencedefaults.h"

using namespace IncidenceEditorNG;

int main( int argc, char **argv )
{
  K4AboutData about( "IncidenceEditorNGApp",
                    "korganizer",
                    ki18n( "KOrganizer" ),
                    "0.1",
                    ki18n( "KDE application to run the KOrganizer incidenceeditor." ),
                    K4AboutData::License_LGPL,
                    ki18n( "(C) 2010 Bertjan Broeksema" ),
                    ki18n( "Run the KOrganizer incidenceeditor ng." ),
                    "http://kdepim.kde.org",
                    "kdepim@kde.org" );
  about.addAuthor( ki18n( "Bertjan Broeksema" ), ki18n( "Author" ), "broeksema@kde.org" );
  about.setProgramIconName( "korganizer" );

  KCmdLineOptions options;
  options.add( "new-event", ki18n( "Creates a new event" ) );
  options.add( "new-todo", ki18n( "Creates a new todo" ) );
  options.add( "new-journal", ki18n( "Creates a new journal" ) );
  options.add( "+item",
               ki18n( "Loads an existing item, or returns without doing anything "
                      "when the item is not an event or todo." ) );

  KCmdLineArgs::addCmdLineOptions( options );
  KCmdLineArgs::init( argc, argv, &about );
  KApplication app;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  Akonadi::Item item( -1 );

  IncidenceDefaults defaults;
  // Set the full emails manually here, to avoid that we get dependencies on
  // KCalPrefs all over the place.
  defaults.setFullEmails( CalendarSupport::KCalPrefs::instance()->fullEmails() );
  // NOTE: At some point this should be generalized. That is, we now use the
  //       freebusy url as a hack, but this assumes that the user has only one
  //       groupware account. Which doesn't have to be the case necessarily.
  //       This method should somehow depend on the calendar selected to which
  //       the incidence is added.
  if ( CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication() ) {
    defaults.setGroupWareDomain(
      KUrl( Akonadi::CalendarSettings::self()->freeBusyRetrieveUrl() ).host() );
  }

  if ( args->isSet( "new-event" ) ) {
    std::cout << "Creating new event..." << std::endl;
    KCalCore::Event::Ptr event( new KCalCore::Event );
    defaults.setDefaults( event );
    item.setPayload<KCalCore::Event::Ptr>( event );
  } else if ( args->isSet( "new-todo" ) ) {
    std::cout << "Creating new todo..." << std::endl;
    KCalCore::Todo::Ptr todo( new KCalCore::Todo );
    defaults.setDefaults( todo );
    item.setPayload<KCalCore::Todo::Ptr>( todo );
  } else if ( args->isSet( "new-journal" ) ) {
    std::cout << "Creating new journal..." << std::endl;
    KCalCore::Journal::Ptr journal( new KCalCore::Journal );
    defaults.setDefaults( journal );
    item.setPayload<KCalCore::Journal::Ptr>( journal );
  } else if ( args->count() == 1 ) {
    if ( argc == 2 ) {
      bool ok = false;
      qint64 id = QString( argv[1] ).toLongLong( &ok );
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

  IncidenceDialog *dialog = new IncidenceDialog();

  Akonadi::Collection collection( CalendarSupport::KCalPrefs::instance()->defaultCalendarId() );

  if ( collection.isValid() )
    dialog->selectCollection( collection );

  dialog->load( item ); // The dialog will show up once the item is loaded.

  return app.exec();
}
