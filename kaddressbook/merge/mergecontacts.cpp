/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mergecontacts.h"

using namespace KABMergeContacts;

MergeContacts::MergeContacts(const Akonadi::Item::List &items)
    : mListItem(items)
{
}

MergeContacts::~MergeContacts()
{

}

KABC::Addressee MergeContacts::mergedContact()
{
    KABC::Addressee newContact;
    if (mListItem.count() == 1)
        return newContact;
    bool firstAddress = true;
    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        if (item.hasPayload<KABC::Addressee>()) {
            KABC::Addressee address = item.payload<KABC::Addressee>();
            if (firstAddress) {
                firstAddress = false;
                newContact = address;
            } else {
                mergeToContact(newContact, address);
            }
        }
    }
    return newContact;
}

void MergeContacts::mergeToContact(KABC::Addressee &newContact, const KABC::Addressee &fromContact)
{
#if 0
    //TODO
    newContact.setName(fromContact.name());
    newContact.setFamilyName(fromContact.familyName());
    newContact.setFormattedName(fromContact.formattedName());
    newContact.setEmails(fromContact.emails());
#endif
}

bool MergeContacts::needManualSelectInformations()
{
    bool result = false;
    if (mListItem.count() < 2)
        return result;
    //TODO
    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        if (item.hasPayload<KABC::Addressee>()) {
            const KABC::Addressee address = item.payload<KABC::Addressee>();
            if (address.birthday().isValid()) {
                result = true;
            }
        }
    }

    return result;
}
