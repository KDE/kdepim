/*
    KAddressBook version 2
    
    Copyright (C) 1999 Rik Hemsley rik@kde.org
    
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

// KDE includes
#include <kinstance.h>
#include <kurl.h>

// Local includes
#include "KAddressBookLDAPBackend.h"
#include "Entry.h"
#include "Field.h"

KAddressBookLDAPBackendFactory::KAddressBookLDAPBackendFactory
(
 QObject * parent,
 const char * name
)
  : KLibFactory(parent, name)
{
  new KInstance("kabbackend_file");
}

KAddressBookLDAPBackendFactory::~KAddressBookLDAPBackendFactory()
{
}

  QObject *
KAddressBookLDAPBackendFactory::createObject
(
 QObject * parent,
 const char * name,
 const char *,
 const QStringList & args
)
{
  QString id = args[0];
  QString path = args[1];

  return new KAddressBookLDAPBackend(id, path, parent, name);
}

extern "C"
{
  void * init_libkabbackend_file()
  {
    return new KAddressBookLDAPBackendFactory;
  }
}

KAddressBookLDAPBackend::KAddressBookLDAPBackend
(
 QString id,
 QString path,
 QObject * parent,
 const char * name
)
  : KAddressBookBackend(id, path, parent, name)
{
  // No setting of init success, so this will be marked as broken.
}

KAddressBookLDAPBackend::~KAddressBookLDAPBackend()
{
}

  Entry
KAddressBookLDAPBackend::entry(QString id) const
{
  qDebug("STUB");
  return Entry();
}

  bool
KAddressBookLDAPBackend::contains(QString id) const
{
  qDebug("STUB");
  return false;
}

  QString
KAddressBookLDAPBackend::insert(Entry e)
{
  qDebug("STUB");
  return QString();
}

  bool
KAddressBookLDAPBackend::remove(QString id)
{
  qDebug("STUB");
  return false;
}

  bool
KAddressBookLDAPBackend::replace(Entry /* e */)
{
  qDebug("STUB");
	return false;
}

  QStringList
KAddressBookLDAPBackend::entryList() const
{
}

