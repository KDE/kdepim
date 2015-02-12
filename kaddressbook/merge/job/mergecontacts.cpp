/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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
using namespace KContacts;
MergeContacts::MergeContacts(const Akonadi::Item::List &items)
    : mListItem(items)
{
}

MergeContacts::~MergeContacts()
{

}

KContacts::Addressee MergeContacts::mergedContact()
{
    KContacts::Addressee newContact;
    if (mListItem.count() == 1) {
        return newContact;
    }
    bool firstAddress = true;
    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        if (item.hasPayload<KContacts::Addressee>()) {
            KContacts::Addressee address = item.payload<KContacts::Addressee>();
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

void MergeContacts::mergeToContact(KContacts::Addressee &newContact, const KContacts::Addressee &fromContact)
{
    // Duplicate notes.
    const QString fromContactNote = fromContact.note();
    if (!fromContactNote.isEmpty()) {
        QString newContactNote = newContact.note();
        if (!newContactNote.isEmpty()) {
            newContactNote += QLatin1Char('\n');
        }
        newContactNote += fromContactNote;
        newContact.setNote(newContactNote);
    }
    // Duplicate emails
    const QStringList emails = fromContact.emails();
    if (!emails.isEmpty()) {
        QStringList newContactsEmail = newContact.emails();
        Q_FOREACH(const QString &email, emails) {
            if (!newContactsEmail.contains(email)) {
                newContactsEmail.append(email);
            }
        }
        newContact.setEmails(newContactsEmail);
    }
    // Merge Categories
    const QStringList categories = fromContact.categories();
    if (!categories.isEmpty()) {
        QStringList newContactsCategories = newContact.categories();
        Q_FOREACH(const QString &category, categories) {
            if (!newContactsCategories.contains(category)) {
                newContactsCategories.append(category);
            }
        }
        newContact.setCategories(newContactsCategories);
    }

    // Merge Phone

    // Merge blog

    // Merge HomePage

    // Merge geo
#if 0
    //TODO
    newContact.setName(fromContact.name());
    newContact.setFamilyName(fromContact.familyName());
    newContact.setFormattedName(fromContact.formattedName());
#endif
}

MergeContacts::ConflictInformations MergeContacts::needManualSelectInformations()
{
    MergeContacts::ConflictInformations result = None;
    if (mListItem.count() < 2)
        return result;
    KContacts::Addressee newContact;
    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        if (item.hasPayload<KContacts::Addressee>()) {
            //Test anniversary
            const KContacts::Addressee address = item.payload<KContacts::Addressee>();
            if (address.birthday().isValid()) {
                if (newContact.birthday().isValid()) {
                    if (newContact.birthday() != address.birthday()) {
                        result |= Birthday;
                    }
                } else {
                    newContact.setBirthday(address.birthday());
                }
            }
            //Test Geo
            const KContacts::Geo geo = address.geo();
            if (geo.isValid()) {
                if (newContact.geo().isValid()) {
                    if (newContact.geo() != geo) {
                        result |= Geo;
                    }
                } else {
                    newContact.setGeo(address.geo());
                }
            }
            // Test Photo
            const KContacts::Picture photo = address.photo();
            if (!photo.isEmpty()) {
                if (!newContact.photo().isEmpty()) {
                    if (newContact.photo() != photo) {
                        result |= Photo;
                    }
                } else {
                    newContact.setPhoto(address.photo());
                }
            }
            //Test Logo
            const KContacts::Picture logo = address.logo();
            if (!logo.isEmpty()) {
                if (!newContact.logo().isEmpty()) {
                    if (newContact.logo() != logo) {
                        result |= Logo;
                    }
                } else {
                    newContact.setLogo(address.logo());
                }
            }
        }
    }
    return result;
}
