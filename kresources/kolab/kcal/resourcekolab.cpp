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

#include <kabc/locknull.h>

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
  void *init_kcal_kolab()
  {
    return ( new KolabFactory() );
  }
}

static const char* kmailEventContentsType = "Event";
static const char* kmailTodoContentsType = "Task";
static const char* kmailJournalContentsType = "Journal";
static const char* eventAttachmentMimeType = "application/x-vnd.kolab.event";
static const char* todoAttachmentMimeType = "application/x-vnd.kolab.task";
static const char* journalAttachmentMimeType = "application/x-vnd.kolab.journal";

ResourceKolab::ResourceKolab( const KConfig *config )
  : ResourceCalendar( config ), ResourceKolabBase( "ResourceKolab-libkcal" ),
    mOpen( false )
{
}

ResourceKolab::~ResourceKolab()
{
  // The resource is deleted on exit (StdAddressBook's KStaticDeleter),
  // and it wasn't closed before that, so close here to save the config.
  if ( isOpen() ) {
    close();
  }
}

void ResourceKolab::loadSubResourceConfig( KConfig& config,
                                           const QString& name, bool writable,
                                           ResourceMap& subResource )
{
  KConfigGroup group( &config, name );
  bool active = group.readBoolEntry( "Active", true );
  subResource.insert( name, Kolab::SubResource( active, writable, name ) );
}

bool ResourceKolab::openResource( KConfig& config, const char* contentType,
                                  ResourceMap& map )
{
  // Read the subresource entries from KMail
  QMap<QString, bool> subResources;
  if ( !kmailSubresources( subResources, contentType ) )
    return false;
  map.clear();
  QMap<QString, bool>::ConstIterator it;
  for ( it = subResources.begin(); it != subResources.end(); ++it )
    loadSubResourceConfig( config, it.key(), it.data(), map );
  return true;
}

bool ResourceKolab::doOpen()
{
  if ( mOpen )
    // Already open
    return true;
  mOpen = true;

  KConfig config( configFile() );

  return openResource( config, kmailEventContentsType, mEventSubResources )
    && openResource( config, kmailTodoContentsType, mTodoSubResources )
    && openResource( config, kmailJournalContentsType, mJournalSubResources );
}

static void closeResource( KConfig& config, ResourceMap& map )
{
  ResourceMap::ConstIterator it;
  for ( it = map.begin(); it != map.end(); ++it ) {
    config.setGroup( it.key() );
    config.writeEntry( "Active", it.data().active() );
  }
}

void ResourceKolab::doClose()
{
  if ( !mOpen )
    // Not open
    return;
  mOpen = false;

  KConfig config( configFile() );
  closeResource( config, mEventSubResources );
  closeResource( config, mTodoSubResources );
  closeResource( config, mJournalSubResources );
}

bool ResourceKolab::loadSubResource( const QString& subResource,
                                     const char* mimetype )
{
  QMap<Q_UINT32, QString> lst;
  if ( !kmailIncidences( lst, mimetype, subResource ) ) {
    kdError() << "Communication problem in ResourceKolab::load()\n";
    return false;
  }

  kdDebug() << "Kolab resource: got " << lst.count() << " in "
                << subResource << " of type " << mimetype << endl;

  for( QMap<Q_UINT32, QString>::ConstIterator it = lst.begin(); it != lst.end(); ++it )
    addIncidence( mimetype, it.data(), subResource, it.key() );

  return true;
}

bool ResourceKolab::load()
{
  mUidMap.clear();

  return loadAllEvents() & loadAllTodos() & loadAllJournals();
}

bool ResourceKolab::doLoadAll( ResourceMap& map, const char* mimetype )
{
  bool rc = true;
  for ( ResourceMap::ConstIterator it = map.begin(); it != map.end(); ++it ) {
    if ( !it.data().active() )
      // This resource is disabled
      continue;

    rc &= loadSubResource( it.key(), mimetype );
  }
  return rc;
}

bool ResourceKolab::loadAllEvents()
{
  mCalendar.deleteAllEvents();
  return doLoadAll( mEventSubResources, eventAttachmentMimeType );
}

bool ResourceKolab::loadAllTodos()
{
  mCalendar.deleteAllTodos();
  return doLoadAll( mTodoSubResources, todoAttachmentMimeType );
}

