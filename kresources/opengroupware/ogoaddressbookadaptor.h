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
#ifndef KABC_OGOADDRESSBOOKADAPTOR_H
#define KABC_OGOADDRESSBOOKADAPTOR_H

#include "addressbookadaptor.h"

#include <kabc/addressee.h>
#include <kurl.h>

namespace KABC {

class OGoAddressBookAdaptor : public AddressBookAdaptor
{
  public:
    OGoAddressBookAdaptor();

    void adaptDownloadUrl( KURL &url );
    void adaptUploadUrl( KURL &url );
    QString mimeType() const;
    QCString identifier() const;

    QString extractFingerprint( KIO::TransferJob *job, 
           const QString &rawText );
    KIO::TransferJob *createListItemsJob( const KURL &url );
    KIO::TransferJob *createDownloadItemJob( const KURL &url );

    bool itemsForDownloadFromList( KIO::Job *job, 
      QStringList &currentlyOnServer, QStringList &itemsForDownload );
    KABC::Addressee::List parseData( KIO::TransferJob *job, const QString &rawText );
    void updateFingerprintId( KIO::TransferJob *trfjob, KPIM::GroupwareUploadItem *item );
    KIO::Job *createRemoveItemsJob( const KURL &uploadurl, 
       KPIM::GroupwareUploadItem::List deletedItems );
};

}

#endif
