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
#include <kdebug.h>
#include <kinstance.h>
#include <kurl.h>

// Local includes
#include <kab2/Command.h>
#include <kab2/Entry.h>
#include <kab2/Field.h>

#include "KAddressBookFileBackend.h"

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
    kdDebug() << "Cannot create dirs. I'm broken." << endl;
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
  kdDebug() << "KAddressBookFileBackend::runCommand()" << endl;

  switch (baseCommand->type())
  {
    case KAB::CommandTypeEntry:
      {
        kdDebug() << "CommandTypeEntry" << endl;

        KAB::CommandEntry * c(static_cast<KAB::CommandEntry *>(baseCommand));

        kdDebug() << "Doing index.find()" << endl;

        QStringList::Iterator it(index_.find(c->entryID()));

        if (index_.end() == it)
        {
          kdDebug() << "KAddressBookFileBackend: can't find entry" << endl;
          c->setEntry(KAB::Entry());
        }
        else
        {
          KAB::Entry e(_readEntry(*it));

          if (e.isNull())
          {
            kdDebug() << "KAddressBookFileBackend: entry read is null!" << endl;
          }
          else
          {
            kdDebug() << "KAddressBookFileBackend: entry is ok at this end"
              << endl;
          }
          c->setEntry(e);
        }

        emit(commandComplete(c));
      }
      break;

    case KAB::CommandTypeContains:
      kdDebug() << "CommandTypeContains" << endl;
      {
        KAB::CommandContains * c =
          static_cast<KAB::CommandContains *>(baseCommand);

        c->setContains(index_.contains(c->entryID()));

        emit(commandComplete(c));
      }
      break;

    case KAB::CommandTypeInsert:
      kdDebug() << "CommandTypeInsert";
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
      kdDebug() << "CommandTypeRemove" << endl;
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
      kdDebug() << "CommandTypeReplace" << endl;
      {
        KAB::CommandReplace * c =
          static_cast<KAB::CommandReplace *>(baseCommand);

        kdDebug() << "STUB" << endl;

        c->setSuccess(false);

        emit(commandComplete(c));
      }
      break;

    case KAB::CommandTypeEntryList:
      kdDebug() << "CommandTypeEntryList" << endl;
      {
        KAB::CommandEntryList * c =
          static_cast<KAB::CommandEntryList *>(baseCommand);

        kdDebug() << "Setting entry list to index, which has "
          << index_.count() << " entries" << endl;

        c->setEntryList(index_);

        kdDebug() << "Completed entry list" << endl;
        kdDebug() << "Emitting commandComplete" << endl;
        emit(commandComplete(c));
      }
      break;

    default:

      kdDebug() << "Unknown command type" << endl;

      break;
  }
}

  bool
KAddressBookFileBackend::_checkDirs()
{
  QDir base(path());

  kdDebug() << "KAddressBookFileBackend::_checkDirs(): path == "
    << path() << endl;

  if (!base.exists())
    if (!base.mkdir(path()))
    {
      kdDebug() << "Could not create dir `" + path() + "' - exiting" << endl;
      return false;
    }

  QDir tmp(path() + "/tmp");
  
  if (!tmp.exists())
    if (!tmp.mkdir(path() + "/tmp"))
    {
      kdDebug() << "Could not create dir `" + path() + "/tmp' - exiting"
        << endl;

      return false;
    }

  QDir entries(path() + "/entries");
  
  if (!entries.exists())
    if (!entries.mkdir(path() + "/entries"))
    {
      kdDebug() << "Could not create dir `" + path() + "/entries' - exiting"
        << endl;
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
  kdDebug() << "KAddressBook::_writeEntry()" << endl;

  QString filename = path() + "/tmp" + e.id();

  QFile f(filename);

  if (f.exists())
  {
    kdDebug() << "File `" << filename << "' exists" << endl;
    usleep(2000);
  }

  if (f.exists())
  {
    kdDebug() << "File `" << filename << "' still exists" << endl;
    return false;
  }

  if (!f.open(IO_WriteOnly))
  {
    kdDebug() << "Couldn't open file `" << filename << "' for writing" << endl;
    return false;
  }

  QDomDocument doc("kab-entry");

  e.insertInDomTree(doc, doc);

  kdDebug() << "Doc as string: " << doc.toString() << endl;

  QTextStream str(&f);

  str << doc.toString();

  f.flush();
  f.close();

  if (f.status() != IO_Ok)
  {
    kdDebug() << "Couldn't flush file `" << filename << "'" << endl;
    f.remove();
    return false;
  }
  
  QString linkTarget(path() + "/entries/" + e.id());

  if (::link(QFile::encodeName(filename), QFile::encodeName(linkTarget)) != 0) {
    kdDebug() << "Couldn't successfully link `" 
      << filename  << "' to `"  << linkTarget << "' - giving up" << endl;
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

  if (!f.exists())
  {
    kdDebug() << "File `" << filename << "' does not exist" << endl;
    return KAB::Entry();
  }
  
  if (!f.open(IO_ReadOnly))
  {
    kdDebug() << "Couldn't open `" << filename << "' for reading" << endl;
    return KAB::Entry();
  }
  
  QDomDocument doc;

  if (!doc.setContent(&f))
  {
    kdDebug() << "Couldn't set content to `" << filename << "'" << endl;
    return KAB::Entry();
  }

  QDomElement docElem = doc.documentElement();

  if (docElem.isNull())
  {
    kdDebug() << "Can't parse file `" << filename << "'" << endl;
    return KAB::Entry();
  }

  if (docElem.tagName() != "kab:entry")
  {
    kdDebug() << "Can't parse file `" << filename << "'" << endl;
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

