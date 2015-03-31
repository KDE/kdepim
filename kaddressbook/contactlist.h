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


#ifndef CONTACTLIST_H
#define CONTACTLIST_H
#include <KABC/Addressee>
#include <KABC/ContactGroup>

class ContactList
{
public:
    ContactList();

    bool isEmpty() const;
    int count() const;
    void clear();

    void append(const KABC::Addressee &addr);
    void append(const KABC::ContactGroup &group);

    KABC::Addressee::List addressList() const;
    void setAddressList(const KABC::Addressee::List &value);
    KABC::ContactGroup::List contactGroupList() const;
    void setContactGroupList(const KABC::ContactGroup::List &value);

private:
    KABC::Addressee::List mAddressList;
    KABC::ContactGroup::List mContactGroupList;
};

#endif // CONTACTLIST_H
