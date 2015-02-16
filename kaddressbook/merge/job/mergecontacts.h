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

#ifndef MERGECONTACTS_H
#define MERGECONTACTS_H

#include <AkonadiCore/Item>
#include <KContacts/Addressee>

#include "kaddressbook_export.h"

namespace KABMergeContacts
{
class KADDRESSBOOK_EXPORT MergeContacts
{
public:
    MergeContacts(const Akonadi::Item::List &items = Akonadi::Item::List());
    ~MergeContacts();

    enum ConflictInformation {
        None = 0,
        Birthday = 1,
        Geo = 2,
        Photo = 4,
        Logo = 8,
        Anniversary = 16,
        Name = 32,
        NickName = 64,
        Blog = 128,
        HomePage = 256,
        Organization = 512,
        Profession = 1024,
        Title = 2056,
        Departement = 4096,
        Office = 8192,
        ManagerName = 16384,
        Assistant = 32768,
        FreeBusy = 65536,
        FamilyName = 131072,
        PartnerName = 262144
        //TODO Key
    };
    Q_ENUMS(ConflictInformation)
    Q_DECLARE_FLAGS(ConflictInformations, ConflictInformation)

    KContacts::Addressee mergedContact(bool excludeConflictPart = false);

    MergeContacts::ConflictInformations needManualSelectInformations();

    void setItems(const Akonadi::Item::List &items);
private:
    void mergeToContact(KContacts::Addressee &newAddress, const KContacts::Addressee &fromContact, bool excludeConflictPart);
    Akonadi::Item::List mListItem;
};
}

#endif // MERGECONTACTS_H
