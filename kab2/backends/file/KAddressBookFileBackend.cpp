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

// System includes
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <iostream.h>

// Qt includes
#include <qdir.h>
#include <qfile.h>
#include <qtextstream.h>

// KDE includes
#include <kinstance.h>
#include <kurl.h>

// Local includes
#include "KAddressBookFileBackend.h"
#include "Entry.h"
#include "Field.h"

KAddressBookFileBackendFactory::KAddressBookFileBackendFactory
(
 QObject * parent,
 const char * name
)
  : KLibFactory(parent, name)
{
  new KInstance("kabbackend_file");
}

KAddressBookFileBackendFactory::~KAddressBookFileBackendFactory()
{
}

  QObject *
KAddressBookFileBackendFactory::createObject
(
 QObject * parent,
 const char * name,
 const char *,
 const QStringList & args
)
{
  QString id = args[0];
  QString path = args[1];

  return new KAddressBookFileBackend(id, path, parent, name);
}

extern "C"
{
  void * init_libkabbackend_file()
  {
    return new KAddressBookFileBackendFactory;
  }
}

int KAddressBookFileBackendUniqueEntryID = 0;

KAddressBookFileBackend::KAddressBookFileBackend
(
 QString id,
 QString path,
 QObject * parent,
 const char * name
)
  : KAddressBookBackend(id, KURL(path).path(), parent, name)
{
  struct utsname utsName;

  if (uname(&utsName) == 0)
    uniquePartOne_ = utsName.nodename;
  else
    uniquePartOne_ = "localhost";

  struct timeval timeVal;
  struct timezone timeZone;
  gettimeofday(&timeVal, &timeZone);

  uniquePartOne_ += "_" + QString().setNum(timeVal.tv_sec);
  uniquePartOne_ += "_" + QString().setNum(getpid());

  bool dirsOk = _checkDirs();

  if (!dirsOk)
  {
    qDebug("Cannot create dirs. I'm broken.");
    return;
  }

  _initIndex();

  setInitSuccess();
}

KAddressBookFileBackend::~KAddressBookFileBackend()
{
}

  Entry
KAddressBookFileBackend::entry(QString id) const
{
  Entry e;

  if (!index_.contains(id))
    return e;

  for (QStringList::ConstIterator it(index_.begin()); it != index_.end(); ++it)

    if (*it == id)
    {
      Entry loaded = _readEntry(*it);

      if (!loaded.isNull())
        e = loaded;

      break;
    }

  return e;
}

  bool
KAddressBookFileBackend::contains(QString id) const
{
  return index_.contains(id);
}

  QString
KAddressBookFileBackend::insert(Entry e)
{
  e.setID(_generateUniqueID());

  index_.append(e.id());
  
  bool ok = _writeEntry(e);

  if (!ok)
    return QString::null;

  return e.id();
}

  bool
KAddressBookFileBackend::remove(QString id)
{
  if (!index_.contains(id))
    return false;

  index_.remove(id);

  _removeEntry(id);
  
  return true;
}

  bool
KAddressBookFileBackend::replace(Entry /* e */)
{
#warning STUB needs implementing
	return false;
  //return index_.contains(e.id());
}

  QString
KAddressBookFileBackend::_generateUniqueID()
{
  return uniquePartOne_ + "_" +
    QString().setNum(KAddressBookFileBackendUniqueEntryID++);
}

  bool
KAddressBookFileBackend::_checkDirs()
{
  QDir base(path());

  qDebug("KAddressBookFileBackend::_checkDirs(): path == %s", path().ascii());

  if (!base.exists())
    if (!base.mkdir(path())) {
      cerr << "Could not create dir `" + path() + "' - exiting" << endl;
      return false;
    }

  QDir tmp(path() + "/tmp");
  
  if (!tmp.exists())
    if (!tmp.mkdir(path() + "/tmp")) {
      cerr << "Could not create dir `" + path() + "/tmp' - exiting" << endl;
      return false;
    }

  QDir entries(path() + "/entries");
  
  if (!entries.exists())
    if (!entries.mkdir(path() + "/entries")) {
      cerr << "Could not create dir `" + path() + "/entries' - exiting" << endl;
      return false;
    }

  return true;
}

  void
KAddressBookFileBackend::_initIndex()
{
  QDir d(path() + "/entries");

  d.setFilter(QDir::Files | QDir::NoSymLinks | QDir::Readable);
  
  index_ = d.entryList();
}

  bool
KAddressBookFileBackend::_writeEntry(const Entry & e)
{
  qDebug("KAddressBook::_writeEntry()");
  QString filename = path() + "/tmp" + e.id();

  QFile f(filename);

  if (f.exists()) {
    qDebug("File `" + filename + "' exists");
    usleep(2000);
  }

  if (f.exists()) {
    qDebug("File `" + filename + "' still exists");
    return false;
  }

  if (!f.open(IO_WriteOnly)) {
    qDebug("Couldn't open file `" + filename + "' for writing");
    return false;
  }

  QDomDocument doc("kab-entry");

  e.insertInDomTree(doc, doc);

  qDebug("Doc as string: %s", doc.toString().ascii());

  QTextStream str(&f);

  str << doc.toString();

  f.flush();
  f.close();

  if (f.status() != IO_Ok) {
    qDebug("Couldn't flush file `" + filename + "'");
    f.remove();
    return false;
  }
  
  QString linkTarget(path() + "/entries/" + e.id());

  if (::link(QFile::encodeName(filename), QFile::encodeName(linkTarget)) != 0) {
    qDebug("Couldn't successfully link `" + filename +
      "' to `" + linkTarget + "' - giving up");
    return false;
  }

  f.remove();

  return true;
}

  QStringList
KAddressBookFileBackend::entryList() const
{
  return
    QDir(path() + "/entries").entryList
    (QDir::Files | QDir::NoSymLinks | QDir::Readable, QDir::Unsorted);
}

  Entry
KAddressBookFileBackend::_readEntry(const QString & id) const
{
  QString filename = path() + "/entries/" + id;
  QFile f(filename);

  if (!f.exists()) {
    qDebug("File `" + filename + "' does not exist");
    return Entry();
  }
  
  if (!f.open(IO_ReadOnly)) {
    qDebug("Couldn't open `" + filename + "' for reading");
    return Entry();
  }
  
  QDomDocument doc;

  if (!doc.setContent(&f))
  {
    qDebug("Couldn't set content to `" + filename + "'");
    return Entry();
  }

  QDomElement docElem = doc.documentElement();

  if (docElem.isNull())
  {
    qDebug("Can't parse file `" + filename + "'");
    return Entry();
  }

  if (docElem.tagName() != "kab:entry")
  {
    qDebug("Can't parse file `" + filename + "'");
    return Entry();
  }

  return Entry(docElem);
}

  bool
KAddressBookFileBackend::_removeEntry(const QString & id)
{
  QFile f(path() + "/entries/" + id);

  if (!f.exists())
    return false;

  if (!f.remove())
    return false;

  index_.remove(id);

  return true;
}

