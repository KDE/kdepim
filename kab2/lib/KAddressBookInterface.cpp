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
#include <kurl.h>
#include <klibloader.h>
#include <kdebug.h>

// Local includes
#include "KAddressBookInterface.h"
#include "KAddressBookBackend.h"
#include "Field.h"

int KABUniqueEntryID = 0;

KAddressBookInterface::KAddressBookInterface(QString name, QString path)
  : DCOPObject(name.utf8()),
		name_(name),
    path_(path),
    backend_(0)
{
  QString protocol(KURL(path).protocol());

  QString libraryName(QString("libkabbackend_%1").arg(protocol));

  KLibrary * lib = KLibLoader::self()->library(libraryName);

  if (0 == lib)
  {
    kdDebug() << "Cannot find library " << libraryName << endl;
    return;
  }

  KLibFactory * factory = lib->factory();

  if (0 == factory)
  {
    kdDebug() << "Cannot find factory in library " << libraryName << endl;
    return;
  }

  QStringList argList;
  argList << name_ << path_;

  QObject * obj = factory->create(0, 0, 0, argList);

  if (0 == obj)
  {
    kdDebug() << "Factory would not create backend object" << endl;
    return;
  }

  backend_ = static_cast<KAddressBookBackend *>(obj);
}

KAddressBookInterface::~KAddressBookInterface()
{
}

	QString
KAddressBookInterface::path()
{
	return path_;
}

	QString
KAddressBookInterface::name()
{
	return name_;
}

  Entry
KAddressBookInterface::entry(QString id)
{
  if (0 == backend_)
  {
    return Entry();
  }
  else
  {
    return backend_->entry(id);
  }
}

  QStringList
KAddressBookInterface::entryList()
{
  if (0 == backend_)
  {
    return QStringList();
  }
  else
  {
    return backend_->entryList();
  }
}

  bool
KAddressBookInterface::contains(QString id)
{
  if (0 == backend_)
  {
    return false;
  }
  else
  {
    return backend_->contains(id);
  }
}

  QString
KAddressBookInterface::insert(Entry e)
{
  if (0 == backend_)
  {
    return QString::null;
  }
  else
  {
    return backend_->insert(e);
  }
}

  bool
KAddressBookInterface::remove(QString id)
{
  if (0 == backend_)
  {
    return false;
  }
  else
  {
    return backend_->remove(id);
  }
}

  bool
KAddressBookInterface::replace(Entry e)
{
  if (0 == backend_)
  {
    return false;
  }
  else
  {
    return backend_->replace(e);
  }
}

