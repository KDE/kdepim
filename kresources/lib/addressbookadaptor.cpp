/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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
*/

#include "addressbookadaptor.h"

#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#include <libkdepim/kabcresourcecached.h>
#include <kio/job.h>


using namespace KABC;


AddressBookUploadItem::AddressBookUploadItem( 
                                        KPIM::GroupwareDataAdaptor *adaptor, 
                                        KABC::Addressee addr, 
                                        GroupwareUploadItem::UploadType type )
    : GroupwareUploadItem( type )
{
  mItemType = KPIM::FolderLister::Contact;
  setUrl( addr.custom( adaptor->identifier(), "storagelocation" ) );
  setUid( addr.uid() );
  KABC::VCardConverter vcard;
  setData( vcard.createVCard( addr ) );
}



AddressBookAdaptor::AddressBookAdaptor()
{
}

QString AddressBookAdaptor::mimeType() const
{
  return "text/x-vcard";
}

bool AddressBookAdaptor::localItemExists( const QString &localId )
{
  KABC::Addressee a = mResource->findByUid( localId );
  return !a.isEmpty();
}

bool AddressBookAdaptor::localItemHasChanged( const QString &localId )
{
  KABC::Addressee::List addressees = mResource->deletedAddressees();
  KABC::Addressee::List::ConstIterator it;
  for( it = addressees.begin(); it != addressees.end(); ++it ) {
    if ( (*it).uid() == localId ) return true;
  }

  addressees = mResource->changedAddressees();
  for( it = addressees.begin(); it != addressees.end(); ++it ) {
    if ( (*it).uid() == localId ) return true;
  }

  return false;
}


void AddressBookAdaptor::deleteItem( const QString &localId )
{
  KABC::Addressee a = mResource->findByUid( localId );
  if ( !a.isEmpty() ) {
    mResource->removeAddressee( a );
    mResource->clearChange( a.uid() );
  }
}

void AddressBookAdaptor::addItem( KABC::Addressee addr )
{
  if ( !addr.isEmpty() ) {
    addr.setResource( mResource );
    mResource->insertAddressee( addr );
    clearChange( addr.uid() );
  }
}

void AddressBookAdaptor::addressbookItemDownloaded( KABC::Addressee addr,
    const QString &newLocalId, const QString &remoteId, const QString &fingerprint,
    const QString &storagelocation )
{
  // remove the currently existing item from the cache
  deleteItem( newLocalId );
  QString localId = idMapper()->localId( remoteId );
  if ( !localId.isEmpty() ) deleteItem( localId );
  
  // add the new item
  addr.insertCustom( identifier(), "storagelocation", storagelocation );
  if ( !localId.isEmpty() ) addr.setUid( localId );
  addItem( addr );
  
  // update the fingerprint and the ids in the idMapper
  idMapper()->removeRemoteId( localId );
  idMapper()->removeRemoteId( newLocalId );
  emit itemDownloaded( addr.uid(), remoteId, fingerprint );
}


void AddressBookAdaptor::clearChange( const QString &uid )
{
  mResource->clearChange( uid );
}

KPIM::GroupwareUploadItem *AddressBookAdaptor::newUploadItem( 
              KABC::Addressee addr, KPIM::GroupwareUploadItem::UploadType type )
{
  return new AddressBookUploadItem( this, addr, type );
}
