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

#include <qapplication.h>
#include <qthread.h>

#include "synchelper.h"

using namespace KSync;

namespace {
    struct DoneEvent : public QCustomEvent {
        DoneEvent() : QCustomEvent( 6666 ) {
        }
        ~DoneEvent() {
        }
    };
    class SyncThread : public QThread {
    public:
        SyncThread(Syncer*, QObject* obj );
        ~SyncThread();
        void run();
    private:
        Syncer* m_sync;
        QObject* m_obj;
    };
    SyncThread::SyncThread( Syncer* syn, QObject* obj) {
        m_sync = syn;
        m_obj = obj;
    }
    SyncThread::~SyncThread() {
    }
    void SyncThread::run() {
        m_sync->sync();
        /* we're done tell it the other thread.... */
        QApplication::postEvent( m_obj, new DoneEvent );
    }

}
struct SyncHelper::Data {
    Syncer *syncer;
};

SyncHelper::SyncHelper( SyncUi* ui, SyncAlgorithm* al) {
    data = new Data;
    data->syncer = new Syncer( ui, al );
}
SyncHelper::~SyncHelper() {
    delete data->syncer;
    delete data;
}
void SyncHelper::sync() {
    SyncThread thread(data->syncer, this );
    thread.start();
}
void SyncHelper::addSyncee( Syncee* syn ) {
    data->syncer->addSyncee( syn);
}
void SyncHelper::customEvent( QCustomEvent* ) {
    emit done();
}

#include "synchelper.moc"
