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

#include <qfile.h>
#include <qtextstream.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <kdebug.h>

#include <kab2/KAddressBookInterface.h>
#include <kab2/KAddressBookServerInterface.h>

KAddressBookServerInterface::KAddressBookServerInterface()
  :	DCOPObject("KAddressBookServer")
{
  KGlobal::dirs()
    ->addResourceType("formatDefinition", "share/apps/kab2/formats");

  KGlobal::dirs()->addResourceType("addressbook", "share/apps/kab2/books");

  _updateFormatDefinitionList();

  addressBookList_.setAutoDelete(true);

  // Create default book

  QString filenameForDefaultBook = locateLocal("addressbook", "default/");

  KAddressBookInterface * ab =
    new KAddressBookInterface
    (
     "default",
     "file:" + filenameForDefaultBook,
     defaultFormatDefinition()
    );

  addressBookList_.append(ab);

  // Read the rest of the books.

  _readConfig();
}

KAddressBookServerInterface::~KAddressBookServerInterface()
{
  // Empty ?
}

  void
KAddressBookServerInterface::_updateFormatDefinitionList()
{
  formatDefinitionList_.clear();

  QStringList l =
    KGlobal::dirs()->findAllResources("formatDefinition", "*.kabformat");

  for (QStringList::Iterator it(l.begin()); it != l.end(); ++it)
  {
    QString filename =
      (*it).replace(QRegExp("^.*/"), "").replace(QRegExp("\\.kabformat$"), "");

    formatDefinitionList_.append(filename);
  }
}

  QStringList
KAddressBookServerInterface::list()
{
  QStringList ret;

  QListIterator<KAddressBookInterface> it(addressBookList_); 

  for (; it.current(); ++it)
    ret << it.current()->name();

  return ret;
}

  bool
KAddressBookServerInterface::remove(QString name)
{
  bool deleted = false;

  QListIterator<KAddressBookInterface> it(addressBookList_);

  for (; it.current(); ++it)
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
KAddressBookServerInterface::create
(
 QString name,
 QString path,
 QString formatName
)
{
  KAB::FormatDefinition fd(formatDefinition(formatName));

  if (!fd)
  {
    kdDebug() << "Can't find format definition " << formatName << endl;
    return false;
  }

  KAddressBookInterface * ab = new KAddressBookInterface(name, path, fd);

  addressBookList_.append(ab);

  _writeConfig();

  return true;
}

  void
KAddressBookServerInterface::_readConfig()
{
  KConfig * c = KGlobal::config();

  c->setGroup("General");

  QStringList l = c->readListEntry("BookNames");

  for (QStringList::ConstIterator it(l.begin()); it != l.end(); ++it)
  {
    c->setGroup(*it);

    KAB::FormatDefinition fd =
      formatDefinition(c->readEntry("FormatDefinition"));

    if (!fd)
      continue;

    KAddressBookInterface * ab =
      new KAddressBookInterface
      (
       *it,
       c->readEntry("Path"),
       fd
      );

    addressBookList_.append(ab);
  }
}

  void
KAddressBookServerInterface::_writeConfig()
{
  KConfig * c = KGlobal::config();

  c->setGroup("General");

  QStringList l;

  QListIterator<KAddressBookInterface> it(addressBookList_);

  for (it.toFirst(); it.current(); ++it)
  {
    if (it.current()->name() != "default")
    {
      l << it.current()->name();
    }
  }

  c->writeEntry("BookNames", l);

  for (it.toFirst(); it.current(); ++it)
  {
    if (it.current()->name() != "default")
    {
      c->setGroup(it.current()->name());

      c->writeEntry("Path", it.current()->path());

      c->writeEntry
        (
         "FormatDefinition",
         it.current()->formatDefinition().name()
        );
    }
  }

  c->sync();
}

  KAB::FormatDefinition
KAddressBookServerInterface::defaultFormatDefinition()
{
  return formatDefinition("default");
}

  KAB::FormatDefinition
KAddressBookServerInterface::formatDefinition(QString name)
{
  _updateFormatDefinitionList();

  if (!formatDefinitionList_.contains(name))
  {
    kdDebug() << "Format definition with name " << name << " not found." << endl;

    if ("default" == name)
    {
      kdDebug()
        << "Default format definition should exist. Check your installation."
        << endl;
    }

    return KAB::FormatDefinition();
  }

  QFile f(name + ".kabformat");

  if (!f.open(IO_ReadOnly))
    return KAB::FormatDefinition();

  return KAB::FormatDefinition(f.readAll());
}

  QStringList
KAddressBookServerInterface::formatDefinitionList()
{
  _updateFormatDefinitionList();

  return formatDefinitionList_;
}

  bool
KAddressBookServerInterface::addFormatDefinition(KAB::FormatDefinition fd)
{
  if (fd.isNull())
  {
    kdDebug() << "Format definition is null." << endl;
    return false;
  }

  _updateFormatDefinitionList();

  if (formatDefinitionList_.contains(fd.name()))
  {
    kdDebug() << "Format definition `" << fd.name()
      << "' already exists." << endl;

    return false;
  }

  QString saveLocation(KGlobal::dirs()->saveLocation("formatDefinition"));

  if (!saveLocation)
  {
    kdDebug() << "Can't find save location." << endl;

    return false;
  }

  QFile f(saveLocation);

  if (!f.open(IO_WriteOnly))
  {
    kdDebug() << "Can't open file " << saveLocation << " for writing." << endl;
    return false;
  }

  QTextStream str(&f);

  str << fd.toXML();

  f.close();

  // XXX: QFile::close() has no retval. How to check file was written
  // successfully ?

  return true;
}

  bool
KAddressBookServerInterface::updateFormatDefinition(KAB::FormatDefinition fd)
{
  _updateFormatDefinitionList();

  if (fd.isNull())
  {
    kdDebug() << "Format definition is null." << endl;
    return false;
  }

  if (!formatDefinitionList_.contains(fd.name()))
  {
    kdDebug() << "Format definition `" << fd.name()
      << "' does not exist." << endl;

    return false;
  }

  QString saveLocation =
    KGlobal::dirs()
    ->findResource("formatDefinition", fd.name() + ".kabformat"); 

  if (!saveLocation)
  {
    kdDebug() << "Can't find save location." << endl;
    return false;
  }

  QFile f(saveLocation);

  if (!f.open(IO_WriteOnly))
  {
    kdDebug() << "Can't open file " << saveLocation << "for writing." << endl;
    return false;
  }

  QTextStream str(&f);

  str << fd.toXML();

  f.close();

  return true;
}

  bool
KAddressBookServerInterface::removeFormatDefinition(QString name)
{
  _updateFormatDefinitionList();

  if (!formatDefinitionList_.contains(name))
  {
    kdDebug() << "No format definition called " << name << endl;
    return false;
  }

  QString filename(locate("formatDefinition", name));

  if (!filename)
  {
    kdDebug() << "Can't find file for format definition " << name << endl;
    return false;
  }

  if (!QFile(filename).remove())
  {
    kdDebug() << "Can't remove format definition " << name << endl;
    return false;
  }

  return true;
}

  bool
KAddressBookServerInterface::hasFormatDefinition(QString name)
{
  _updateFormatDefinitionList();

  return formatDefinitionList_.contains(name);
}

