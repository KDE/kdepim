/*******************************************************************
 This file is part of KNotes.

 Copyright (c) 2004, Bo Thorsen <bo@sonofthor.dk>
               2004-2006, Michael Brade <brade@kde.org>

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
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 MA  02110-1301, USA.

 In addition, as a special exception, the copyright holders give
 permission to link the code of this program with any edition of
 the Qt library by Trolltech AS, Norway (or with modified versions
 of Qt that use the same license as Qt), and distribute linked
 combinations including the two.  You must obey the GNU General
 Public License in all respects for all of the code used other than
 Qt.  If you modify this file, you may extend this exception to
 your version of the file, but you are not obligated to do so.  If
 you do not wish to do so, delete this exception statement from
 your version.
*******************************************************************/

#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kconfiggroup.h>

#include <kcal/icalformat.h>

#include "knotes/resourcelocal.h"
#include "knotes/resourcelocalconfig.h"
#include "knotes/resourcemanager.h"
#include "knotes/resourcenotes.h"


ResourceLocal::ResourceLocal()
  : ResourceNotes(), mCalendar( QString::fromLatin1( "UTC" ) )
{
  kDebug( 5500 ) << "ResourceLocal::ResourceLocal()";
  setType( "file" );
  mURL = KUrl::fromPath( KGlobal::dirs()->saveLocation( "data", "knotes/" ) +
                         "notes.ics" );
}

ResourceLocal::ResourceLocal( const KConfigGroup &group )
  : ResourceNotes( group ), mCalendar( QString::fromLatin1( "UTC" ) )
{
  kDebug( 5500 ) << "ResourceLocal::ResourceLocal()";
  setType( "file" );
  mURL = KUrl::fromPath( KGlobal::dirs()->saveLocation( "data", "knotes/" ) +
                         "notes.ics" );
  
  KUrl u = group.readPathEntry( "NotesURL", QString() );
  if ( !u.isEmpty() ) {
    mURL = u;
  }
}

ResourceLocal::~ResourceLocal()
{
}

void ResourceLocal::writeConfig( KConfigGroup &group )
{
  KRES::Resource::writeConfig( group );
  group.writePathEntry( "NotesURL", mURL.prettyUrl() );
}

bool ResourceLocal::load()
{
  mCalendar.load( mURL.path() );
  
  KCal::Journal::List notes = mCalendar.journals();
  KCal::Journal::List::ConstIterator it;
  for ( it = notes.constBegin(); it != notes.constEnd(); ++it ) {
    manager()->registerNote( this, *it );
  }
  
  return true;
}

bool ResourceLocal::save()
{
  if ( !mCalendar.save( mURL.path(), new KCal::ICalFormat() ) ) {
    KMessageBox::error( 0, i18n( "<qt>Unable to save the notes to <b>%1</b>. "
                                 "Check that there is sufficient disk space."
                                 "<br />There should be a backup in the same "
                                 "directory though.</qt>", mURL.path() ) );
    return false;
  }
  
    return true;
}

bool ResourceLocal::addNote( KCal::Journal *journal )
{
  mCalendar.addJournal( journal );
  return true;
}

bool ResourceLocal::deleteNote( KCal::Journal *journal )
{
  mCalendar.deleteJournal( journal );
  return true;
}

KCal::Alarm::List ResourceLocal::alarms( const KDateTime &from,
                                         const KDateTime &to )
{
  KCal::Alarm::List alarms;
  KCal::Journal::List notes = mCalendar.journals();
  KCal::Journal::List::ConstIterator note;
  
  for ( note = notes.constBegin(); note != notes.constEnd(); ++note ) {
    KDateTime preTime = from.addSecs( -1 );
    KCal::Alarm::List::ConstIterator it;
    for( it = ( *note )->alarms().begin();
         it != ( *note )->alarms().end(); ++it ) {
      if ( ( *it )->enabled() ) {
        KDateTime dt = ( *it )->nextRepetition( preTime );
        if ( dt.isValid() && dt <= to ) {
          alarms.append( *it );
        }
      }
    }
  }
  
  return alarms;
}
