/*
    This file is part of libkabc and/or kaddressbook.
    Copyright (c) 2002 - 2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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

#include <kabc/formatfactory.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktempfile.h>

#include <qstring.h>

#include "resourcekolab.h"
#include <kmessagebox.h>


class KolabFactory : public KRES::PluginFactoryBase
{
  public:
    KRES::Resource *resource( const KConfig *config )
    {
      return new KABC::ResourceKolab( config );
    }

    KRES::ConfigWidget *configWidget( QWidget* )
    {
      return 0;
    }
};

extern "C"
{
  void *init_kabc_kolab()
  {
    return ( new KolabFactory() );
  }
}


KABC::ResourceKolab::ResourceKolab( const KConfig *config )
  : KPIM::ResourceABC( config ),
    Kolab::ResourceKolabBase( "ResourceKolab-KABC" )
{
  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( "vcard" );
}

KABC::ResourceKolab::~ResourceKolab()
{
  // The resource is deleted on exit (StdAddressBook's KStaticDeleter),
  // and it wasn't closed before that, so close here to save the config.
  if ( isOpen() ) {
    close();
  }
  delete mFormat;
}

void KABC::ResourceKolab::loadSubResourceConfig( KConfig& config,
                                                 const QString& name,
                                                 bool writable )
{
  KConfigGroup group( &config, name );
  bool active = group.readBoolEntry( "Active", true );
  int completionWeight = group.readNumEntry( "CompletionWeight", 80 );
  mResources.insert( name, Kolab::SubResource( active, writable, name,
                                               completionWeight ) );
}

bool KABC::ResourceKolab::doOpen()
{
  KConfig config( configFile() );

  // Read the calendar entries
  QMap<QString, bool> resources;
  if ( !kmailSubresources( resources, "Contact" ) )
    return false;
  mResources.clear();
  QMap<QString, bool>::ConstIterator it;
  for ( it = resources.begin(); it != resources.end(); ++it )
    loadSubResourceConfig( config, it.key(), it.data() );

  return true;
}

void KABC::ResourceKolab::doClose()
{
  KConfig config( configFile() );

  Kolab::ResourceMap::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it ) {
    config.setGroup( it.key() );
    config.writeEntry( "Active", it.data().active() );
    config.writeEntry( "CompletionWeight", it.data().completionWeight() );
  }
}

KABC::Ticket * KABC::ResourceKolab::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdError() << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void KABC::ResourceKolab::releaseSaveTicket( Ticket* ticket )
{
  delete ticket;
}

bool KABC::ResourceKolab::loadResource( const QString& resource )
{
  QMap<Q_UINT32, QString> lst;
  if ( !kmailIncidences( lst, "Contact", resource ) ) {
    kdError() << "Communication problem in ResourceKolab::load()\n";
    return false;
  }

#if 0
  for( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    KABC::Addressee addr = mConverter.parseVCard( *it );
    addr.setResource( this );
    addr.setChanged( false );
    Resource::insertAddressee( addr );
    mUidmap[ addr.uid() ] = resource;
  }
#endif

  return true;
}

bool KABC::ResourceKolab::load()
{
  mUidMap.clear();
  mAddrMap.clear();

  bool rc = true;
  Kolab::ResourceMap::ConstIterator itR;
  for ( itR = mResources.begin(); itR != mResources.end(); ++itR ) {
    if ( !itR.data().active() )
      // This resource is disabled
      continue;

    rc &= loadResource( itR.key() );
  }

  return rc;
}

bool KABC::ResourceKolab::save( Ticket* )
{
  bool rc = true;

#if 0 // TODO
  for( ConstIterator  it = begin(); it != end(); ++it )
    if( (*it).changed() ) {
      // First the uid and then the vCard
      const QString uid = (*it).uid();
      const QString vCard = mConverter.createVCard( *it );
      const QString resource = mUidMap[ uid ].resource();
      rc &= kmailUpdate( "Contact", resource, uid, vCard );
    }

  // Mark all of them as read
  for( Iterator it = begin(); it != end(); ++it )
    (*it).setChanged( false );
#endif

  return rc;
}

void KABC::ResourceKolab::insertAddressee( const Addressee& addr )
{
  // Call kmail ...
  if ( !mSilent ) {
#if 0 // TODO
    // Check if we need to update or save this
    bool update = false;
    if( mAddrMap.contains( addr.uid() ) ) {
      // This address is already in the map
      if( !addr.changed() )
        // Not changed, no need to save it
        return;
      else
        update = true;
    }

    const QString vCard = mConverter.createVCard( addr );
    const QString uid = addr.uid();

    bool rc;
    if( !update ) {
      // Save the new addressee
      const QString resource = findWritableResource( mResources, "Contact" );
      rc = kmailAddIncidence( "Contact", resource, uid, vCard );
      mUidMap[ uid ] = resource;
    } else
      // Update existing addressee
      rc = kmailUpdate( "Contact", mUidMap[ uid ], uid, vCard );

    if( rc )
        // This is ugly, but it's faster than doing
        // mAddrMap.find(addr.uid()), which would give the same :-(
        // Reason for this: The Changed attribute of Addressee should
        // be mutable
        const_cast<Addressee&>(addr).setChanged( false );
#endif
  }

  Resource::insertAddressee( addr );
}

void KABC::ResourceKolab::removeAddressee( const Addressee& addr )
{
#if 0 // TODO
  kmailDeleteIncidence( "Contact", mUidMap[ addr.uid() ], addr.uid() );
  mUidMap.remove( addr.uid() );
#endif
  Resource::removeAddressee( addr );
}

/*
 * These are the DCOP slots that KMail call to notify when something
 * changed.
 */
