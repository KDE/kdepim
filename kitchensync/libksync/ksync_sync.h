
#ifndef KitchenSyncSyncManager
#define KitchenSyncSyncManager

#include <qobject.h>

#include <ksyncentry.h>

#include "ksync_return.h"

namespace KitchenSync {

    enum SyncManagerMode{ SYNC_INTERACTIVE=1,
                          SYNC_FIRSTOVERRIDE,
                          SYNC_SECONDOVERRIDE };
    class SyncManager : public QObject {
        Q_OBJECT
    public:
        SyncManager( QObject *obj,  const char* name );
        ~SyncManager();
        SyncReturn sync(int mode,
                        const KSyncEntryList& first,
                        const KSyncEntryList& second);

        // synchronize asynchron
        void syncAsync( int mode,
                        const KSyncEntryList& first,
                        const KSyncEntryList& second );
    signals:
        void done( const SyncReturn& );
    private slots:
        void doneSync( const SyncReturn& );

    };
};

#endif
