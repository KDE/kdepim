/***************************************************************************
                         helper.cpp  -  description
                             -------------------
    begin                : Wed Oct 09 2002
    copyright            : (C) 2002 by Maurus Erni
    email                : erni@pocketviewer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qbuffer.h>
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>

#include <kapplication.h>
#include <kdebug.h>

#include <kabc/resourcefile.h>
#include <kabc/phonenumber.h>
#include <kabc/address.h>

#include <idhelper.h>

// project includes
#include "addressbook.h"
#include "event.h"
#include "todo.h"

#include "helper.h"

using namespace KSync;
using namespace PVHelper;
 
Syncee::PtrList Helper::XML2Syncee(const QByteArray& array)
{
  kdDebug() << "Begin:PVHelper::XML2Syncee: " << endl;
  Syncee::PtrList sync;
  QDomDocument doc("mydocument");
  // Set content of DOM document
  if(doc.setContent(array))
  {
    // Parse DOM Document
    QDomElement docElem = doc.documentElement();
    if (docElem.tagName() == QString::fromLatin1("pvdataentries"))
    {
      kdDebug() << "pvdataentries found!" << endl;
      QDomNode n =  docElem.firstChild(); // child of pvdataentries -> type of entries (e.g. contacts)
      QDomElement e = n.toElement();
      if( e.tagName() == QString::fromLatin1("contacts"))
      {
        AddressBookSyncee* syncee = new AddressBookSyncee();
        QDomNode n = e.firstChild();
        syncee = AddressBook::toAddressBookSyncee(n);
        sync.append(syncee);
      }
      else if( e.tagName() == QString::fromLatin1("events"))
      {
        EventSyncee* syncee = new EventSyncee();
        QDomNode n = e.firstChild();
        syncee = Event::toEventSyncee(n);
        sync.append(syncee);
      }
      if( e.tagName() == QString::fromLatin1("todos"))
      {
        TodoSyncee* syncee = new TodoSyncee();
        QDomNode n = e.firstChild();
        syncee = Todo::toTodoSyncee(n);
        sync.append(syncee);
      }
    }  // end of if pvdataentries
    else
    {
      kdDebug() << "PVHelper::XML2Syncee -> pvdataentries not found" << endl;
      return /*0L*/; // xxx syncee auf null setzen? wie geht das?
    }
    return sync;
  }  // end of if doc.setContents()
  else
  {
    kdDebug() << "PVHelper::XML2Syncee !doc.setContent() " << endl;
    return /*0L*/; // xxx syncee auf null setzen? wie geht das?
  }
}

QByteArray Helper::Syncee2XML(KSync::Syncee::PtrList* synceePtrListNew)
{
  kdDebug() << "Syncee2XML" << endl;
  //  ok lets write back the changes from the Konnector
  QByteArray array;
  QBuffer buffer(array);
  if (buffer.open( IO_WriteOnly))
  {
    if (!synceePtrListNew->isEmpty())
    {
      Syncee* syncee;
      QTextStream stream( &buffer );
      stream.setEncoding( QTextStream::UnicodeUTF8 );
      stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE pvdataentries>" << endl
              << "<pvdataentries>" << endl;
      for (syncee = synceePtrListNew->first(); syncee != 0; syncee = synceePtrListNew->next())
      {
        if (syncee->type() == QString::fromLatin1("AddressBookSyncee"))
        {
          stream << (AddressBook::toXML(dynamic_cast<AddressBookSyncee*>(syncee))).data();
        }
        else if (syncee->type() == QString::fromLatin1("EventSyncee"))
        {
          stream << (Event::toXML(dynamic_cast<EventSyncee*>(syncee))).data();
        }
        if (syncee->type() == QString::fromLatin1("TodoSyncee"))
        {
          stream << (Todo::toXML(dynamic_cast<TodoSyncee*>(syncee))).data();
        }
      }
      stream << "</pvdataentries>" << endl;
    }  // end if
  }
    // now replace the UIDs for us -> xxx for what??
//    m_helper->replaceIds( "AddressBookSyncEntry",  m_kde2opie ); // to keep the use small
  return array;
}


KSync::Syncee::PtrList Helper::doMeta(KSync::Syncee::PtrList* synceePtrListOld,
                                         KSync::Syncee::PtrList* synceePtrListNew)
{
  kdDebug() << "PVHelper::doMeta ListOld count =" << synceePtrListOld->count() << endl;
  kdDebug() << "PVHelper::doMeta ListNew count =" << synceePtrListNew->count() << endl;
  KSync::Syncee::PtrList synceePtrListMeta;
  Syncee* syncee;
  for (syncee = synceePtrListNew->first(); syncee != 0; syncee = synceePtrListNew->next())
  {
    if (syncee->type() == QString::fromLatin1("AddressBookSyncee"))
    {
      kdDebug() << "AddressBookSyncee found" << endl;
/*      AddressBookSyncee* abSynceeNew = dynamic_cast<AddressBookSyncee*>(syncee);
      unsigned int index = synceePtrListOld->find("AddressBookSyncee");
      if (index)
      {
        AddressBookSyncee* abSynceeOld = synceePtrListOld->currentNode();
        abNewSyncee = doMeta(abSynceeOld, abSynceeNew);
        synceePtrListNew->remove();
        synceePtrListNew->append(abNewSyncee);
      }*/
    }
    else if (syncee->type() == QString::fromLatin1("EventSyncee"))
    {
      kdDebug() << "EventSyncee found" << endl;
/*      EventSyncee* evSynceeNew = dynamic_cast<EventSyncee*>(syncee);
      unsigned int index = synceePtrListOld->find("EventSyncee");
      if (index)
      {
        EventSyncee* evSynceeOld = synceePtrListOld->currentNode();
        doMeta(evSynceeOld, evSynceeNew);
        synceePtrListNew->remove();
        synceePtrListNew->append(evNewSyncee);
      }*/
    }
    else if (syncee->type() == QString::fromLatin1("TodoSyncee"))
    {
      kdDebug() << "TodoSyncee found" << endl;
/*      TodoSyncee* toSynceeNew = dynamic_cast<TodoSyncee*>(syncee);
      unsigned int index = synceePtrListOld->find("TodoSyncee");
      if (index)
      {
        TodoBookSyncee* toSynceeOld = synceePtrListOld->currentNode();
        doMeta(toSynceeOld, toSynceeNew);
        synceePtrListNew->remove();
        synceePtrListNew->append(toNewSyncee);
      }*/
    }// else if UnknownSyncEntry.... upload

  }
//  lis.setAutoDelete( true );
//  lis.clear();
  return synceePtrListMeta;
}
