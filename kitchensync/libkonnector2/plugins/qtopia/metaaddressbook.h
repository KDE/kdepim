#ifndef META_ADDRESS_BOOK_H
#define META_ADDRESS_BOOK_H

#include <addressbooksyncee.h>

#include "md5metatemplate.h"

namespace OpieHelper {
    class MetaAddressbook : public MD5Template<KSync::AddressBookSyncee, KSync::AddressBookSyncEntry> {
    public:
        MetaAddressbook();
        ~MetaAddressbook();

        QString string( KSync::AddressBookSyncEntry* );
    };
};


#endif
