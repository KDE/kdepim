 /*
    This file is part of kdepim.

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
#ifndef EXCHANGEGLOBALS_H
#define EXCHANGEGLOBALS_H

#include <kurl.h>
#include <groupwareuploadjob.h>
#include <qdom.h>

namespace KIO {
class Job;
class TransferJob;
}

namespace KPIM {
class GroupwareDataAdaptor;
class GroupwareUploadItem;
}
namespace KABC {
class AddressBookAdaptor;
}
namespace KCal {
class CalendarAdaptor;
}

class ExchangeGlobals
{
  public:
    ExchangeGlobals() {}
    static KPIM::FolderLister::ContentType getContentType( const QDomElement &prop );
    static KPIM::FolderLister::ContentType getContentType( const QString &contentclass );
    static KPIM::FolderLister::ContentType getContentType( const QDomNode &folderNode );
    static bool getFolderHasSubs( const QDomNode &folderNode );


    static KIO::Job *createListFoldersJob( const KURL &url );
    static KIO::TransferJob *createListItemsJob( const KURL &url );
    static KIO::TransferJob *createDownloadJob( KPIM::GroupwareDataAdaptor *adaptor,
                       const KURL &url, KPIM::FolderLister::ContentType ctype );
    static KIO::Job *createRemoveJob( const KURL &uploadurl,
                          const KPIM::GroupwareUploadItem::List &deletedItems );


    static bool interpretListItemsJob( KPIM::GroupwareDataAdaptor *adaptor,
           KIO::Job *job, const QString &jobData );
    static bool interpretCalendarDownloadItemsJob( KCal::CalendarAdaptor *adaptor,
                                        KIO::Job *job, const QString &jobData );
    static bool interpretAddressBookDownloadItemsJob( KABC::AddressBookAdaptor *adaptor,
                                        KIO::Job *job, const QString &jobData );
};

#endif
