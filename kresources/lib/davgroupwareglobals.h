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
#ifndef DAVGROUPWAREGLOBALS_H
#define DAVGROUPWAREGLOBALS_H

#include "groupwareresourcejob.h"
#include <kurl.h>
#include <qstring.h>
#include <qdom.h>

namespace KIO {
class Job;
class TransferJob;
}

namespace KPIM {
class GroupwareDataAdaptor;
class GroupwareUploadItem;
}

namespace KCal {
class CalendarAdaptor;
}

namespace KABC {
class AddressBookAdaptor;
}

class DAVGroupwareGlobals
{
  public:
    DAVGroupwareGlobals() {}

    static KIO::TransferJob *createListItemsJob( const KURL &url );
    static QString extractFingerprint( KIO::Job *job, const QString &/*jobData*/ );

    static bool interpretListItemsJob( KPIM::GroupwareDataAdaptor *adaptor,
        KIO::Job *job );
    static bool interpretCalendarDownloadItemsJob( KCal::CalendarAdaptor *adaptor,
        KIO::Job *job, const QString &jobData );
    static bool interpretAddressBookDownloadItemsJob( KABC::AddressBookAdaptor *adaptor,
        KIO::Job *job, const QString &jobData );
    
    static KPIM::GroupwareJob::ContentType getContentType( const QDomElement &prop );
};

#endif
