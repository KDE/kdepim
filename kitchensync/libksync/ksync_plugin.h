
#ifndef KitchenSync_SyncPlugin
#define KitchenSync_SyncPlugin

#include <qobject.h>

#include <ksyncentry.h>
#include "ksync_return.h"

namespace KitchenSync {
    class SyncPlugin : public QObject{
        Q_OBJECT
    public:
        SyncPlugin(QObject* obj, const char* name = 0);
        virtual ~SyncPlugin();
        virtual SyncReturn sync( int mode,  KSyncEntry *in,
                                 KSyncEntry* out ) = 0;
        virtual void syncAsync( int mode,  KSyncEntry *in,
                                KSyncEntry *out ) = 0;
    signals:
        void done( const SyncReturn& );

    };
};

#endif
