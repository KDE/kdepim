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
#ifndef KABC_GROUPDAVADDRESSBOOKADAPTOR_H
#define KABC_GROUPDAVADDRESSBOOKADAPTOR_H

#include "davaddressbookadaptor.h"
#include "groupdavglobals.h"
#include <davgroupwareglobals.h>
#include <kabc/addressee.h>
#include <kurl.h>

#include <qdom.h>

namespace KABC {

class GroupDavAddressBookAdaptor : public DavAddressBookAdaptor
{
  public:
    GroupDavAddressBookAdaptor();

    void customAdaptDownloadUrl( KURL &url );
    void customAdaptUploadUrl( KURL &url );
    QString mimeType() const { return "text/x-vcard"; }
    QCString identifier() const { return "KABCResourceGroupDAV"; }
    QString defaultNewItemName( KPIM::GroupwareUploadItem */*item*/ ) { return "new.vcf"; }
    long flags() const { return GWResBatchDelete; }


    // Creating Jobs
    KIO::Job *createListFoldersJob( const KURL &url )
        { return GroupDavGlobals::createListFoldersJob( url ); }
    KIO::TransferJob *createListItemsJob( const KURL &url )
        { return GroupDavGlobals::createListItemsJob( url ); }
    KIO::TransferJob *createDownloadJob( const KURL &url, KPIM::FolderLister::ContentType ctype )
        { return GroupDavGlobals::createDownloadJob( this, url, ctype ); }
    KIO::Job *createRemoveJob( const KURL &uploadurl, const KPIM::GroupwareUploadItem::List &deletedItems )
        { return GroupDavGlobals::createRemoveJob( this, uploadurl, deletedItems ); }


    // Interpreting Jobs
    bool interpretListItemsJob( KIO::Job *job, const QString &/*jobData*/ )
        { return GroupDavGlobals::interpretListItemsJob( this, job ); }
    bool interpretDownloadItemsJob( KIO::Job *job, const QString &jobData )
        { return GroupDavGlobals::interpretAddressBookDownloadItemsJob(
           this, job, jobData ); }


    bool getFolderHasSubs( const QDomNode &folderNode )
        { return GroupDavGlobals::getFolderHasSubs( folderNode ); }
    KPIM::FolderLister::ContentType getContentType( const QDomNode &folderNode )
        { return GroupDavGlobals::getContentType( folderNode ); }
};

}




#endif
