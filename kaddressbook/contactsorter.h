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

#ifndef CONTACTSORTER_H
#define CONTACTSORTER_H

#include "contactfields.h"

class ContactSorter
{
public:
    explicit ContactSorter(ContactFields::Field field,
                           Qt::SortOrder sortOrder = Qt::AscendingOrder);

    void sort(QList<KContacts::Addressee> &contacts) const;

private:
    const ContactFields::Field mSortField;
    const Qt::SortOrder mSortOrder;
};

#endif
