
#ifndef KitchenSync_KaddressbookSyncAlgo
#define KitchenSync_KaddressbookSyncAlgo

#include <kaddressbooksyncentry.h>

#include <ksync_plugin.h>

namespace KitchenSync {
    class SyncAddressbook : public SyncPlugin {
    public:
        SyncAddressbook( QObject* obj, const char *name = 0,  const QStringList & = QStringList() );
        virtual ~SyncAddressbook();
        SyncReturn sync(int mode,  KSyncEntry *in,
                        KSyncEntry *out );
        void syncAsync( int mode,  KSyncEntry *in,
                        KSyncEntry *out );
    private:
        int m_mode;
        KAddressbookSyncEntry *m_entry;
        QStringList blackIds1,  blackIds2;

        void syncMeta( KAddressbookSyncEntry *entry1,
	               KAddressbookSyncEntry *entry2 );
        void syncNormal( KAddressbookSyncEntry* entry1,  KAddressbookSyncEntry* entry2 );
        void syncAdded(const QValueList<KABC::Addressee> &added,
                       const QValueList<KABC::Addressee> &added2 );
        void syncAdded(const KABC::AddressBook *added,
	               const KABC::AddressBook *added2 );
    };
};

#endif
