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
#ifndef KCAL_BLOGGINGCALENDARADAPTOR_H
#define KCAL_BLOGGINGCALENDARADAPTOR_H

#include "calendaradaptor.h"
#include "groupwareuploadjob.h"
#include <kurl.h>

namespace KIO {
class Job;
}

namespace KCal {

class BloggingCalendarAdaptor : public CalendarAdaptor
{
  public:
    BloggingCalendarAdaptor();

    QCString identifier() const { return "KCalResourceBlogging"; }

    QString extractFingerprint( KIO::TransferJob */*job*/,
           const QString &/*rawText*/ ) { return QString::null; }

    KIO::Job         *createListFoldersJob ( const KURL &url );
    KIO::TransferJob *createListItemsJob   ( const KURL &url );
    KIO::TransferJob *createDownloadItemJob( const KURL &url, KPIM::GroupwareJob::ContentType ctype );
    KIO::Job         *createRemoveItemsJob ( const KURL &url, KPIM::GroupwareUploadItem::List deletedItems );

    void interpretListFoldersJob( KIO::Job *job, KPIM::FolderLister *folderLister );
    bool interpretListItemsJob  ( KIO::Job *job,
      QStringList &currentlyOnServer, QMap<QString,KPIM::GroupwareJob::ContentType> &itemsForDownload );
};

}

#endif
