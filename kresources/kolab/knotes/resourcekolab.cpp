/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

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

#include "resourcekolab.h"
#include "note.h"

#include <knotes/resourcemanager.h>

#include <libkcal/icalformat.h>

#include <kdebug.h>
#include <kglobal.h>

using namespace Kolab;


class KolabFactory : public KRES::PluginFactoryBase
{
public:
  KRES::Resource *resource( const KConfig *config )
  {
    return new ResourceKolab( config );
  }

  KRES::ConfigWidget *configWidget( QWidget* )
  {
    return 0;
  }
};

extern "C"
{
  void *init_knotes_kolab()
  {
    return ( new KolabFactory() );
  }
}

static const char* configGroupName = "Note";
static const char* kmailContentsType = "Note";
static const char* attachmentMimeType = "application/x-vnd.kolab.note";

ResourceKolab::ResourceKolab( const KConfig *config )
  : ResourceNotes( config ), ResourceKolabBase( "ResourceKolab-KNotes" )
{
}

ResourceKolab::~ResourceKolab()
{
}

bool ResourceKolab::doOpen()
{
  KConfig config( configFile() );
  config.setGroup( configGroupName );

  // Get the list of Notes folders from KMail
  QMap<QString, bool> resources;
  if ( !kmailSubresources( resources, kmailContentsType ) )
    return false;

  // Make the resource map from the folder list
  QMap<QString, bool>::ConstIterator it;
  mResources.clear();
  for ( it = resources.begin(); it != resources.end(); ++it ) {
    const QString resource = it.key();
    const bool active = config.readBoolEntry( it.key(), true );
    const bool writable = it.data();
    mResources[ resource ] = Kolab::SubResource( active, writable, resource );
  }

  return true;
}

void ResourceKolab::doClose()
{
  KConfig config( configFile() );
  config.setGroup( configGroupName );
  Kolab::ResourceMap::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it )
    config.writeEntry( it.key(), it.data().active() );
}

bool ResourceKolab::loadSubResource( const QString& resource )
{
  // Get the list of journals
  QMap<Q_UINT32, QString> lst;
  if( !kmailIncidences( lst, attachmentMimeType, resource ) ) {
    kdError(5500) << "Communication problem in "
                  << "ResourceKolab::getIncidenceList()\n";
    return false;
  }

  kdDebug(5500) << "Notes kolab resource: got " << lst.count() << " notes in " << resource << endl;

  // Populate with the new entries
  const bool silent = mSilent;
  mSilent = true;
  QMap<Q_UINT32, QString>::Iterator it;
  for ( it = lst.begin(); it != lst.end(); ++it ) {
    bool ret = addNote( it.data(), resource, it.key() );
    if ( !ret )
      kdDebug(5500) << "loading note " << it.key() << " failed" << endl;
  }
  mSilent = silent;

  return true;
}

bool ResourceKolab::load()
{
  // We get a fresh list of events, so clean out the old ones
  mCalendar.deleteAllEvents();
  mUidMap.clear();

  bool rc = true;
  Kolab::ResourceMap::ConstIterator itR;
  for ( itR = mResources.begin(); itR != mResources.end(); ++itR ) {
    if ( !itR.data().active() )
      // This resource is disabled
      continue;

    rc &= loadSubResource( itR.key() );
  }

  kdDebug() << k_funcinfo << " - done" << endl;
  return rc;
}

bool ResourceKolab::save()
{
  // Nothing to do here, we save everything in incidenceUpdated()
  return true;
}

bool ResourceKolab::addNote( KCal::Journal* journal )
{
  return addNote( journal, QString::null, 0 );
}

bool ResourceKolab::addNote( const QString& xml, const QString& subresource,
                             Q_UINT32 sernum )
{
  KCal::Journal* journal = Note::xmlToJournal( xml );
  Q_ASSERT( journal );
  Q_ASSERT( !mUidMap.contains( journal->uid() ) );
  if( journal && !mUidMap.contains( journal->uid() ) )
    return addNote( journal, subresource, sernum );
  return false;
}

bool ResourceKolab::addNote( KCal::Journal* journal,
                             const QString& subresource, Q_UINT32 sernum )
{
  kdDebug(5500) << "ResourceKolab::addNote( KCal::Journal*, '" << subresource << "', " << sernum << " )\n";

  journal->registerObserver( this );

  // Find out if this note was previously stored in KMail
  bool newNote = subresource.isEmpty();

  if ( !newNote ) {
    mCalendar.addJournal( journal );
  }

  QString resource =
    newNote ? findWritableResource( mResources ) : subresource;

  if ( !mSilent ) {
    QString xml = Note::journalToXML( journal );
    kdDebug(5500) << k_funcinfo << "XML string:\n" << xml << endl;

    if( !kmailUpdate( resource, sernum, xml, attachmentMimeType, journal->summary() ) ) {
      kdError(5500) << "Communication problem in ResourceKolab::addNote()\n";
      return false;
    }
  }

  if ( !resource.isEmpty() && sernum != 0 ) {
    mUidMap[ journal->uid() ] = StorageReference( resource, sernum );
    return true;
  }

  return false;
}

