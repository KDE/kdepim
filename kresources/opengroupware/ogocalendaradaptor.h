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
#ifndef KCAL_OGOCALENDARADAPTOR_H
#define KCAL_OGOCALENDARADAPTOR_H

#include "calendaradaptor.h"
#include "groupwareuploadjob.h"
#include <kurl.h>

namespace KIO {
class Job;
}

namespace KCal {

class OGoCalendarAdaptor : public CalendarAdaptor
{
  public:
    OGoCalendarAdaptor();

    void customAdaptDownloadUrl( KURL &url );
    void customAdaptUploadUrl( KURL &url );
    QCString identifier() const { return "KCalResourceOpengroupware"; }

    QString extractFingerprint( KIO::TransferJob *job,
           const QString &rawText );

    KIO::TransferJob *createListItemsJob( const KURL &url );
    KIO::TransferJob *createDownloadItemJob( const KURL &url, KPIM::GroupwareJob::ContentType ctype );

    bool itemsForDownloadFromList( KIO::Job *job,
      QStringList &currentlyOnServer, QMap<QString,KPIM::GroupwareJob::ContentType> &itemsForDownload );
    KIO::Job *createRemoveItemsJob( const KURL &uploadurl,
       KPIM::GroupwareUploadItem::List deletedItems );
    QString defaultNewItemName( KPIM::GroupwareUploadItem* /*item*/ ) { return "new.ics"; }
};

}

#endif
