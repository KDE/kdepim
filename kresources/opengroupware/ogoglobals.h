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
#ifndef OGOGLOBALS_H
#define OGOGLOBALS_H

#include <kurl.h>
#include <groupwareuploadjob.h>

namespace KPIM {
class GroupwareDataAdaptor;
class GroupwareUploadItem;
};
namespace KIO {
class TransferJob;
class Job;
};

class OGoGlobals
{
  public:
    OGoGlobals() {}
    static KIO::TransferJob *createDownloadItemJob( 
          KPIM::GroupwareDataAdaptor *adaptor, const KURL &url );

    static QString extractFingerprint( KIO::TransferJob *job, const QString &rawText );
    static void updateFingerprintId( KPIM::GroupwareDataAdaptor *adaptor, 
           KIO::TransferJob *trfjob, KPIM::GroupwareUploadItem *item );
    static KIO::Job *createRemoveItemsJob( const KURL &uploadurl, 
       KPIM::GroupwareUploadItem::List deletedItems );
};

#endif
