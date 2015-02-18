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
using namespace KABC;
MergeContacts::MergeContacts(const Akonadi::Item::List &items)
    : mListItem(items)
{
}

MergeContacts::~MergeContacts()
{

}

void MergeContacts::setItems(const Akonadi::Item::List &items)
{
    mListItem = items;
}

KABC::Addressee MergeContacts::mergedContact(bool excludeConflictPart)
{
    KABC::Addressee newContact;
    if (mListItem.count() <= 1)
        return newContact;
    bool firstAddress = true;
    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        if (item.hasPayload<KABC::Addressee>()) {
            KABC::Addressee address = item.payload<KABC::Addressee>();
            if (firstAddress) {
                firstAddress = false;
                newContact = address;
            } else {
                mergeToContact(newContact, address, excludeConflictPart);
            }
        }
    }
    return newContact;
}

void MergeContacts::mergeToContact(KABC::Addressee &newContact, const KABC::Addressee &fromContact, bool excludeConflictPart)
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
    const PhoneNumber::List listPhone = fromContact.phoneNumbers();
    if (!listPhone.isEmpty()) {
        PhoneNumber::List newContactsPhone = newContact.phoneNumbers();
        Q_FOREACH(const PhoneNumber &phone, listPhone) {
            if (!newContactsPhone.contains(phone)) {
                newContact.insertPhoneNumber(phone);
            }
        }
    }

    // Merge Address
    const Address::List listAddress = fromContact.addresses();
    if (!listAddress.isEmpty()) {
        Address::List newContactsAddress = newContact.addresses();
        Q_FOREACH(const Address &addr, listAddress) {
            if (!newContactsAddress.contains(addr)) {
                newContact.insertAddress(addr);
            }
        }
    }

    if (!excludeConflictPart) {
        // Merge Name
        if (newContact.name().isEmpty() && !fromContact.name().isEmpty()) {
            newContact.setName(fromContact.name());
        }
        // Merge organization
        if (newContact.organization().isEmpty() && !fromContact.organization().isEmpty()) {
            newContact.setOrganization(fromContact.organization());
        }
        // Merge NickName
        if (newContact.nickName().isEmpty() && !fromContact.nickName().isEmpty()) {
            newContact.setNickName(fromContact.nickName());
        }
        // Merge Title
        if (newContact.title().isEmpty() && !fromContact.title().isEmpty()) {
            newContact.setTitle(fromContact.title());
        }
        // Merge Departement
        if (newContact.department().isEmpty() && !fromContact.department().isEmpty()) {
            newContact.setDepartment(fromContact.department());
        }

        // Merge blog

        // Merge HomePage

        // Merge geo

    }
#if 0
    //TODO
    newContact.setName(fromContact.name());
    newContact.setFamilyName(fromContact.familyName());
    newContact.setFormattedName(fromContact.formattedName());
#endif
}

MergeContacts::ConflictInformations MergeContacts::requiresManualSelectionOfInformation()
{
    MergeContacts::ConflictInformations result = None;
    if (mListItem.count() < 2)
        return result;
    KABC::Addressee newContact;
    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        if (item.hasPayload<KABC::Addressee>()) {
            //Test Birthday
            const KABC::Addressee address = item.payload<KABC::Addressee>();
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
            const KABC::Geo geo = address.geo();
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
            const KABC::Picture photo = address.photo();
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
            const KABC::Picture logo = address.logo();
            if (!logo.isEmpty()) {
                if (!newContact.logo().isEmpty()) {
                    if (newContact.logo() != logo) {
                        result |= Logo;
                    }
                } else {
                    newContact.setLogo(address.logo());
                }
            }
            // Test Name
            const QString name = address.name();
            if (!name.isEmpty()) {
                if (!newContact.name().isEmpty()) {
                    if (newContact.name() != name) {
                        result |= Name;
                    }
                } else {
                    newContact.setName(address.name());
                }
            }
            // Test NickName
            const QString nickName = address.nickName();
            if (!nickName.isEmpty()) {
                if (!newContact.nickName().isEmpty()) {
                    if (newContact.nickName() != nickName) {
                        result |= NickName;
                    }
                } else {
                    newContact.setNickName(address.nickName());
                }
            }
            // Test Organization
            const QString organization = address.organization();
            if (!organization.isEmpty()) {
                if (!newContact.organization().isEmpty()) {
                    if (newContact.organization() != organization) {
                        result |= Organization;
                    }
                } else {
                    newContact.setOrganization(address.organization());
                }
            }
            // Test Title
            const QString title = address.title();
            if (!title.isEmpty()) {
                if (!newContact.title().isEmpty()) {
                    if (newContact.title() != title) {
                        result |= Title;
                    }
                } else {
                    newContact.setTitle(address.title());
                }
            }
            // Test Departement
            const QString departement = address.department();
            if (!departement.isEmpty()) {
                if (!newContact.department().isEmpty()) {
                    if (newContact.department() != departement) {
                        result |= Departement;
                    }
                } else {
                    newContact.setDepartment(address.department());
                }
            }
            // Test HomePage
            const KUrl url = address.url();
            if (url.isValid() && !url.isEmpty()) {
                if (newContact.url().isValid() && !newContact.url().isEmpty()) {
                    if (newContact.url() != url) {
                        result |= HomePage;
                    }
                } else {
                    newContact.setUrl(address.url());
                }
            }
            // Test FamilyName
            const QString familyName = address.familyName();
            if (!familyName.isEmpty()) {
                if (!newContact.familyName().isEmpty()) {
                    if (newContact.familyName() != familyName) {
                        result |= FamilyName;
                    }
                } else {
                    newContact.setFamilyName(address.familyName());
                }
            }
            // Test Blog
            checkCustomValue(address, QLatin1String( "BlogFeed" ), newContact, result, Blog);
            // Test profession
            checkCustomValue(address, QLatin1String( "X-Profession" ), newContact, result, Profession);
            // Test profession
            checkCustomValue(address, QLatin1String( "X-Office" ), newContact, result, Office);
            // Test ManagersName
            checkCustomValue(address, QLatin1String( "X-ManagersName" ), newContact, result, ManagerName);
            // Test AssistantsName
            checkCustomValue(address, QLatin1String( "X-AssistantsName" ), newContact, result, Assistant);
            // Test SpousesName
            checkCustomValue(address, QLatin1String( "X-SpousesName" ), newContact, result, PartnerName);            
            //Test Anniversary
            checkCustomValue(address, QLatin1String( "X-Anniversary" ), newContact, result, Anniversary);
        }
    }
    //qDebug()<<" result "<<result;
    return result;
}

void MergeContacts::checkCustomValue(const KABC::Addressee &address, const QString &variable, KABC::Addressee &newContact, MergeContacts::ConflictInformations &result, MergeContacts::ConflictInformation conflict)
{
    const QString value = address.custom( QLatin1String( "KADDRESSBOOK" ), variable);
    if (!value.isEmpty()) {
        const QString newValue = newContact.custom( QLatin1String( "KADDRESSBOOK" ), variable);
        if (!newValue.isEmpty()) {
            if (newValue != value) {
                result |= conflict;
            }
        } else {
            newContact.insertCustom(QLatin1String( "KADDRESSBOOK" ), variable, value);
        }
    }
}
