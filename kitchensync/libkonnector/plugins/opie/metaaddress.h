
#ifndef OpieHelper_MetaAddress
#define OpieHelper_MetaAddress

#include <qvaluelist.h>


#include <addressbooksyncee.h>

namespace OpieHelper {
    class MetaAddress {
    public:
        MetaAddress();
        ~MetaAddress();

        /**
         * @param newSyncee
         * @param oldSyncee it'll be deleted
         * @return newSyncee including the state
         */
        KSync::AddressBookSyncee* doMeta( KSync::AddressBookSyncee* newSyncee,
                                  KSync::AddressBookSyncee* oldSyncee);

    };
};

#endif
