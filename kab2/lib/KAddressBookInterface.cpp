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
#include <kdatastream.h>
#include <kapp.h>
#include <dcopclient.h>

// Local includes
#include "KAddressBookInterface.h"
#include "KAddressBookBackend.h"
#include "Command.h"

int KABUniqueEntryID = 0;
int KAddressBookInterface::ID_ = 0;

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

  connect
    (
     backend_,
     SIGNAL(commandComplete(KAB::Command *)), 
     this, 
     SLOT(slotCommandComplete(KAB::Command *))
    );
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

  int
KAddressBookInterface::entry(QString id)
{
  return _queueCommand(new KAB::CommandEntry(id));
}

  int
KAddressBookInterface::entryList()
{
  return _queueCommand(new KAB::CommandEntryList);
}

  int
KAddressBookInterface::contains(QString id)
{
  return _queueCommand(new KAB::CommandContains(id));
}

  int
KAddressBookInterface::insert(KAB::Entry e)
{
  return _queueCommand(new KAB::CommandInsert(e));
}

  int
KAddressBookInterface::remove(QString id)
{
  return _queueCommand(new KAB::CommandRemove(id));
}

  int
KAddressBookInterface::replace(KAB::Entry e)
{
  return _queueCommand(new KAB::CommandReplace(e));
}

  int
KAddressBookInterface::_queueCommand(KAB::Command * c)
{
  if (0 == backend_)
    return -1;

  int id = ID_++;

  c->setID(id);

  backend_->queueCommand(c);

  return id;
}

  void
KAddressBookInterface::slotCommandComplete(KAB::Command * baseCommand)
{
  qDebug("KAddressBookInterface::slotCommandComplete()");
  QByteArray params;
  QDataStream str(params, IO_WriteOnly);
  QCString function;

  switch (baseCommand->type())
  {
    case KAB::CommandTypeEntry:
      {
        KAB::CommandEntry * c =
          static_cast<KAB::CommandEntry *>(baseCommand);

        str << c->id() << c->entry();

        function = "entryComplete(int,Entry)";
      }
      break;

    case KAB::CommandTypeContains:
      {
        KAB::CommandContains * c =
          static_cast<KAB::CommandContains *>(baseCommand);

        str << c->id() << c->contains();

        function = "containsComplete(int,bool)";
      }
      break;

    case KAB::CommandTypeInsert:
      {
        KAB::CommandInsert * c =
          static_cast<KAB::CommandInsert *>(baseCommand);

        str << c->id() << c->success();

        function = "insertComplete(int,bool)";
      }
      break;

    case KAB::CommandTypeRemove:
      {
        KAB::CommandRemove * c =
          static_cast<KAB::CommandRemove *>(baseCommand);

        str << c->id() << c->success();

        function = "removeComplete(int,bool)";
      }
      break;

    case KAB::CommandTypeReplace:
      {
        KAB::CommandReplace * c =
          static_cast<KAB::CommandReplace *>(baseCommand);

        str << c->id() << c->success();

        function = "replaceComplete(int,bool)";
      }
      break;

    case KAB::CommandTypeEntryList:
      {
        KAB::CommandEntryList * c =
          static_cast<KAB::CommandEntryList *>(baseCommand);

        str << c->id() << c->entryList();

        function = "entryListComplete(int,QStringList)";
      }
      break;

    default:

      qDebug("Unknown command type");
      return;
      break;
  }

  qDebug("emitting dcop signal with signature `%s'", function.data());
  emitDCOPSignal(function, params);
}

