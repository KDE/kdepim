/*
    This file is part of KitchenSync.

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
#ifndef KSYNC_SYNC_HELPER_H
#define KSYNC_SYNC_HELPER_H

#include <qobject.h>

#include <syncer.h>


namespace KSync {
    /**
     * Sync helper helps to sync to Syncee's
     * in a different thread and emits a signal
     * when done with syncing
     */
    class SyncHelper : public QObject{
        Q_OBJECT
    public:
        SyncHelper( SyncUi*, SyncAlgorithm* );
        ~SyncHelper();
        void sync();
        void addSyncee( Syncee* );
    signals:
        void done();

    protected:
         void customEvent( QCustomEvent* );
    private:
        struct Data;
        Data* data;
        class Private;
        Private* d;
    };
}

#endif
