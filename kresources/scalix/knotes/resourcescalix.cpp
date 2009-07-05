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

#include "resourcescalix.h"

#include <knotes/resourcemanager.h>

#include <kcal/icalformat.h>

#include <kdebug.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <QList>

using namespace Scalix;

static const char* configGroupName = "Note";
static const char* kmailContentsType = "Note";
static const char* attachmentMimeType = "application/x-vnd.kolab.note";
static const char* inlineMimeType = "text/calendar";

ResourceScalix::ResourceScalix()
  : ResourceNotes(), ResourceScalixBase( "ResourceScalix_KNotes" ),
    mCalendar( QLatin1String("UTC") )
{
  setType( "scalix" );
}

ResourceScalix::ResourceScalix( const KConfigGroup& config )
  : ResourceNotes( config ), ResourceScalixBase( "ResourceScalix_KNotes" ),
    mCalendar( QLatin1String("UTC") )
{
  setType( "scalix" );
}

ResourceScalix::~ResourceScalix()
{
}

bool ResourceScalix::doOpen()
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
  for ( it = subResources.constBegin(); it != subResources.constEnd(); ++it ) {
    const QString subResource = (*it).location;
    const bool active = group.readEntry( subResource, true );
    mSubResources[ subResource ] = Scalix::SubResource( active, (*it).writable, (*it).label );
  }

  return true;
}

void ResourceScalix::doClose()
{
  KConfig config( configFile() );
  KConfigGroup group = config.group( configGroupName );
  Scalix::ResourceMap::ConstIterator it;
  for ( it = mSubResources.constBegin(); it != mSubResources.constEnd(); ++it )
    group.writeEntry( it.key(), it.value().active() );
}

bool ResourceScalix::loadSubResource( const QString& subResource,
                                     const QString &mimetype )
{
  // Get the list of journals
  int count = 0;
  if ( !kmailIncidencesCount( count, mimetype, subResource ) ) {
    kError() <<"Communication problem in ResourceScalix::load()";
    return false;
  }

  KMail::SernumDataPair::List lst;
  if( !kmailIncidences( lst, mimetype, subResource, 0, count ) ) {
    kError(5500) <<"Communication problem in"
                  << "ResourceScalix::getIncidenceList()\n";
    return false;
  }

  kDebug(5500) <<"Notes scalix resource: got" << lst.count() <<" notes in" << subResource;

  // Populate with the new entries
  const bool silent = mSilent;
  mSilent = true;
  KMail::SernumDataPair::List::ConstIterator it;
  for ( it = lst.constBegin(); it != lst.constEnd(); ++it ) {
    KCal::Journal* journal = addNote( it->data, subResource, it->sernum, mimetype );
    if ( !journal )
      kDebug(5500) <<"loading note" << it->sernum <<" failed";
    else
      manager()->registerNote( this, journal );
  }
  mSilent = silent;

  return true;
}

