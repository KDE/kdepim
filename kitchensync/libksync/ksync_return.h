
#ifndef KitchenSync_SyncReturn
#define KitchenSync_SyncReturn

#include <qptrlist.h>

#include <ksyncentry.h>

namespace KitchenSync {
    class SyncReturn{
    public:
        SyncReturn();
        SyncReturn(const KSyncEntryList& synced,
                   const KSyncEntryList& inNotSynced,
                   const KSyncEntryList& ndNotSynced );
        SyncReturn( const SyncReturn& );
        bool isEmpty() const;
        ~SyncReturn();
        SyncReturn &operator=( const SyncReturn& );
        KSyncEntryList synced();
        KSyncEntryList in1NotSynced();
        KSyncEntryList in2NotSynced();
    private:
        bool m_empty:1;
        KSyncEntryList m_sycned;
        KSyncEntryList m_1not;
        KSyncEntryList m_2not;
    };
};

#endif
