/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include <KABC/Addressee>

bool MergeContactUtil::hasDifferentNames(const Akonadi::Item::List &lst)
{
    QStringList names;
    bool foundSameName = false;
    Q_FOREACH (const Akonadi::Item &item, lst) {
        const KABC::Addressee address = item.payload<KABC::Addressee>();
        const QString name = address.additionalName();
        qDebug()<<" name"<<name;
        qDebug()<<" address "<<address.familyName();
        if (names.contains(name)) {
            foundSameName = true;
        }
        names.append(name);
    }
    return foundSameName;
}
