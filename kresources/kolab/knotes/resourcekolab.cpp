/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

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

#include <kcal/icalformat.h>

#include <kdebug.h>
#include <kglobal.h>
#include <QList>

using namespace Kolab;

static const char* configGroupName = "Note";
static const char* kmailContentsType = "Note";
static const char* attachmentMimeType = "application/x-vnd.kolab.note";
static const char* inlineMimeType = "text/calendar";

ResourceKolab::ResourceKolab()
  : ResourceNotes(), ResourceKolabBase( "ResourceKolab_KNotes" ),
    mCalendar( QLatin1String("UTC") )
{
  setType( "imap" );
}

ResourceKolab::ResourceKolab( const KConfigGroup& config )
  : ResourceNotes( config ), ResourceKolabBase( "ResourceKolab_KNotes" ),
    mCalendar( QLatin1String("UTC") )
{
  setType( "imap" );
}

ResourceKolab::~ResourceKolab()
{
}

bool ResourceKolab::doOpen()
{
  KConfig config( configFile() );
  KConfigGroup group = config.group( configGroupName );

  // Get the list of Notes folders from KMail
  QList<KMail::SubResource> subResources;
  if ( !kmailSubresources( subResources, QString::fromLatin1(kmailContentsType) ) )
    return false;

  // Make the resource map from the folder list
  QList<KMail::SubResource>::ConstIterator it;
  mSubResources.clear();
  for ( it = subResources.begin(); it != subResources.end(); ++it ) {
    const QString subResource = (*it).location;
    const bool active = group.readEntry( subResource, true );
    mSubResources[ subResource ] = Kolab::SubResource( active, (*it).writable, (*it).label );
  }

  return true;
}

void ResourceKolab::doClose()
{
  KConfig config( configFile() );
  KConfigGroup group = config.group( configGroupName );
  Kolab::ResourceMap::ConstIterator it;
  for ( it = mSubResources.begin(); it != mSubResources.end(); ++it )
    group.writeEntry( it.key(), it.value().active() );
}

