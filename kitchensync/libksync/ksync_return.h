
#ifndef KitchenSync_SyncReturn
#define KitchenSync_SyncReturn

#include <ksyncentry.h>
#include <qptrlist.h>

//#include "ksyncentry.h"
#include "konnector.h"

namespace KitchenSync {
    class SyncReturn{
    public:
        SyncReturn();
        SyncReturn(const KSyncEntry::List& synced,
                   const KSyncEntry::List& inNotSynced,
                   const KSyncEntry::List& ndNotSynced );
        SyncReturn( const SyncReturn& );
        bool isEmpty() const;
        ~SyncReturn();
        SyncReturn &operator=( const SyncReturn& );
        KSyncEntry::List synced();
        KSyncEntry::List in1NotSynced();
        KSyncEntry::List in2NotSynced();
    private:
        bool m_empty:1;
        KSyncEntry::List m_sycned;
        KSyncEntry::List m_1not;
        KSyncEntry::List m_2not;
    };
};

#endif
