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
#include <sys/time.h>

// Qt includes
#include <qtimer.h>

// KDE includes
#include <kinstance.h>
#include <kurl.h>

// Local includes
#include "KAddressBookLDAPBackend.h"
#include "Command.h"
#include "Entry.h"

KAddressBookLDAPBackendFactory::KAddressBookLDAPBackendFactory
(
 QObject * parent,
 const char * name
)
  : KLibFactory(parent, name)
{
  new KInstance("kabbackend_ldap");
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
  void * init_libkabbackend_ldap()
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
  : KAddressBookBackend(id, path, parent, name),
    currentCommand_(0)
{
  qDebug("KAddressBookLDAPBackend: ctor");

  pollTimer_ = new QTimer(this, "LDAP poll timer");

  connect(pollTimer_, SIGNAL(timeout()), this, SLOT(slotPoll()));

  KURL url(path);
  
  // XXX host should be retrieved UTF8 ? Don't think so.

  QCString host = url.hasHost() ? url.host().utf8().data() : "localhost";

  int port = 0 != url.port() ? url.port() : LDAP_PORT;

  qDebug("doing ldap_init(`%s', %d)", host.data(), port);

  client_ = ldap_init(const_cast<char *>(host.data()), port);

  if (0 != client_)
  {
    int msgid = ldap_simple_bind(client_, 0, 0);

    if (-1 != msgid)
    {
      expectedMessage_ = msgid;
      pollTimer_->start(100, true);
    }
    else
    {
      qDebug("KAddressBookLDAPBackend: ctor failed - can't ldap_simple_bind");
    }
  }
  else
  {
    qDebug("KAddressBookLDAPBackend: ctor failed - can't ldap_init");
  }
}

KAddressBookLDAPBackend::~KAddressBookLDAPBackend()
{
  if (0 != client_)
    ldap_unbind(client_);
}

  void
KAddressBookLDAPBackend::runCommand(KAB::Command * baseCommand)
{
  currentCommand_ = baseCommand;

  switch (baseCommand->type())
  {
    case KAB::CommandTypeEntry:
      qDebug("CommandTypeEntry");
      {
        KAB::CommandEntry * c(static_cast<KAB::CommandEntry *>(baseCommand));
      }
      break;

    case KAB::CommandTypeContains:
      qDebug("CommandTypeContains");
      {
        KAB::CommandContains * c
          = static_cast<KAB::CommandContains *>(baseCommand);
      }
      break;

    case KAB::CommandTypeInsert:
      qDebug("CommandTypeInsert");
      {
        KAB::CommandInsert * c(static_cast<KAB::CommandInsert *>(baseCommand));
        KAB::Entry e(c->entry());
      }
      break;

    case KAB::CommandTypeRemove:
      qDebug("CommandTypeRemove");
      {
        KAB::CommandRemove * c(static_cast<KAB::CommandRemove *>(baseCommand));
      }
      break;

    case KAB::CommandTypeReplace:
      qDebug("CommandTypeReplace");
      {
        KAB::CommandReplace * c =
          static_cast<KAB::CommandReplace *>(baseCommand);
        KAB::Entry e(c->entry());
      }
      break;

    case KAB::CommandTypeEntryList:
      qDebug("CommandTypeEntryList");
      {
        KAB::CommandEntryList * c =
          static_cast<KAB::CommandEntryList *>(baseCommand);

        char * base = 0;
        int scope = LDAP_SCOPE_SUBTREE;
        char * filter = const_cast<char *>("kab-id=*");
        char * attrs[] = { const_cast<char *>("kab-id"), 0 };
        int attrsonly = 0;

        int msgid = ldap_search(client_, base, scope, filter, attrs, attrsonly);

        if (-1 != msgid)
        {
          expectedMessage_ = msgid;
          pollTimer_->start(100, true);
        }
        else
        {
          ldap_perror(client_, "ldap_search");
        }
      }
      break;

    default:

      qDebug("Unknown command type");

      break;
  }
}

  void
KAddressBookLDAPBackend::slotPoll()
{
//  qDebug("KAddressBookLDAPBackend::slotPoll()");

  struct timeval tv;
  tv.tv_sec = tv.tv_usec = 0;

  LDAPMessage * result;

  int i = ldap_result(client_, expectedMessage_, 0, &tv, &result);

  if (-1 == i)
  {
    qDebug("Something bad happened doing ldap_result");
    return;
  }

  if (0 == i)
  {
//    qDebug("Nothing going on. Polling again.");
    pollTimer_->start(100, true);
    return;
  }

  switch (i)
  {
    case LDAP_RES_BIND:
      {
        qDebug("Got LDAP_RES_BIND");

        int ret = ldap_result2error(client_, result, 1);
        
        if (LDAP_SUCCESS == ret)
        {
          qDebug("Bind ok :)");
          setInitSuccess();
        }
        else
        {
          qDebug("Bind failed :(");
          qDebug("%s", ldap_err2string(ret));
        }
      }
      break;

    case LDAP_RES_SEARCH_ENTRY:
      {
        qDebug("Got LDAP_RES_SEARCH_ENTRY");
        KAB::CommandEntryList * c =
          static_cast<KAB::CommandEntryList *>(currentCommand_);
        QStringList l(c->entryList());
        LDAPMessage * entry = ldap_first_entry(client_, result);

        if (0 == entry)
        {
          qDebug("Er, no first entry in search result");
          ldap_msgfree(result);
          break;
        }
        else
        {
          BerElement * elem;
          char * attr = ldap_first_attribute(client_, entry, &elem);

          qDebug("First attr: `%s'", attr);
        }
        c->setEntryList(l);
        ldap_msgfree(result);
      }
      break;

    case LDAP_RES_SEARCH_REFERENCE:
      qDebug("Got LDAP_RES_SEARCH_REFERENCE");
      break;

    case LDAP_RES_SEARCH_RESULT:
      qDebug("Got LDAP_RES_SEARCH_RESULT");
      ldap_msgfree(result);
      emit(commandComplete(currentCommand_));
      break;

    case LDAP_RES_MODIFY:
      qDebug("Got LDAP_RES_MODIFY");
      qDebug("Result: %s", ldap_err2string(ldap_result2error(client_, result, 1)));
      break;

    case LDAP_RES_ADD:
      qDebug("Got LDAP_RES_ADD");
      qDebug("Result: %s", ldap_err2string(ldap_result2error(client_, result, 1)));
      break;

    case LDAP_RES_DELETE:
      qDebug("Got LDAP_RES_DELETE");
      qDebug("Result: %s", ldap_err2string(ldap_result2error(client_, result, 1)));
      break;

    case LDAP_RES_MODRDN:
      qDebug("Got LDAP_RES_MODRDN");
      break;

    case LDAP_RES_COMPARE:
      qDebug("Got LDAP_RES_COMPARE");
      break;

    case LDAP_RES_EXTENDED:
      qDebug("Got LDAP_RES_EXTENDED");
      break;

    case LDAP_RES_EXTENDED_PARTIAL:
      qDebug("Got LDAP_RES_EXTENDED_PARTIAL");
      break;


    default:
      qDebug("Got unknown result type %d from ldap_result", i);
      break;
  }
}


#include "KAddressBookLDAPBackend.moc"