bool ResourceKolab::loadSubResource( const QString& subResource,
                                     const QString &mimetype )
{
  // Get the list of journals
  int count = 0;
  if ( !kmailIncidencesCount( count, mimetype, subResource ) ) {
    kError() << "Communication problem in ResourceKolab::load()\n";
    return false;
  }

  QMap<quint32, QString> lst;
  if( !kmailIncidences( lst, mimetype, subResource, 0, count ) ) {
    kError(5500) << "Communication problem in "
                  << "ResourceKolab::getIncidenceList()\n";
    return false;
  }

  kDebug(5500) << "Notes kolab resource: got " << lst.count() << " notes in " << subResource << endl;

  // Populate with the new entries
  const bool silent = mSilent;
  mSilent = true;
  QMap<quint32, QString>::Iterator it;
  for ( it = lst.begin(); it != lst.end(); ++it ) {
    KCal::Journal* journal = addNote( it.value(), subResource, it.key(), mimetype );
    if ( !journal )
      kDebug(5500) << "loading note " << it.key() << " failed" << endl;
    else
      manager()->registerNote( this, journal );
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
  for ( itR = mSubResources.begin(); itR != mSubResources.end(); ++itR ) {
    if ( !itR.value().active() )
      // This subResource is disabled
      continue;

    QString mimetype = inlineMimeType;
    rc &= loadSubResource( itR.key(), mimetype );
    mimetype = attachmentMimeType;
    rc &= loadSubResource( itR.key(), mimetype );
  }

  return rc;
}

bool ResourceKolab::save()
{
  // Nothing to do here, we save everything in incidenceUpdated()
  return true;
}

bool ResourceKolab::addNote( KCal::Journal* journal )
{
  return addNote( journal, QString(), 0 );
}

KCal::Journal* ResourceKolab::addNote( const QString& data, const QString& subresource,
                             quint32 sernum, const QString &mimetype )
{
  KCal::Journal* journal = 0;
    // FIXME: This does not take into account the time zone!
  KCal::ICalFormat formatter;
  if ( mimetype == attachmentMimeType )
    journal = Note::xmlToJournal( data );
  else
    journal = static_cast<KCal::Journal*>( formatter.fromString( data ) );

  Q_ASSERT( journal );
  if( journal && !mUidMap.contains( journal->uid() ) )
    if ( addNote( journal, subresource, sernum ) )
      return journal;
    else
      delete journal;
  return 0;
}

bool ResourceKolab::addNote( KCal::Journal* journal,
                             const QString& subresource, quint32 sernum )
{
  kDebug(5500) << "ResourceKolab::addNote( KCal::Journal*, '" << subresource << "', " << sernum << " )\n";

  journal->registerObserver( this );

  // Find out if this note was previously stored in KMail
  bool newNote = subresource.isEmpty();
  mCalendar.addJournal( journal );

  QString resource =
    newNote ? findWritableResource( mSubResources ) : subresource;
  if ( resource.isEmpty() ) // canceled
    return false;

  if ( !mSilent ) {
    QString xml = Note::journalToXML( journal );
    kDebug(5500) << k_funcinfo << "XML string:\n" << xml << endl;

    if( !kmailUpdate( resource, sernum, xml, attachmentMimeType, journal->uid() ) ) {
      kError(5500) << "Communication problem in ResourceKolab::addNote()\n";
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

  if ( !mSilent ) {
    kmailDeleteIncidence( mUidMap[ uid ].resource(),
                          mUidMap[ uid ].serialNumber() );
  }
  mUidMap.remove( uid );
  manager()->deleteNote( journal );
  mCalendar.deleteJournal( journal );
  return true;
}

KCal::Alarm::List ResourceKolab::alarms( const KDateTime& from, const KDateTime& to )
{
    KCal::Alarm::List alarms;
    KCal::Journal::List notes = mCalendar.journals();
    KCal::Journal::List::ConstIterator note;
    for ( note = notes.begin(); note != notes.end(); ++note )
    {
        KDateTime preTime = from.addSecs( -1 );
        KCal::Alarm::List::ConstIterator it;
        for( it = (*note)->alarms().begin(); it != (*note)->alarms().end(); ++it )
        {
            if ( (*it)->enabled() )
            {
                KDateTime dt = (*it)->nextRepetition( preTime );
                if ( dt.isValid() && dt <= to )
                    alarms.append( *it );
            }
        }
    }

    return alarms;
}

void ResourceKolab::incidenceUpdated( KCal::IncidenceBase* i )
{
  QString subResource;
  quint32 sernum;
  if ( mUidMap.contains( i->uid() ) ) {
    subResource = mUidMap[ i->uid() ].resource();
    sernum = mUidMap[ i->uid() ].serialNumber();
  } else { // can this happen?
    subResource = findWritableResource( mSubResources );
    if ( subResource.isEmpty() ) // canceled
      return;
    sernum = 0;
  }

  KCal::Journal* journal = static_cast<KCal::Journal*>( i );
  QString xml = Note::journalToXML( journal );
  if( !xml.isEmpty() && kmailUpdate( subResource, sernum, xml, attachmentMimeType, journal->uid() ) )
    mUidMap[ i->uid() ] = StorageReference( subResource, sernum );
}

/*
 * These are the DCOP slots that KMail call to notify when something
 * changed.
 */
bool ResourceKolab::fromKMailAddIncidence( const QString& type,
                                           const QString& subResource,
                                           quint32 sernum,
                                           int format,
                                           const QString& note )
{
  // Check if this is a note
  if( type != kmailContentsType ) return false;

  const bool silent = mSilent;
  mSilent = true;
  QString mimetype;
  if ( format == KMail::StorageXML )
    mimetype = attachmentMimeType;
  else
    mimetype = inlineMimeType;
  KCal::Journal* journal = addNote( note, subResource, sernum, mimetype );
  if ( journal )
    manager()->registerNote( this, journal );
  mSilent = silent;
  return true;
}

void ResourceKolab::fromKMailDelIncidence( const QString& type,
                                           const QString& /*subResource*/,
                                           const QString& uid )
{
  // Check if this is a note
  if( type != kmailContentsType ) return;

  kDebug(5500) << "ResourceKolab::fromKMailDelIncidence( " << type << ", " << uid
                << " )" << endl;

  const bool silent = mSilent;
  mSilent = true;
  KCal::Journal* j = mCalendar.journal( uid );
  if( j )
    deleteNote( j );
  mSilent = silent;
}

void ResourceKolab::fromKMailRefresh( const QString& type,
                                      const QString& /*subResource*/ )
{
  if ( type == kmailContentsType )
    load(); // ### should call loadSubResource(subResource) probably
}

void ResourceKolab::fromKMailAddSubresource( const QString& type,
                                             const QString& subResource,
                                             const QString& mimetype,
                                             bool writable,
                                             bool /*alarmRelevant*/ )
{
  if ( type != kmailContentsType )
    // Not ours
    return;

  if ( mSubResources.contains( subResource ) )
    // Already registered
    return;

  KConfig config( configFile() );
  KConfigGroup group = config.group( configGroupName );

  bool active = group.readEntry( subResource, true );
  mSubResources[ subResource ] = Kolab::SubResource( active, writable, subResource );
  loadSubResource( subResource, mimetype );
  emit signalSubresourceAdded( this, type, subResource );
}

void ResourceKolab::fromKMailDelSubresource( const QString& type,
                                             const QString& subResource )
{
  if ( type != configGroupName )
    // Not ours
    return;

  if ( !mSubResources.contains( subResource ) )
    // Not registered
    return;

  // Ok, it's our job, and we have it here
  mSubResources.remove( subResource );

  KConfig config( configFile() );
  KConfigGroup group = config.group( configGroupName );
  group.deleteEntry( subResource );
  group.sync();

  // Make a list of all uids to remove
  Kolab::UidMap::ConstIterator mapIt;
  QStringList uids;
  for ( mapIt = mUidMap.begin(); mapIt != mUidMap.end(); ++mapIt )
    if ( mapIt.value().resource() == subResource )
      // We have a match
      uids << mapIt.key();

  // Finally delete all the incidences
  if ( !uids.isEmpty() ) {
    const bool silent = mSilent;
    mSilent = true;
    QStringList::ConstIterator it;
    for ( it = uids.begin(); it != uids.end(); ++it ) {
      KCal::Journal* j = mCalendar.journal( *it );
      if( j )
        deleteNote( j );
    }
    mSilent = silent;
  }

  emit signalSubresourceRemoved( this, type, subResource );
}

void ResourceKolab::fromKMailAsyncLoadResult( const QMap<quint32, QString>& map,
                                              const QString& type,
                                              const QString& folder )
{
  // We are only interested in notes
  if ( ( type != attachmentMimeType ) && ( type != inlineMimeType ) ) return;
  // Populate with the new entries
  const bool silent = mSilent;
  mSilent = true;
  QString mimetype;
  if ( kmailStorageFormat( folder ) == KMail::StorageXML )
    mimetype = attachmentMimeType;
  else
    mimetype = inlineMimeType;
  for( QMap<quint32, QString>::ConstIterator it = map.begin(); it != map.end(); ++it ) {
    KCal::Journal* journal = addNote( it.value(), folder, it.key(), mimetype );
    if ( !journal )
      kDebug(5500) << "loading note " << it.key() << " failed" << endl;
    else
      manager()->registerNote( this, journal );
  }
  mSilent = silent;
}


QStringList ResourceKolab::subresources() const
{
  return mSubResources.keys();
}

bool ResourceKolab::subresourceActive( const QString& res ) const
{
  if ( mSubResources.contains( res ) ) {
    return mSubResources[ res ].active();
  }

  // Safe default bet:
  kDebug(5650) << "subresourceActive( " << res << " ): Safe bet\n";

  return true;
}


#include "resourcekolab.moc"
