/*
  This file is part of KAddressBook.

  Copyright (c) 2015 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "contactlist.h"

ContactList::ContactList()
{

}

bool ContactList::isEmpty() const
{
    return (mAddressList.isEmpty() && mContactGroupList.isEmpty());
}

int ContactList::count() const
{
    return (mAddressList.count() + mContactGroupList.count());
}

void ContactList::clear()
{
    mAddressList.clear();
    mContactGroupList.clear();
}

void ContactList::append(const KABC::Addressee &addr)
{
    mAddressList.append(addr);
}

void ContactList::append(const KABC::ContactGroup &group)
{
    mContactGroupList.append(group);
}

KABC::ContactGroup::List ContactList::contactGroupList() const
{
    return mContactGroupList;
}

void ContactList::setContactGroupList(const KABC::ContactGroup::List &value)
{
    mContactGroupList = value;
}

KABC::Addressee::List ContactList::addressList() const
{
    return mAddressList;
}

void ContactList::setAddressList(const KABC::Addressee::List &value)
{
    mAddressList = value;
}

