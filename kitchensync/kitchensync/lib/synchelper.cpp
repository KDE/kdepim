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
