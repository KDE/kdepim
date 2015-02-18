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

void MergeContacts::setItems(const Akonadi::Item::List &items)
{
    mListItem = items;
}

KContacts::Addressee MergeContacts::mergedContact(bool excludeConflictPart)
{
    KContacts::Addressee newContact;
    if (mListItem.count() <= 1) {
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
                mergeToContact(newContact, address, excludeConflictPart);
            }
        }
    }
    return newContact;
}

void MergeContacts::mergeToContact(KContacts::Addressee &newContact, const KContacts::Addressee &fromContact, bool excludeConflictPart)
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
        Q_FOREACH (const QString &email, emails) {
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
        Q_FOREACH (const QString &category, categories) {
            if (!newContactsCategories.contains(category)) {
                newContactsCategories.append(category);
            }
        }
        newContact.setCategories(newContactsCategories);
    }

    // Merge Address

    // Merge Phone

    // Merge blog

    // Merge HomePage

    // Merge geo
    if (!excludeConflictPart) {

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
    if (mListItem.count() < 2) {
        return result;
    }
    KContacts::Addressee newContact;
    Q_FOREACH (const Akonadi::Item &item, mListItem) {
        if (item.hasPayload<KContacts::Addressee>()) {
            const KContacts::Addressee address = item.payload<KContacts::Addressee>();
            //Test Birthday
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
            const QUrl url = address.url();
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
    qDebug() << " result " << result;
    return result;
}

void MergeContacts::checkCustomValue(const KContacts::Addressee &address, const QString &variable, KContacts::Addressee &newContact, MergeContacts::ConflictInformations &result, MergeContacts::ConflictInformation conflict)
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
