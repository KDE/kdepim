/***************************************************************************
                        event.cpp  -  description
                             -------------------
    begin                : Wed Oct 23 2002
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

#include <qtextstream.h>
#include <qdir.h>

#include <kdebug.h>

#include <idhelper.h>

#include "libpv/pvdataentry.h"

#include "event.h"

using namespace KSync;
using namespace PVHelper;

EventSyncee* Event::toEventSyncee(QDomNode& n)
{
  kdDebug() << "Begin::Event::toEventSyncee" << endl;
  EventSyncee* syncee = new KSync::EventSyncee();
  KonnectorUIDHelper helper(QDir::homeDirPath() + "/.kitchensync/meta/idhelper");
  
  QString id;

  while(!n.isNull())
  {
    QDomElement e = n.toElement();
    if( e.tagName() == QString::fromLatin1("event"))
    {
      // convert XML contact to event
      KCal::Event* event = new KCal::Event();      
      helper.addId("Event", e.attribute("uid"), event->uid());
      // Get subentries
      QDomNode n = e.firstChild();
      QDomElement el = n.toElement();

      while(!n.isNull()) // get all sub entries of element todo
      {
        QDomElement el = n.toElement();
        if (el.tagName() == QString::fromLatin1("alarmdate"))
        {
// xxx alarm objekt anhängen!!!
        }
        else if (el.tagName() == QString::fromLatin1("alarmtime"))
        {
// xxx alarm objekt anhängen!!!
        }
        else if (el.tagName() == QString::fromLatin1("category"))
        {
          if (el.text() == "0")
            event->setCategories(QString("A"));
          else if (el.text() == "1")
            event->setCategories(QString("B"));
          else if (el.text() == "2")
            event->setCategories(QString("C"));
          else if (el.text() == "3")
            event->setCategories(QString("D"));
          else if (el.text() == "4")
            event->setCategories(QString("E"));            
        }
        else if (el.tagName() == QString::fromLatin1("duedate"))
        {
/*          event->setHasDueDate(true);
          QDate date(el.text().left(4).toInt(), el.text().mid(4, 2).toInt(), el.text().mid(6, 2).toInt());
          QDateTime time(date);
          event->setDtDue(time);*/
        }
        else if (el.tagName() == QString::fromLatin1("priority"))
        {
          event->setPriority(el.text().toInt());
        }
        else if (el.tagName() == QString::fromLatin1("description"))
        {
          event->setDescription(el.text());
          // Summary = first line of description! Because PV has no summary!
          int i = el.text().find('\n');
          if (i)
          {
            event->setSummary(el.text().left(i));
          }
          else
          {
            event->setSummary(el.text());
          }
        }        
        // Go to next entry
        n = n.nextSibling();
      }  // end of while
      EventSyncEntry* entry = new EventSyncEntry(event);
      // add entry to syncee
      syncee->addEntry(entry);
      // Check state and set it in syncee
      switch (e.attribute("state").toUInt())
      {
        case CasioPV::PVDataEntry::UNDEFINED:
          entry->setState(KSync::SyncEntry::Undefined);
          break;
        case CasioPV::PVDataEntry::ADDED:
          entry->setState(KSync::SyncEntry::Added);
          break;
        case CasioPV::PVDataEntry::MODIFIED:
          entry->setState(KSync::SyncEntry::Modified);
          break;
        case CasioPV::PVDataEntry::REMOVED:
          entry->setState(KSync::SyncEntry::Removed);
          break;
        default:
          break;
      }
    }  // end of if contact
    else
    {
      kdDebug() << "PVHelper::XML2Syncee -> contact not found" << endl;
      return 0l; // xxx bessere fehlerbehandlung!
    }
    n = n.nextSibling(); // jump to next element
  }  // end of while
  kdDebug() << "End::Todo::toTodoSyncee" << endl;
  return syncee;
}

QString Event::toXML(EventSyncee* syncee)
{
  KonnectorUIDHelper helper(QDir::homeDirPath() + "/.kitchensync/meta/idhelper");
  
  QStringList categories;
 
  QString str;
    
  str.append("<todos>\n");
  
  // for all entries
  KCal::Event* event;
  KSync::EventSyncEntry *entry;
    
  for (entry = (KSync::EventSyncEntry*)syncee->firstEntry(); entry != 0l;
                           entry = (KSync::EventSyncEntry*)syncee->nextEntry())
  {    
    QString id;
    QString state;
    QString category;
    // Get todo from entry
    event = entry->incidence();
     
    // Check if id is new
    id =  helper.konnectorId("Event", event->uid());
    if (id.isEmpty())
    {
      // New entry -> set state = added
      state = "added";
    }
    else
    {
      state = "undefined";
    }
      
    // Convert to XML stream
    str.append("<event uid='" + id + "' state='" + state + "' ");
    str.append("category='" + category + "'>\n");

    str.append("<alarmdate></alarmdate>\n");/* xxx alarm objekt extrahieren!!! */
    str.append("<alarmtime></alarmtime>\n");/* xxx alarm objekt extrahieren!!! */
    QStringList categories;
    categories = event->categories(); // xxx only one category supported yet!
    if (categories[0] != "A" && categories[0] != "B" && categories[0] != "C" &&
         categories[0] != "D" && categories[0] != "E")
    {
      kdDebug() << "setting default category!!" << endl;
      str.append("<category>0</category>\n");      
    }
    else
    {
      if (categories[0] == "A")
        str.append("<category>0</category>\n");
      else if (categories[0] == "B")
        str.append("<category>1</category>\n");
      else if (categories[0] == "C")
        str.append("<category>1</category>\n");
      else if (categories[0] == "D")
        str.append("<category>1</category>\n");
      else if (categories[0] == "E")
        str.append("<category>1</category>\n");        
    }
    
    QString strdate;    
/*    if (event->hasDueDate())
    {
      QDate dat = event->dtDue().date();
      strdate = dat.toString("yyyyMMdd");
      str.append("<duedate>" + strdate + "</duedate>\n");
    }      
    else
    {
      str.append("<duedate></duedate>\n");    
    }*/
     
    // If no description, insert summary instead
    QString description = event->description();
    if (description.isEmpty())
    {
      description = event->summary();
    }
    str.append("<description>" + description + "</description>\n");
    str.append("</event>\n");
  } // end for
  str.append("</events>\n");
    
  return str;
}
