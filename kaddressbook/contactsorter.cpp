/*
  This file is part of KAddressBook.

  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "contactsorter.h"

#include <KABC/Addressee>

class ContactSortHelper
{
public:
    ContactSortHelper(ContactFields::Field field, Qt::SortOrder sortOrder)
        : mSortField(field), mSortOrder(sortOrder)
    {
    }

    inline bool operator()(const KABC::Addressee &contact,
                           const KABC::Addressee &otherContact) const
    {
        int result =
            QString::localeAwareCompare(
                ContactFields::value(mSortField, contact),
                ContactFields::value(mSortField, otherContact));

        if (result == 0) {
            int givenNameResult =
                QString::localeAwareCompare(
                    ContactFields::value(ContactFields::GivenName, contact),
                    ContactFields::value(ContactFields::GivenName, otherContact));

            if (givenNameResult == 0) {
                int familyNameResult =
                    QString::localeAwareCompare(
                        ContactFields::value(ContactFields::FamilyName, contact),
                        ContactFields::value(ContactFields::FamilyName, otherContact));

                if (familyNameResult == 0) {
                    result =
                        QString::localeAwareCompare(
                            ContactFields::value(ContactFields::FormattedName, contact),
                            ContactFields::value(ContactFields::FormattedName, otherContact));
                } else {
                    result = familyNameResult;
                }
            } else {
                result = givenNameResult;
            }
        }

        bool lesser = result < 0;

        if (mSortOrder == Qt::DescendingOrder) {
            lesser = !lesser;
        }

        return lesser;
    }

private:
    const ContactFields::Field mSortField;
    const Qt::SortOrder mSortOrder;
};

ContactSorter::ContactSorter(ContactFields::Field field, Qt::SortOrder sortOrder)
    : mSortField(field), mSortOrder(sortOrder)
{
}

void ContactSorter::sort(QList<KABC::Addressee> &contacts) const
{
    qStableSort(contacts.begin(), contacts.end(), ContactSortHelper(mSortField, mSortOrder));
}
