
#ifndef OpieHelperAddressBookShit_H
#define OpieHelperAddressBookShit_H

#include <qstring.h>
#include <kaddressbooksyncentry.h>

#include "helper.h"


namespace OpieHelper {

    class AddressBook : public Base {
    public:
        AddressBook( CategoryEdit* edit = 0,
                     KonnectorUIDHelper* helper = 0,
                     const QString &tz = QString::null,
                     bool metaSyncing = FALSE );
        ~AddressBook();
        KAddressbookSyncEntry* toKDE( const QString &fileName );
//        it must be deleted but not by this one here
        QByteArray fromKDE(KAddressbookSyncEntry* entry);
    };
};


#endif
