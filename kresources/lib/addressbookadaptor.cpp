/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "addressbookadaptor.h"

#include <kabc/addressee.h>
#include <kabc/vcardconverter.h>
#include <libkdepim/kabcresourcecached.h>
#include <kio/job.h>


using namespace KABC;


AddressBookUploadItem::AddressBookUploadItem( KPIM::GroupwareDataAdaptor *adaptor, KABC::Addressee addr, GroupwareUploadItem::UploadType type )
    : GroupwareUploadItem( type )
{
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

void AddressBookAdaptor::uploadFinished( KIO::TransferJob */*trfjob*/, KPIM::GroupwareUploadItem *item )
{
//   OGoGlobals::uploadFinished( this, trfjob, item );
  Addressee addr( resource()->findByUid( item->uid() ) );
  if ( !addr.isEmpty() ) {
  // FIXME: This doesn't work yet. So for now, just don't do anything and hope that the next folder refresh will clean things up nicely.
//     resource()->removeAddressee( addr );
/*    addr.insertCustom( identifier(), "storagelocation",
               idMapper()->remoteId( item->uid() ) );*/
  }
}


void AddressBookAdaptor::deleteItem( const QString &localId )
{
  KABC::Addressee a = mResource->findByUid( localId );
  if ( !a.isEmpty() ) mResource->removeAddressee( a );
}

KABC::Addressee::List AddressBookAdaptor::interpretDownloadItemJob( KIO::TransferJob *, const QString &rawText )
{
  KABC::VCardConverter conv;
  return conv.parseVCards( rawText );
}

QString AddressBookAdaptor::addItem( KIO::TransferJob *job,
     const QString &rawText, QString &fingerprint,
     const QString &localId, const QString &storageLocation )
{
  fingerprint = extractFingerprint( job, rawText );

  KABC::Addressee::List addressees( interpretDownloadItemJob( job, rawText ) );

  if ( addressees.count() > 1 ) {
    kdError() << "More than one addressee in vCard" << endl;
    return QString::null;
  }

  if ( addressees.count() == 0 ) {
    kdError() << "No valid addressee in vCard" << endl;
    return QString::null;
  }

  KABC::Addressee addr = *addressees.begin();
  if ( addr.isEmpty() ) {
    kdError() << "Addressee is empty." << endl;
    return QString::null;
  } else {
    if ( !localId.isEmpty() ) addr.setUid( localId );
    addr.setResource( mResource );
    addr.insertCustom( identifier(), "storagelocation", storageLocation );
    mResource->insertAddressee( addr );
    clearChange( addr.uid() );

    return addr.uid();
  }
}

QString AddressBookAdaptor::extractUid( KIO::TransferJob *job, const QString &data )
{
  KABC::Addressee::List addressees = interpretDownloadItemJob( job, data );
  if ( addressees.begin() == addressees.end() ) return QString::null;

  KABC::Addressee a = *(addressees.begin());
  return a.uid();
}

void AddressBookAdaptor::clearChange( const QString &uid )
{
  mResource->clearChange( uid );
}

KPIM::GroupwareUploadItem *AddressBookAdaptor::newUploadItem( KABC::Addressee addr,
             KPIM::GroupwareUploadItem::UploadType type )
{
  return new AddressBookUploadItem( this, addr, type );
}
