
#ifndef OpieHelperAddressBookShit_H
#define OpieHelperAddressBookShit_H

#include <qstring.h>

#include <addressbooksyncee.h>

#include "helper.h"


namespace OpieHelper {

    class AddressBook : public Base {
    public:
        AddressBook( CategoryEdit* edit = 0,
                     KSync::KonnectorUIDHelper* helper = 0,
                     const QString &tz = QString::null,
                     bool metaSyncing = FALSE );
        ~AddressBook();
        KSync::AddressBookSyncee * toKDE( const QString &fileName );
        /* it must be deleted but not by this one here */
        KTempFile* fromKDE(KSync::AddressBookSyncee* syncee );
    private:
        enum Fields {
            Uid = 0,
            Category,
            Title,
            FirstName,
            MiddleName,
            LastName,
            Suffix,
            FileAs,
            JobTitle,
            Department,
            Company,
            BusinessPhone,
            BusinessFax,
            BusinessMobile,
            DefaultEmail,
            Emails,
            HomePhone,
            HomeFax,
            HomeMobile,
            BusinessStreet,
            BusinessCity,
            BusinessState,
            BusinessZip,
            BusinessCountry,
            BusinessPager,
            BusinessWebPage,
            Office,
            Profession,
            Assistant,
            Manager,
            HomeStreet,
            HomeCity,
            HomeState,
            HomeZip,
            HomeCountry,
            HomeWebPage,
            Spouse,
            Gender,
            Birthday,
            Anniversary,
            Nickname,
            Children,
            Notes,
            Groups
        };
    };
}


#endif