bool ResourceScalix::load()
{
  // We get a fresh list of events, so clean out the old ones
  mCalendar.deleteAllEvents();
  mUidMap.clear();

  bool rc = true;
  Scalix::ResourceMap::ConstIterator itR;
  for ( itR = mSubResources.constBegin(); itR != mSubResources.constEnd(); ++itR ) {
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

bool ResourceScalix::save()
{
  // Nothing to do here, we save everything in incidenceUpdated()
  return true;
}

bool ResourceScalix::addNote( KCal::Journal* journal )
{
  return addNote( journal, QString(), 0 );
}

KCal::Journal* ResourceScalix::addNote( const QString& data, const QString& subresource,
                             quint32 sernum, const QString &mimetype )
{
  KCal::Journal* journal = 0;
    // FIXME: This does not take into account the time zone!
  KCal::ICalFormat formatter;
  journal = static_cast<KCal::Journal*>( formatter.fromString( data ) );

  Q_ASSERT( journal );
  if( journal && !mUidMap.contains( journal->uid() ) ) {
    if ( addNote( journal, subresource, sernum ) ) {
      return journal;
    } else {
      delete journal;
    }
  }
  return 0;
}

bool ResourceScalix::addNote( KCal::Journal* journal,
                             const QString& subresource, quint32 sernum )
{
  kDebug(5500) <<"ResourceScalix::addNote( KCal::Journal*, '" << subresource <<"'," << sernum <<" )";

  journal->registerObserver( this );

  // Find out if this note was previously stored in KMail
  bool newNote = subresource.isEmpty();
  mCalendar.addJournal( journal );

  QString resource =
    newNote ? findWritableResource( mSubResources ) : subresource;
  if ( resource.isEmpty() ) // canceled
    return false;

  if ( !mSilent ) {
    KCal::ICalFormat formatter;
    const QString xml = formatter.toString( journal );
    kDebug(5500) <<"XML string:" << xml;

    if( !kmailUpdate( resource, sernum, xml, attachmentMimeType, journal->uid() ) ) {
      kError(5500) <<"Communication problem in ResourceScalix::addNote()";
      return false;
    }
  }

  if ( !resource.isEmpty() && sernum != 0 ) {
    mUidMap[ journal->uid() ] = StorageReference( resource, sernum );
    return true;
  }

  return false;
}

bool ResourceScalix::deleteNote( KCal::Journal* journal )
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

KCal::Alarm::List ResourceScalix::alarms( const KDateTime& from, const KDateTime& to )
{
    KCal::Alarm::List alarms;
    KCal::Journal::List notes = mCalendar.journals();
    KCal::Journal::List::ConstIterator note;
    for ( note = notes.constBegin(); note != notes.constEnd(); ++note )
    {
        KDateTime preTime = from.addSecs( -1 );
        KCal::Alarm::List::ConstIterator it;
        for( it = (*note)->alarms().constBegin(); it != (*note)->alarms().constEnd(); ++it )
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

void ResourceScalix::incidenceUpdated( KCal::IncidenceBase* i )
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
  KCal::ICalFormat formatter;
  const QString xml = formatter.toString( journal );
  if( !xml.isEmpty() && kmailUpdate( subResource, sernum, xml, attachmentMimeType, journal->uid() ) )
    mUidMap[ i->uid() ] = StorageReference( subResource, sernum );
}

/*
 * These are the DCOP slots that KMail call to notify when something
 * changed.
 */
bool ResourceScalix::fromKMailAddIncidence( const QString& type,
                                           const QString& subResource,
                                           quint32 sernum,
                                           int format,
                                           const QString& note )
{
  // Check if this is a note
  if( type != kmailContentsType ) return false;

  const bool silent = mSilent;
  mSilent = true;
  QString mimetype = inlineMimeType;
  KCal::Journal* journal = addNote( note, subResource, sernum, mimetype );
  if ( journal )
    manager()->registerNote( this, journal );
  mSilent = silent;
  return true;
}

void ResourceScalix::fromKMailDelIncidence( const QString& type,
                                           const QString& /*subResource*/,
                                           const QString& uid )
{
  // Check if this is a note
  if( type != kmailContentsType ) return;

  kDebug(5500) <<"ResourceScalix::fromKMailDelIncidence(" << type <<"," << uid
                << ")";

  const bool silent = mSilent;
  mSilent = true;
  KCal::Journal* j = mCalendar.journal( uid );
  if( j )
    deleteNote( j );
  mSilent = silent;
}

void ResourceScalix::fromKMailRefresh( const QString& type,
                                      const QString& /*subResource*/ )
{
  if ( type == kmailContentsType )
    load(); // ### should call loadSubResource(subResource) probably
}

void ResourceScalix::fromKMailAddSubresource( const QString& type,
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
  mSubResources[ subResource ] = Scalix::SubResource( active, writable, subResource );
  loadSubResource( subResource, mimetype );
  emit signalSubresourceAdded( this, type, subResource );
}

void ResourceScalix::fromKMailDelSubresource( const QString& type,
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
  Scalix::UidMap::ConstIterator mapIt;
  QStringList uids;
  for ( mapIt = mUidMap.constBegin(); mapIt != mUidMap.constEnd(); ++mapIt )
    if ( mapIt.value().resource() == subResource )
      // We have a match
      uids << mapIt.key();

  // Finally delete all the incidences
  if ( !uids.isEmpty() ) {
    const bool silent = mSilent;
    mSilent = true;
    QStringList::ConstIterator it;
    for ( it = uids.constBegin(); it != uids.constEnd(); ++it ) {
      KCal::Journal* j = mCalendar.journal( *it );
      if( j )
        deleteNote( j );
    }
    mSilent = silent;
  }

  emit signalSubresourceRemoved( this, type, subResource );
}

void ResourceScalix::fromKMailAsyncLoadResult( const QMap<quint32, QString>& map,
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
  for( QMap<quint32, QString>::ConstIterator it = map.constBegin(); it != map.constEnd(); ++it ) {
    KCal::Journal* journal = addNote( it.value(), folder, it.key(), mimetype );
    if ( !journal )
      kDebug(5500) <<"loading note" << it.key() <<" failed";
    else
      manager()->registerNote( this, journal );
  }
  mSilent = silent;
}


QStringList ResourceScalix::subresources() const
{
  return mSubResources.keys();
}

bool ResourceScalix::subresourceActive( const QString& res ) const
{
  if ( mSubResources.contains( res ) ) {
    return mSubResources[ res ].active();
  }

  // Safe default bet:
  kDebug(5650) <<"subresourceActive(" << res <<" ): Safe bet";

  return true;
}


#include "resourcescalix.moc"
