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

#include "davaddressbookadaptor.h"
#include "ogoglobals.h"
#include <davgroupwareglobals.h>
#include <kabc/addressee.h>
#include <kurl.h>

#include <qdom.h>

namespace KABC {

class OGoAddressBookAdaptor : public DavAddressBookAdaptor
{
  public:
    OGoAddressBookAdaptor();

    void adaptDownloadUrl( KURL &url );
    void adaptUploadUrl( KURL &url );
    QString mimeType() const { return "text/x-vcard"; }
    QCString identifier() const { return "KABCResourceOpengroupware"; }
    QString defaultNewItemName( KPIM::GroupwareUploadItem */*item*/ ) { return "new.vcf"; }


    QString extractFingerprint( KIO::TransferJob *job, const QString &rawText )
        { return OGoGlobals::extractFingerprint( job, rawText ); }


    // Creating Jobs
    KIO::Job *createListFoldersJob( const KURL &url )
        { return OGoGlobals::createListFoldersJob( url ); }
    KIO::TransferJob *createListItemsJob( const KURL &url )
        { return DAVGroupwareGlobals::createListItemsJob( url ); }
    KIO::TransferJob *createDownloadItemJob( const KURL &url, KPIM::GroupwareJob::ContentType ctype )
        { return OGoGlobals::createDownloadItemJob( this, url, ctype ); }
    KIO::Job *createRemoveItemsJob( const KURL &uploadurl, KPIM::GroupwareUploadItem::List deletedItems )
        { return OGoGlobals::createRemoveItemsJob( uploadurl, deletedItems ); }

        
    // Interpreting Jobs
    bool interpretListItemsJob( KIO::Job *job, QStringList &currentlyOnServer,
            QMap<QString,KPIM::GroupwareJob::ContentType> &itemsForDownload )
        { return DAVGroupwareGlobals::interpretListItemsJob( this, job, currentlyOnServer, itemsForDownload ); }
    KABC::Addressee::List interpretDownloadItemJob( KIO::TransferJob *job, const QString &rawText );

        
    bool getFolderHasSubs( const QDomNode &folderNode )
        { return OGoGlobals::getFolderHasSubs( folderNode ); }
    KPIM::FolderLister::FolderType getFolderType( const QDomNode &folderNode )
        { return OGoGlobals::getFolderType( folderNode ); }
};

}




#endif
