/*
    This file is part of the IMAP resources.
    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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
#include <kstandarddirs.h>

class IMAPFactory : public KRES::PluginFactoryBase
{
  public:
    KRES::Resource *resource( const KConfig *config )
    {
      return new KNotesIMAP::ResourceIMAP( config );
    }

    KRES::ConfigWidget *configWidget( QWidget* )
    {
      return 0;
    }
};

extern "C"
{
  void *init_knotes_imap()
  {
    return ( new IMAPFactory() );
  }
}


KNotesIMAP::ResourceIMAP::ResourceIMAP( const KConfig *config )
  : ResourceNotes( config ),
    ResourceIMAPBase::ResourceIMAPShared( "ResourceIMAP-KNotes" )
{
}

KNotesIMAP::ResourceIMAP::~ResourceIMAP()
{
}

bool KNotesIMAP::ResourceIMAP::doOpen()
{
  // Get the config file
  QString configFile = locateLocal( "config", "kresources/imap/knotesrc" );
  KConfig config( configFile );

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
  // Get the config file
  QString configFile = locateLocal( "config", "kresources/imap/kabcrc" );
  KConfig config( configFile );

  config.setGroup( "Note" );
  QMap<QString, bool>::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it )
    config.writeEntry( it.key(), it.data() );
}

bool KNotesIMAP::ResourceIMAP::load()
{
  // We get a fresh list of events, so clean out the old ones
  mCalendar.deleteAllEvents();
  mUidmap.clear();

  QMap<QString, bool>::ConstIterator itR;
  for ( itR = mResources.begin(); itR != mResources.end(); ++itR ) {
    if ( !itR.data() )
      // This resource is disabled
      continue;

    // Get the list of journals
    QStringList lst;
    if( !kmailIncidences( lst, "Note", itR.key() ) ) {
      kdError(5500) << "Communication problem in "
                    << "ResourceIMAP::getIncidenceList()\n";
      return false;
    }

    // Populate the calendar with the new events
    const bool silent = mSilent;
    mSilent = true;
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
      KCal::Journal* journal = parseJournal( *it );
      if( journal )
        addNote( journal, itR.key() );
    }
    mSilent = silent;
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
  manager()->registerNote( this, journal );
  journal->registerObserver( this );

  QString resource = subresource;
  if ( subresource.isEmpty() )
    // TODO: In which resource?
    resource = mResources.begin().key();
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
  // TODO: Optimize this
  slotRefresh( type, resource );
}

void KNotesIMAP::ResourceIMAP::subresourceDeleted( const QString& type,
                                                   const QString& resource )
{
  // TODO: Optimize this
  slotRefresh( type, resource );
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



#include "resourceimap.moc"
