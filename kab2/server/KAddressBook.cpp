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
#include <qdatastream.h>

// KDE includes
#include <kapp.h>

// Local includes
#include "KAddressBookInterface.h"
#include "Entity.h"
#include "Field.h"

int KABUniqueEntityID = 0;

// Addressbook /////////////////////////////////////////////////////////////

KAddressBook::KAddressBook(QCString name, QString path)
  : DCOPObject(name),
    path_(path)
{
  _init();
}

KAddressBook::~KAddressBook()
{
}

  bool
KAddressBook::process(
  const QCString & fun, const QByteArray & argData,
	QCString & replyType, QByteArray & replyData)
{
  QDataStream argStream   (argData, IO_ReadOnly);
  QDataStream replyStream (replyData, IO_WriteOnly);
  
  if (fun == "entity(QString)") {
    replyType = "Entity";

    QString arg;
    argStream >> arg;

    Entity retval;
    retval = entity(arg);
    replyStream << retval;

    return true;
  }
  
  else if (fun == "insert(Entity)") {
    replyType = "QString";

    Entity arg;
    argStream >> arg;

    QString retval;
    retval = insert(arg);
    replyStream << retval;

    return true;
  }
  
  else if (fun == "remove(QString)") {
    replyType = "bool";

    QString arg;
    argStream >> arg;

    bool retval;
    retval = remove(arg);
    replyStream << retval;

    return true;
  }
  
  else if (fun == "bool replace(Entity)") {
    replyType = "bool";

    Entity arg;
    argStream >> arg;

    bool retval;
    retval = replace(arg);
    replyStream << retval;

    return true;
  }

  else if (fun == "bool contains(QString)") {
    replyType = "bool";

    QString arg;
    argStream >> arg;

    bool retval;
    retval = contains(arg);
    replyStream << retval;

    return true;
  }

  else {
    qDebug("Unknown function call to KAddressBook::process()");
    return false;
  }
}

  Entity
KAddressBook::entity(QString id)
{
  Entity e;

  if (!index_.contains(id))
    return e;

  QStringList::ConstIterator it;

  for (QStringList::ConstIterator it(index_.begin()); it != index_.end(); ++it)
    if (*it == id) {
      Entity * loaded = _readEntity(*it);
      if (loaded != 0)
        e = *loaded;
      break;
    }

  return e;
}

  bool
KAddressBook::contains(QString id)
{
  return index_.contains(id);
}

  QString
KAddressBook::insert(Entity e)
{
  qDebug("insert entity `" + e.name() + "'");

  e.setID(_generateUniqueID());

  index_.append(e.id());
  
  bool ok = _writeEntity(e);

  if (!ok)
    return QString::null;

  return e.id();
}

  bool
KAddressBook::remove(QString id)
{
  if (!index_.contains(id))
    return false;

  index_.remove(id);

  _removeEntity(id);
  
  return true;
}

  bool
KAddressBook::replace(Entity e)
{
  //return index_.contains(e.id());
}

  QString
KAddressBook::_generateUniqueID()
{
  return uniquePartOne_ + "_" + QString().setNum(KABUniqueEntityID++);
}

  void
KAddressBook::_init()
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

  _checkDirs();

  _initIndex();
}

  void
KAddressBook::_checkDirs()
{
  QDir base(path_);

  if (!base.exists())
    if (!base.mkdir(path_)) {
      cerr << "Could not create dir `" + path_ + "' - exiting" << endl;
      exit(1);
    }

  QDir tmp(path_ + "/tmp");
  
  if (!tmp.exists())
    if (!tmp.mkdir(path_ + "/tmp")) {
      cerr << "Could not create dir `" + path_ + "/tmp' - exiting" << endl;
      exit(1);
    }

  QDir entries(path_ + "/entries");
  
  if (!entries.exists())
    if (!entries.mkdir(path_ + "/entries")) {
      cerr << "Could not create dir `" + path_ + "/entries' - exiting" << endl;
      exit(1);
    }
}

  void
KAddressBook::_initIndex()
{
  QDir d(path_ + "/entries");

  d.setFilter(QDir::Files | QDir::NoSymLinks | QDir::Readable);
  
  index_ = d.entryList();
}

  bool
KAddressBook::_writeEntity(Entity & e)
{
  QString filename = path_ + "/tmp" + e.id();

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

  QDataStream str(&f);

  str << e;

  f.flush();
  f.close();

  if (f.status() != IO_Ok) {
    qDebug("Couldn't flush file `" + filename + "'");
    f.remove();
    return false;
  }
  
  QString linkTarget(path_ + "/entries/" + e.id());

  if (::link(QFile::encodeName(filename), QFile::encodeName(linkTarget)) != 0) {
    qDebug("Couldn't successfully link `" + filename +
      "' to `" + linkTarget + "' - giving up");
    return false;
  }

  f.remove();

  return true;
}

  Entity *
KAddressBook::_readEntity(const QString & id)
{
  QString filename = path_ + "/entries/" + id;
  QFile f(filename);

  if (!f.exists()) {
    qDebug("File `" + filename + "' does not exist");
    return 0;
  }
  
  if (!f.open(IO_ReadOnly)) {
    qDebug("Couldn't open `" + filename + "' for reading");
    return 0;
  }
  
  QDataStream str(&f);

  Entity * e = new Entity;
  str >> *e;

  return e;
}

  bool
KAddressBook::_removeEntity(const QString & id)
{
  QFile f(path_ + "/entries/" + id);

  if (!f.exists())
    return false;

  if (!f.remove())
    return false;

  index_.remove(id);

  return true;
}

