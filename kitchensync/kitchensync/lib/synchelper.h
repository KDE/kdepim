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