bool ResourceKolab::loadAllJournals()
{
  mCalendar.deleteAllJournals();
  return doLoadAll( mJournalSubResources, journalAttachmentMimeType );
}

bool ResourceKolab::save()
{
  return true;
}

bool ResourceKolab::fromKMailAddIncidence( const QString& type,
                                           const QString& subResource,
                                           Q_UINT32 sernum,
                                           const QString& contact )
{
#if 0
  // TODO: Implement the thing
  // Check if this is a contact
  if( type != kmailContentsType ) return false;

  const bool silent = mSilent;
  mSilent = true;

  Addressee addr = Contact::xmlToAddressee( contact );
  addr.setResource( this );
  addr.setChanged( false );
  //mAddrMap.insert( addr.uid(), addr );
  mUidMap[ addr.uid() ] = StorageReference( subResource, sernum );

  addressBook()->emitAddressBookChanged();

  mSilent = silent;
#endif

  return true;
}

void ResourceKolab::fromKMailDelIncidence( const QString& type,
                                           const QString& /*subResource*/,
                                           const QString& uid )
{
#if 0
  // Check if this is a contact
  if( type != kmailContentsType ) return;

  const bool silent = mSilent;
  mSilent = true;

  //mAddrMap.remove( uid );
  mUidMap.remove( uid );
  addressBook()->emitAddressBookChanged();

  mSilent = silent;
#endif
}

void ResourceKolab::slotRefresh( const QString& type,
                                 const QString& /*subResource*/ )
{
#if 0
  // Check if this is a contact
  if( type != kmailContentsType ) return;

  const bool silent = mSilent;
  mSilent = true;

  load(); // ### should call loadSubResource(subResource) probably
  addressBook()->emitAddressBookChanged();

  mSilent = silent;
#endif
}

void ResourceKolab::fromKMailAddSubresource( const QString& type,
                                             const QString& subResource,
                                             bool writable )
{
#if 0
  if( type != kmailContentsType ) return;

  if ( mSubResources.contains( subResource ) )
    // Already registered
    return;

  KConfig config( configFile() );
  config.setGroup( "Contact" );
  loadSubResourceConfig( config, subResource, writable );
  loadSubResource( subResource );
  addressBook()->emitAddressBookChanged();
  emit signalSubresourceAdded( this, type, subResource );
#endif
}

void ResourceKolab::fromKMailDelSubresource( const QString& type,
                                             const QString& subResource )
{
#if 0
  if( type != kmailContentsType ) return;

  if ( !mSubResources.contains( subResource ) )
    // Not registered
    return;

  // Ok, it's our job, and we have it here
  mSubResources.erase( subResource );

  KConfig config( configFile() );
  config.deleteGroup( subResource );
  config.sync();

  // Make a list of all uids to remove
  Kolab::UidMap::ConstIterator mapIt;
  QStringList uids;
  for ( mapIt = mUidMap.begin(); mapIt != mUidMap.end(); ++mapIt )
    if ( mapIt.data().resource() == subResource )
      // We have a match
      uids << mapIt.key();

  // Finally delete all the incidences
  if ( !uids.isEmpty() ) {
    QStringList::ConstIterator it;
    for ( it = uids.begin(); it != uids.end(); ++it ) {
      //mAddrMap.remove( *it );
      mUidMap.remove( *it );
    }

    addressBook()->emitAddressBookChanged();
  }

  emit signalSubresourceRemoved( this, type, subResource );
#endif
}

QStringList ResourceKolab::subresources() const
{
  return mEventSubResources.keys() + mTodoSubResources.keys()
    + mJournalSubResources.keys();
}

bool ResourceKolab::subresourceActive( const QString& subresource ) const
{
  // Workaround: The ResourceView in KOrganizer wants to know this
  // before it opens the resource :-( Make sure we are open
  const_cast<ResourceKolab*>( this )->doOpen();

  if ( mEventSubResources.contains( subresource ) )
    return mEventSubResources[ subresource ].active();
  if ( mTodoSubResources.contains( subresource ) )
    return mTodoSubResources[ subresource ].active();
  if ( mJournalSubResources.contains( subresource ) )
    return mJournalSubResources[ subresource ].active();

  // Safe default bet:
  kdDebug(5650) << "subresourceActive( " << subresource << " ): Safe bet\n";

  return true;
}

KABC::Lock* ResourceKolab::lock()
{
  return new KABC::LockNull( true );
}


#include "resourcekolab.moc"
