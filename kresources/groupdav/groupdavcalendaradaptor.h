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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KCAL_GROUPDAVCALENDARADAPTOR_H
#define KCAL_GROUPDAVCALENDARADAPTOR_H

#include "davcalendaradaptor.h"
#include "groupwareuploadjob.h"
#include "groupdavglobals.h"
#include <davgroupwareglobals.h>

#include <kurl.h>

namespace KIO {
class Job;
}

namespace KCal {

class GroupDavCalendarAdaptor : public DavCalendarAdaptor
{
  public:
    GroupDavCalendarAdaptor();

    QValueList<KPIM::FolderLister::ContentType> supportedTypes()
    {
      QValueList<KPIM::FolderLister::ContentType> types;
      types << KPIM::FolderLister::Event;
      types << KPIM::FolderLister::Todo;
      return types;
    }
    void customAdaptDownloadUrl( KURL &url );
    void customAdaptUploadUrl( KURL &url );
    QCString identifier() const { return "KCalResourceGroupDAV"; }
    QString defaultNewItemName( KPIM::GroupwareUploadItem */*item*/ ) { return "new.ics"; }
    long flags() const { return 0; }


    // Creating Jobs
    KIO::Job *createListFoldersJob( const KURL &url )
        { return GroupDavGlobals::createListFoldersJob( url ); }
    KIO::TransferJob *createListItemsJob( const KURL &url )
        { return GroupDavGlobals::createListItemsJob( url ); }
    KIO::TransferJob *createDownloadJob( const KURL &url, KPIM::FolderLister::ContentType ctype )
        { return GroupDavGlobals::createDownloadJob( this, url, ctype ); }
    KIO::Job *createRemoveJob( const KURL &uploadurl, KPIM::GroupwareUploadItem *deletedItem )
        { return GroupDavGlobals::createRemoveJob( this, uploadurl, deletedItem ); }


    // Interpreting Jobs
    bool interpretListItemsJob( KIO::Job *job, const QString &/*jobData*/ )
        { return GroupDavGlobals::interpretListItemsJob( this, job ); }
    bool interpretDownloadItemsJob( KIO::Job *job, const QString &jobData )
        { return GroupDavGlobals::interpretCalendarDownloadItemsJob( this, job, jobData );  }


    bool getFolderHasSubs( const QDomNode &folderNode )
        { return GroupDavGlobals::getFolderHasSubs( folderNode ); }
    KPIM::FolderLister::ContentType getContentType( const QDomNode &folderNode )
        { return GroupDavGlobals::getContentType( folderNode ); }
};
}


#endif
