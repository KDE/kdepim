/*
    This file is part of libkabc and/or kaddressbook.
    Copyright (c) 2002 - 2004 Klar�lvdalens Datakonsult AB
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

#include "resourceimap.h"
#include <kmessagebox.h>


class IMAPFactory : public KRES::PluginFactoryBase
{
  public:
    KRES::Resource *resource( const KConfig *config )
    {
      return new KABC::ResourceIMAP( config );
    }

    KRES::ConfigWidget *configWidget( QWidget* )
    {
      return 0;
    }
};

extern "C"
{
  void *init_kabc_imap()
  {
    return ( new IMAPFactory() );
  }
}


KABC::ResourceIMAP::ResourceIMAP( const KConfig *config )
  : KABC::Resource( config ),
    ResourceIMAPBase::ResourceIMAPShared( "ResourceIMAP-KABC" )
{
  FormatFactory *factory = FormatFactory::self();
  mFormat = factory->format( "vcard" );
}

KABC::ResourceIMAP::~ResourceIMAP()
{
  delete mFormat;
}

bool KABC::ResourceIMAP::doOpen()
{
  KConfig config( configFile() );
  config.setGroup( "Contact" );

  // Read the calendar entries
  QStringList resources;
  if ( !kmailSubresources( resources, "Contact" ) )
    return false;
  QStringList::ConstIterator it;
  mResources.clear();
  for ( it = resources.begin(); it != resources.end(); ++it )
    mResources[ *it ] = config.readBoolEntry( *it, true );

  return true;
}

void KABC::ResourceIMAP::doClose()
{
  KConfig config( configFile() );
  config.setGroup( "Contact" );

  QMap<QString, bool>::ConstIterator it;
  for ( it = mResources.begin(); it != mResources.end(); ++it )
    config.writeEntry( it.key(), it.data() );
}

KABC::Ticket * KABC::ResourceIMAP::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdError() << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void KABC::ResourceIMAP::releaseSaveTicket( Ticket* ticket )
{
  delete ticket;
}

bool KABC::ResourceIMAP::loadResource( const QString& resource )
{
  QStringList lst;
  if ( !kmailIncidences( lst, "Contact", resource ) ) {
    kdError() << "Communication problem in ResourceIMAP::load()\n";
    return false;
  }

  for( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    KABC::Addressee addr = mConverter.parseVCard( *it );
    addr.setResource( this );
    addr.setChanged( false );
    Resource::insertAddressee( addr );
    mUidmap[ addr.uid() ] = resource;
  }

  return true;
}

bool KABC::ResourceIMAP::load()
{
  mUidmap.clear();
  mAddrMap.clear();

  bool rc = true;
  QMap<QString, bool>::ConstIterator itR;
  for ( itR = mResources.begin(); itR != mResources.end(); ++itR ) {
    if ( !itR.data() )
      // This resource is disabled
      continue;

    rc &= loadResource( itR.key() );
  }

  return rc;
}

bool KABC::ResourceIMAP::save( Ticket* )
{
  bool rc = true;

  for( ConstIterator  it = begin(); it != end(); ++it )
    if( (*it).changed() ) {
      // First the uid and then the vCard
      const QString uid = (*it).uid();
      const QString vCard = mConverter.createVCard( *it );
      const QString resource = mUidmap[ uid ];
      rc &= kmailUpdate( "Contact", resource, uid, vCard );
    }

  // Mark all of them as read
  for( Iterator it = begin(); it != end(); ++it )
    (*it).setChanged( false );

  return rc;
}

void KABC::ResourceIMAP::insertAddressee( const Addressee& addr )
{
  // Call kmail ...
  if ( !mSilent ) {
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
      mUidmap[ uid ] = resource;
    } else
      // Update existing addressee
      rc = kmailUpdate( "Contact", mUidmap[ uid ], uid, vCard );

    if( rc )
        // This is ugly, but it's faster than doing
        // mAddrMap.find(addr.uid()), which would give the same :-(
        // Reason for this: The Changed attribute of Addressee should
        // be mutable
        const_cast<Addressee&>(addr).setChanged( false );
  }

  Resource::insertAddressee( addr );
}

void KABC::ResourceIMAP::removeAddressee( const Addressee& addr )
{
  kmailDeleteIncidence( "Contact", mUidmap[ addr.uid() ], addr.uid() );
  mUidmap.remove( addr.uid() );
  Resource::removeAddressee( addr );
}

/*
 * These are the DCOP slots that KMail call to notify when something
 * changed.
 */
bool KABC::ResourceIMAP::addIncidence( const QString& type,
                                       const QString& resource,
				       const QString& vCard )
{
  if( type == "Contact" ) {
    const bool silent = mSilent;
    mSilent = true;

    KABC::Addressee addr = mConverter.parseVCard( vCard );
    addr.setResource( this );
    addr.setChanged( false );
    mAddrMap.insert( addr.uid(), addr );
    mUidmap[ addr.uid() ] = resource;

    addressBook()->emitAddressBookChanged();

    mSilent = silent;

    return true;
  }

  return false;
}

void KABC::ResourceIMAP::deleteIncidence( const QString& type,
                                          const QString& /*resource*/,
					  const QString& uid )
{
  if( type == "Contact" ) {
    const bool silent = mSilent;
    mSilent = true;

    mAddrMap.remove( uid );
    mUidmap.remove( uid );
    addressBook()->emitAddressBookChanged();

    mSilent = silent;
  }
}

void KABC::ResourceIMAP::slotRefresh( const QString& type,
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

void KABC::ResourceIMAP::subresourceAdded( const QString& type,
                                           const QString& resource )
{
  if ( type != "Contact" )
    // Not ours
    return;

  if ( mResources.contains( resource ) )
    // Already registered
    return;

  KConfig config( configFile() );
  config.setGroup( "Contact" );

  mResources[ resource ] = config.readBoolEntry( resource, true );
  loadResource( resource );
  addressBook()->emitAddressBookChanged();
  emit signalSubresourceAdded( this, type, resource );
}

void KABC::ResourceIMAP::subresourceDeleted( const QString& type,
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
  config.setGroup( "Contact" );
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
      mAddrMap.remove( *it );
      mUidmap.remove( *it );
    }

    addressBook()->emitAddressBookChanged();
  }

  emit signalSubresourceRemoved( this, type, resource );
}

QStringList KABC::ResourceIMAP::subresources() const
{
  return mResources.keys();
}

bool KABC::ResourceIMAP::subresourceActive( const QString& subresource ) const
{
  if ( mResources.contains( subresource ) ) {
    return mResources[ subresource ];
  }

  // Safe default bet:
  kdDebug(5650) << "subresourceActive( " << subresource << " ): Safe bet\n";

  return true;
}


#include "resourceimap.moc"
