/*
    This file is part of the IMAP resources.
    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

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
*/

#include "resourceimap.h"

#include <knotes/resourcemanager.h>

#include <libkcal/icalformat.h>

#include <kdebug.h>
#include <kglobal.h>


KNotesIMAP::ResourceIMAP::ResourceIMAP( const KConfig *config )
  : ResourceNotes( config ),
    ResourceIMAPBase::ResourceIMAPShared( "ResourceIMAP-KNotes" )
{
  setType( "imap" );
}

KNotesIMAP::ResourceIMAP::~ResourceIMAP()
{
}

bool KNotesIMAP::ResourceIMAP::doOpen()
{
  KConfig config( configFile() );

  // Read the calendar entries
  QStringList resources;
  if ( !kmailSubresources( resources, "Note" ) )
    return false;
  config.setGroup( "Note" );
  QStringList::ConstIterator it;
  mResources.clear();
  for ( it = resources.begin(); it != resources.end(); ++it )
    mResources[ *it ] = config.readBoolEntry( *it, true );

  return true;
}

void KNotesIMAP::ResourceIMAP::doClose()
{
  KConfig config( configFile() );

  config.setGroup( "Note" );
  QMap<QString, bool>::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it )
    config.writeEntry( it.key(), it.data() );
}

bool KNotesIMAP::ResourceIMAP::populate( const QStringList &lst, const QString& resource )
{
  // Populate the calendar with the new entries
  const bool silent = mSilent;
  mSilent = true;
  for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    KCal::Journal* journal = parseJournal( *it );
    if( journal ) {
      addNote( journal, resource );
      mManager->registerNote( this, journal );
    }
  }
  mSilent = silent;
  return true;
}

bool KNotesIMAP::ResourceIMAP::loadResource( const QString& resource )
{
  // Get the list of journals
  QStringList lst;
  if( !kmailIncidences( lst, "Note", resource ) ) {
    kdError(5500) << "Communication problem in "
                  << "ResourceIMAP::getIncidenceList()\n";
    return false;
  }
  return populate( lst, resource );
}

bool KNotesIMAP::ResourceIMAP::load()
{
  // We get a fresh list of events, so clean out the old ones
  mCalendar.deleteAllEvents();
  mUidmap.clear();

  bool rc = true;
  QMap<QString, bool>::ConstIterator itR;
  for ( itR = mResources.begin(); itR != mResources.end(); ++itR ) {
    if ( !itR.data() )
      // This resource is disabled
      continue;

    rc &= loadResource( itR.key() );
  }

  return true;
}

bool KNotesIMAP::ResourceIMAP::save()
{
  return true;
}

bool KNotesIMAP::ResourceIMAP::addNote( KCal::Journal* journal )
{
  return addNote( journal, QString::null );
}

bool KNotesIMAP::ResourceIMAP::addNote( KCal::Journal* journal,
                                        const QString& subresource )
{
  kdDebug(5500) << "KNotesIMAP::ResourceIMAP::addNote( KCal::Journal* )\n";

  mCalendar.addJournal( journal );
  journal->registerObserver( this );

  QString resource = subresource;
  if ( subresource.isEmpty() )
    resource = findWritableResource( mResources, "Note" );
  mUidmap[ journal->uid() ] = resource;

  if ( mSilent ) return true;

  KCal::ICalFormat format;
  QString note = format.toICalString( journal );
  if( !kmailAddIncidence( "Note", resource, journal->uid(), note ) ) {
    kdError(5500) << "Communication problem in ResourceIMAP::addNote()\n";
    return false;
  }

  return true;
}

bool KNotesIMAP::ResourceIMAP::deleteNote( KCal::Journal* journal )
{
  const QString uid = journal->uid();
  kmailDeleteIncidence( "Note", mUidmap[ uid ], uid );
  mUidmap.remove( uid );
  mCalendar.deleteJournal( journal );
  return true;
}

void KNotesIMAP::ResourceIMAP::incidenceUpdated( KCal::IncidenceBase* i )
{
  KCal::ICalFormat format;
  QString note = format.toICalString( static_cast<KCal::Journal*>( i ) );
  if( !kmailUpdate( "Note", mUidmap[ i->uid() ], i->uid(), note ) )
    kdError(5500) << "Communication problem in ResourceIMAP::addNote()\n";
}

