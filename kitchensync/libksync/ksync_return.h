
#ifndef KitchenSync_SyncReturn
#define KitchenSync_SyncReturn

#include <qptrlist.h>

#include <ksyncentry.h>

namespace KitchenSync {
    class SyncReturn{
    public:
        SyncReturn(const QPtrList<KSyncEntry>& synced,
                   const QPtrList<KSyncEntry>& inNotSynced,
                   const QPtrList<KSyncEntry>& ndNotSynced );
        SyncReturn( const SyncReturn& );
        ~SyncReturn();
        SyncReturn &operator=( const SyncReturn& );
        QPtrList<KSyncEntry> synced();
        QPtrList<KSyncEntry> in1NotSynced();
        QPtrList<KSyncEntry> in2NotSynced();
    private:
        QPtrList<KSyncEntry> m_sycned;
        QPtrList<KSyncEntry> m_1not;
        QPtrList<KSyncEntry> m_2not;
    };
};

#endif
