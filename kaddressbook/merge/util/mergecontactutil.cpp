/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "mergecontactutil.h"

#include <KContacts/Addressee>
using namespace KABMergeContacts;
bool MergeContactUtil::hasSameNames(const Akonadi::Item::List &lst)
{
    QStringList names;
    bool foundSameName = true;

    //Improve check name
    Q_FOREACH (const Akonadi::Item &item, lst) {
        if (item.hasPayload<KContacts::Addressee>()) {
            const KContacts::Addressee address = item.payload<KContacts::Addressee>();
            const QString name = address.realName().toLower();
            if (!names.isEmpty() && !names.contains(name)) {
                foundSameName = false;
                break;
            }
            names.append(name);
        }
    }
    return foundSameName;
}