bool ResourceKolab::deleteNote( KCal::Journal* journal )
{
  const QString uid = journal->uid();
  if ( !mUidMap.contains( uid ) )
    // Odd
    return false;

  kmailDeleteIncidence( mUidMap[ uid ].resource(),
                        mUidMap[ uid ].serialNumber() );
  mUidMap.remove( uid );
  mCalendar.deleteJournal( journal );
  return true;
}

void ResourceKolab::incidenceUpdated( KCal::IncidenceBase* i )
{
  QString resource;
  Q_UINT32 sernum;
  if ( mUidMap.contains( i->uid() ) ) {
    resource = mUidMap[ i->uid() ].resource();
    sernum = mUidMap[ i->uid() ].serialNumber();
  } else {
    resource = findWritableResource( mResources );
    sernum = 0;
  }
  kdDebug() << k_funcinfo << "sernum=" << sernum << endl;

  KCal::Journal* journal = dynamic_cast<KCal::Journal*>( i );
  QString xml = Note::journalToXML( journal );
  if( !xml.isEmpty() && kmailUpdate( resource, sernum, xml, attachmentMimeType, journal->summary() ) )
    mUidMap[ i->uid() ] = StorageReference( resource, sernum );
}

/*
 * These are the DCOP slots that KMail call to notify when something
 * changed.
 */
bool ResourceKolab::fromKMailAddIncidence( const QString& type,
                                           const QString& resource,
                                           Q_UINT32 sernum,
                                           const QString& note )
{
  // Check if this is a note
  if( type != kmailContentsType ) return false;

  const bool silent = mSilent;
  mSilent = true;
  addNote( note, resource, sernum );
  mSilent = silent;

  return true;
}

void ResourceKolab::fromKMailDelIncidence( const QString& type,
                                           const QString& /*resource*/,
                                           const QString& note )
{
  // Check if this is a note
  if( type != kmailContentsType ) return;

  // kdDebug(5500) << "ResourceKolab::deleteIncidence( " << type << ", " << uid
  //               << " )" << endl;

  // ### this is just to load the uid - make it faster by parsing it here?
  KCal::Journal* journal = Note::xmlToJournal( note );
  if ( !journal )
    return;

  const bool silent = mSilent;
  mSilent = true;
  KCal::Journal* j = mCalendar.journal( journal->uid() );
  if( j ) deleteNote( j );
  mSilent = silent;

  delete journal;
}

void ResourceKolab::slotRefresh( const QString& type,
                                 const QString& /*resource*/ )
{
  if ( type == kmailContentsType )
    load();
}

void ResourceKolab::fromKMailAddSubresource( const QString& type,
                                             const QString& resource,
                                             bool writable )
{
  if ( type != kmailContentsType )
    // Not ours
    return;

  if ( mResources.contains( resource ) )
    // Already registered
    return;

  KConfig config( configFile() );
  config.setGroup( configGroupName );

  bool active = config.readBoolEntry( resource, true );
  mResources[ resource ] = Kolab::SubResource( active, writable, resource );
  loadSubResource( resource );
  emit signalSubresourceAdded( this, type, resource );
}

void ResourceKolab::fromKMailDelSubresource( const QString& type,
                                             const QString& resource )
{
  if ( type != configGroupName )
    // Not ours
    return;

  if ( !mResources.contains( resource ) )
    // Not registered
    return;

  // Ok, it's our job, and we have it here
  mResources.erase( resource );

  KConfig config( configFile() );
  config.setGroup( configGroupName );
  config.deleteEntry( resource );
  config.sync();

  // Make a list of all uids to remove
  Kolab::UidMap::ConstIterator mapIt;
  QStringList uids;
  for ( mapIt = mUidMap.begin(); mapIt != mUidMap.end(); ++mapIt )
    if ( mapIt.data().resource() == resource )
      // We have a match
      uids << mapIt.key();

  // Finally delete all the incidences
  if ( !uids.isEmpty() ) {
    QStringList::ConstIterator it;
    for ( it = uids.begin(); it != uids.end(); ++it ) {
      KCal::Journal* j = mCalendar.journal( *it );
      if( j ) deleteNote( j );
      mUidMap.remove( *it );
    }
  }

  emit signalSubresourceRemoved( this, type, resource );
}

QStringList ResourceKolab::subresources() const
{
  return mResources.keys();
}

bool ResourceKolab::subresourceActive( const QString& res ) const
{
  if ( mResources.contains( res ) ) {
    return mResources[ res ].active();
  }

  // Safe default bet:
  kdDebug(5650) << "subresourceActive( " << res << " ): Safe bet\n";

  return true;
}


#include "resourcekolab.moc"
