/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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
#ifndef KSYNC_STANDARD_SYNC_H
#define KSYNC_STANDARD_SYNC_H

#include "syncalgorithm.h"

#include <qptrlist.h>

namespace KSync {

class StandardSync : public SyncAlgorithm
{
  public:
    StandardSync( SyncUi *ui ) : SyncAlgorithm( ui ) {}

    virtual ~StandardSync()
    {
    }

    virtual void syncToTarget( Syncee *syncee, Syncee *target,
                               bool override = false );

 private:   
    void syncMeta( Syncee* syncee,
                   Syncee* target,
                   bool over );
    void addEntry( Syncee* in,
                   Syncee* out,
                   SyncEntry* entry );
    void syncSyncEntryListToSyncee ( QPtrList<SyncEntry>,
                   Syncee* syncee,
                   Syncee* target,bool over );
};

}

#endif
