/*
    KAddressBookInterface version 2
    
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

#include <kglobal.h>
#include <kstddirs.h>
#include <kconfig.h>

#include "KAddressBookInterface.h"
#include "KAddressBookServerInterface.h"

KAddressBookServer::KAddressBookServer()
	:	KUniqueApplication()
{
	addressBookList_.setAutoDelete(true);

  KGlobal::dirs()->addResourceType("addressbook", "share/apps/kab2/books");

  // Create default book

  QString filenameForDefaultBook = locateLocal("addressbook", "default/");

  qDebug("filenameForDefaultBook == `%s'", filenameForDefaultBook.ascii());

	KAddressBookInterface * ab =
    new KAddressBookInterface("default", "file:" + filenameForDefaultBook);

	addressBookList_.append(ab);

  // Read the rest of the books.

  _readConfig();
}

KAddressBookServer::~KAddressBookServer()
{
	// Empty ?
}

	QStringList
KAddressBookServer::list()
{
	QStringList ret;

	for (QListIterator<KAddressBookInterface> it(addressBookList_); it.current(); ++it)
		ret << it.current()->name();

	return ret;
}

  bool
KAddressBookServer::remove(QString name)
{
  bool deleted = false;

	for (QListIterator<KAddressBookInterface> it(addressBookList_); it.current(); ++it)
  {
    if (it.current()->name() == name)
    {
      addressBookList_.removeRef(it.current());
      deleted = true;
    }
  }

  if (!deleted)
    return false;

  _writeConfig();
  return true;
}

  bool
KAddressBookServer::create(QString name, QString path, QString formatAsXML)
{
  KAddressBookInterface * ab = new KAddressBookInterface(name, path);
  addressBookList_.append(ab);

  _writeConfig();
  return true;
}

  void
KAddressBookServer::_readConfig()
{
  KConfig * c = KGlobal::config();

  c->setGroup("General");

  QStringList l = c->readListEntry("BookNames");

  for (QStringList::ConstIterator it(l.begin()); it != l.end(); ++it)
  {
    c->setGroup(*it);
    KAddressBookInterface * ab = new KAddressBookInterface(*it, c->readEntry("Path"));
    addressBookList_.append(ab);
  }
}

  void
KAddressBookServer::_writeConfig()
{
  KConfig * c = KGlobal::config();

  c->setGroup("General");

  QStringList l;

	for (QListIterator<KAddressBookInterface> it(addressBookList_); it.current(); ++it)
    if (it.current()->name() != "default")
      l << it.current()->name();

  c->writeEntry("BookNames", l);

	for (QListIterator<KAddressBookInterface> it(addressBookList_); it.current(); ++it)
    if (it.current()->name() != "default")
    {
      c->setGroup(it.current()->name());
      c->writeEntry("Path", it.current()->path());
    }

  c->sync();
}