/*
 * These are the DCOP slots that KMail call to notify when something
 * changed.
 */
bool KNotesIMAP::ResourceIMAP::addIncidence( const QString& type,
                                             const QString& resource,
                                             const QString& note )
{
  // Check if this is a note
  if( type != "Note" ) return false;

  // kdDebug(5500) << "ResourceIMAP::addIncidence( " << type << ", "
  //               << /*ical*/"..." << " )" << endl;
  KCal::Journal* journal = parseJournal( note );
  if ( !journal ) return false;

  const bool silent = mSilent;
  mSilent = true;
  addNote( journal, resource );
  mSilent = silent;

  return true;
}

void KNotesIMAP::ResourceIMAP::deleteIncidence( const QString& type,
                                                const QString& /*resource*/,
                                                const QString& uid )
{
  // Check if this is a note
  if( type != "Note" ) return;

  // kdDebug(5500) << "ResourceIMAP::deleteIncidence( " << type << ", " << uid
  //               << " )" << endl;

  const bool silent = mSilent;
  mSilent = true;
  KCal::Journal* j = mCalendar.journal( uid );
  if( j ) deleteNote( j );
  mSilent = silent;
}

void KNotesIMAP::ResourceIMAP::slotRefresh( const QString& type,
                                            const QString& /*resource*/ )
{
  if ( type == "Note" )
    load();
}

void KNotesIMAP::ResourceIMAP::subresourceAdded( const QString& type,
                                                 const QString& resource )
{
  if ( type != "Note" )
    // Not ours
    return;

  if ( mResources.contains( resource ) )
    // Not registered
    return;

  KConfig config( configFile() );
  config.setGroup( "Note" );

  mResources[ resource ] = config.readBoolEntry( resource, true );
  loadResource( resource );
  emit signalSubresourceAdded( this, type, resource );
}

void KNotesIMAP::ResourceIMAP::subresourceDeleted( const QString& type,
                                                   const QString& resource )
{
  if ( type != "Note" )
    // Not ours
    return;

  if ( !mResources.contains( resource ) )
    // Not registered
    return;

  // Ok, it's our job, and we have it here
  mResources.erase( resource );

  KConfig config( configFile() );
  config.setGroup( "Note" );
  config.deleteEntry( resource );
  config.sync();

  // Make a list of all uids to remove
  QMap<QString, QString>::ConstIterator mapIt;
  QStringList uids;
  for ( mapIt = mUidmap.begin(); mapIt != mUidmap.end(); ++mapIt )
    if ( mapIt.data() == resource )
      // We have a match
      uids << mapIt.key();

  // Finally delete all the incidences
  if ( !uids.isEmpty() ) {
    QStringList::ConstIterator it;
    for ( it = uids.begin(); it != uids.end(); ++it ) {
      KCal::Journal* j = mCalendar.journal( *it );
      if( j ) deleteNote( j );
      mUidmap.remove( *it );
    }
  }

  emit signalSubresourceRemoved( this, type, resource );
}

QStringList KNotesIMAP::ResourceIMAP::subresources() const
{
  return mResources.keys();
}

bool KNotesIMAP::ResourceIMAP::subresourceActive( const QString& res ) const
{
  if ( mResources.contains( res ) ) {
    return mResources[ res ];
  }

  // Safe default bet:
  kdDebug(5650) << "subresourceActive( " << res << " ): Safe bet\n";

  return true;
}


KCal::Journal* KNotesIMAP::ResourceIMAP::parseJournal( const QString& str )
{
  KCal::ICalFormat format;
  KCal::Incidence* i = format.fromString( str );
  if ( i ) {
    if ( i->type() == "Journal" )
      return static_cast<KCal::Journal*>( i );
    else {
      kdDebug(5500) << "Unknown incidence type " << i->type() << endl;
      delete i;
    }
  } else
    kdDebug(5500) << "Parse error\n";

  return 0;
}


void KNotesIMAP::ResourceIMAP::asyncLoadResult( const QStringList& lst, const QString& /*type*/,
                                    const QString& folder )
{
  /* No notification necessary? - till */
  populate( lst, folder );
}



#include "resourceimap.moc"
