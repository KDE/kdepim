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
#include "Command.h"
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

KAddressBookFileBackend::KAddressBookFileBackend
(
 QString id,
 QString path,
 QObject * parent,
 const char * name
)
  : KAddressBookBackend(id, KURL(path).path(), parent, name)
{
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

  void
KAddressBookFileBackend::runCommand(KAB::Command * baseCommand)
{
  qDebug("KAddressBookFileBackend::runCommand()");

  switch (baseCommand->type())
  {
    case KAB::CommandTypeEntry:
      {
        qDebug("CommandTypeEntry");

        KAB::CommandEntry * c(static_cast<KAB::CommandEntry *>(baseCommand));

        qDebug("Doing index.find()");

        QStringList::Iterator it(index_.find(c->entryID()));

        if (index_.end() == it)
        {
          qDebug("**** KAddressBookFileBackend: can't find entry");
          c->setEntry(KAB::Entry());
        }
        else
        {
          KAB::Entry e(_readEntry(*it));

          if (e.isNull())
          {
            qDebug("**** KAddressBookFileBackend: entry read is null!");
          }
          else
          {
            qDebug("**** KAddressBookFileBackend: entry is ok at this end");
          }
          c->setEntry(e);
        }

        emit(commandComplete(c));
      }
      break;

    case KAB::CommandTypeContains:
      qDebug("CommandTypeContains");
      {
        KAB::CommandContains * c =
          static_cast<KAB::CommandContains *>(baseCommand);

        c->setContains(index_.contains(c->entryID()));

        emit(commandComplete(c));
      }
      break;

    case KAB::CommandTypeInsert:
      qDebug("CommandTypeInsert");
      {
        KAB::CommandInsert * c =
          static_cast<KAB::CommandInsert *>(baseCommand);

        bool ok = _writeEntry(c->entry());

        if (!ok)
          c->setSuccess(false);

        else
        {
          index_.append(c->entry().id());
          c->setSuccess(true);
        }

        emit(commandComplete(c));
      }
      break;

    case KAB::CommandTypeRemove:
      qDebug("CommandTypeRemove");
      {
        KAB::CommandRemove * c =
          static_cast<KAB::CommandRemove *>(baseCommand);

        if (!index_.contains(c->entryID()))
          c->setSuccess(false);

        else
        {
          index_.remove(c->entryID());
          _removeEntry(c->entryID());
          c->setSuccess(true);
        }

        emit(commandComplete(c));
      }
      break;

    case KAB::CommandTypeReplace:
      qDebug("CommandTypeReplace");
      {
        KAB::CommandReplace * c =
          static_cast<KAB::CommandReplace *>(baseCommand);

        qDebug("STUB");

        c->setSuccess(false);

        emit(commandComplete(c));
      }
      break;

    case KAB::CommandTypeEntryList:
      qDebug("CommandTypeEntryList");
      {
        KAB::CommandEntryList * c =
          static_cast<KAB::CommandEntryList *>(baseCommand);

        qDebug("Setting entry list to index, which has %d entries", index_.count());
        c->setEntryList(index_);

        qDebug("Completed entry list");
        qDebug("Emitting commandComplete");
        emit(commandComplete(c));
      }
      break;

    default:

      qDebug("Unknown command type");

      break;
  }
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
KAddressBookFileBackend::_writeEntry(const KAB::Entry & e)
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

KAB::Entry
KAddressBookFileBackend::_readEntry(const QString & id) const
{
  QString filename = path() + "/entries/" + id;
  QFile f(filename);

  if (!f.exists()) {
    qDebug("File `" + filename + "' does not exist");
    return KAB::Entry();
  }
  
  if (!f.open(IO_ReadOnly)) {
    qDebug("Couldn't open `" + filename + "' for reading");
    return KAB::Entry();
  }
  
  QDomDocument doc;

  if (!doc.setContent(&f))
  {
    qDebug("Couldn't set content to `" + filename + "'");
    return KAB::Entry();
  }

  QDomElement docElem = doc.documentElement();

  if (docElem.isNull())
  {
    qDebug("Can't parse file `" + filename + "'");
    return KAB::Entry();
  }

  if (docElem.tagName() != "kab:entry")
  {
    qDebug("Can't parse file `" + filename + "'");
    return KAB::Entry();
  }

  return KAB::Entry(docElem);
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

