/*
    KAddressBook version 2
    
    Copyright (C) 1999 The KDE PIM Team <kde-pim@kde.org>
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "KAddressBookBackend.h"

KAddressBookBackend::KAddressBookBackend
(
 QString id,
 QString path,
 QObject * parent,
 const char * name
)
  : QObject(parent, name),
    id_(id),
    path_(path),
    initSuccess_(false)
{
}

KAddressBookBackend::~KAddressBookBackend()
{
}

  QString
KAddressBookBackend::id() const
{
  return id_;
}

  QString
KAddressBookBackend::path() const
{
  return path_;
}

  void
KAddressBookBackend::setInitSuccess()
{
  initSuccess_ = true;
}

  bool
KAddressBookBackend::initSuccess() const
{
  return initSuccess_;
}