bool KABC::ResourceKolab::fromKMailAddIncidence( const QString& type,
                                                 const QString& resource,
                                                 Q_UINT32 sernum,
                                                 const QString& contact )
{
#if 0
  if( type == "Contact" ) {
    const bool silent = mSilent;
    mSilent = true;

    KABC::Addressee addr = mConverter.parseVCard( vCard );
    addr.setResource( this );
    addr.setChanged( false );
    mAddrMap.insert( addr.uid(), addr );
    mUidMap[ addr.uid() ] = resource;

    addressBook()->emitAddressBookChanged();

    mSilent = silent;

    return true;
  }
#endif

  return false;
}

void KABC::ResourceKolab::fromKMailDelIncidence( const QString& type,
                                                 const QString& /*resource*/,
                                                 const QString& uid )
{
  if( type == "Contact" ) {
    const bool silent = mSilent;
    mSilent = true;

    mAddrMap.remove( uid );
    mUidMap.remove( uid );
    addressBook()->emitAddressBookChanged();

    mSilent = silent;
  }
}

void KABC::ResourceKolab::slotRefresh( const QString& type,
                                      const QString& /*resource*/ )
{
  if( type == "Contact" ) {
    const bool silent = mSilent;
    mSilent = true;

    load();
    addressBook()->emitAddressBookChanged();

    mSilent = silent;
  }
}

void KABC::ResourceKolab::fromKMailAddSubresource( const QString& type,
                                                   const QString& resource,
                                                   bool writable )
{
  if ( type != "Contact" )
    // Not ours
    return;

  if ( mResources.contains( resource ) )
    // Already registered
    return;

  KConfig config( configFile() );
  config.setGroup( "Contact" );
  loadSubResourceConfig( config, resource, writable );
  loadResource( resource );
  addressBook()->emitAddressBookChanged();
  emit signalSubresourceAdded( this, type, resource );
}

void KABC::ResourceKolab::fromKMailDelSubresource( const QString& type,
                                                   const QString& resource )
{
  if ( type != "Contact" )
    // Not ours
    return;

  if ( !mResources.contains( resource ) )
    // Not registered
    return;

  // Ok, it's our job, and we have it here
  mResources.erase( resource );

  KConfig config( configFile() );
  config.deleteGroup( resource );
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
      mAddrMap.remove( *it );
      mUidMap.remove( *it );
    }

    addressBook()->emitAddressBookChanged();
  }

  emit signalSubresourceRemoved( this, type, resource );
}

QStringList KABC::ResourceKolab::subresources() const
{
  return mResources.keys();
}

bool KABC::ResourceKolab::subresourceActive( const QString& subresource ) const
{
  if ( mResources.contains( subresource ) ) {
    return mResources[ subresource ].active();
  }

  // Safe default bet:
  kdDebug(5650) << "subresourceActive( " << subresource << " ): Safe bet\n";

  return true;
}

int KABC::ResourceKolab::subresourceCompletionWeight( const QString& subresource ) const
{
  if ( mResources.contains( subresource ) ) {
    return mResources[ subresource ].completionWeight();
  }

  kdDebug(5650) << "subresourceCompletionWeight( " << subresource << " ): not found, using default\n";

  return 80;
}

void KABC::ResourceKolab::setSubresourceCompletionWeight( const QString& subresource, int completionWeight )
{
  if ( mResources.contains( subresource ) ) {
    mResources[ subresource ].setCompletionWeight( completionWeight );
  } else {
    kdDebug(5650) << "setSubresourceCompletionWeight: subresource " << subresource << " not found" << endl;
  }
}

QMap<QString, QString> KABC::ResourceKolab::uidToResourceMap() const
{
  // TODO: Couldn't this be made simpler?
  QMap<QString, QString> map;
  Kolab::UidMap::ConstIterator mapIt;
  for ( mapIt = mUidMap.begin(); mapIt != mUidMap.end(); ++mapIt )
    map[ mapIt.key() ] = mapIt.data().resource();
  return map;
}


#include "resourcekolab.moc"
