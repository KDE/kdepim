/***************************************************************************
                        todo.cpp  -  description
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

#include "todo.h"

using namespace KSync;
using namespace PVHelper;

/**
  * Converts a QDomNode to a TodoSyncee*.
  * @param node The node (part of an XML document) to be converted
  * @return KSync::TodoSyncee* The converted todos
  */
TodoSyncee* Todo::toTodoSyncee(QDomNode& n)
{
  kdDebug(5205) << "Begin::Todo::toTodoSyncee" << endl;
  TodoSyncee* syncee = new KSync::TodoSyncee();
  // Define the helper
  KonnectorUIDHelper helper(QDir::homeDirPath() + "/.kitchensync/meta/idhelper");

  QString id;

  while(!n.isNull())
  {
    QDomElement e = n.toElement();
    if( e.tagName() == QString::fromLatin1("todo"))
    {
      // convert XML contact to todo
      KCal::Todo* todo = new KCal::Todo();
      helper.addId("Todo", e.attribute("uid"), todo->uid());
      QDate alarmDate;
      // Get subentries
      QDomNode n = e.firstChild();
      QDomElement el = n.toElement();

      while(!n.isNull()) // get all sub entries of element todo
      {
        QDomElement el = n.toElement();
        // Check the elements and fill the contents to the todo
        if (el.tagName() == QString::fromLatin1("check"))
        {
          if (el.text() == "0")
          {
            todo->setCompleted(false);
          }
          else if (el.text() == "1")
          {
            todo->setCompleted(true);
          }
        }
        else if (el.tagName() == QString::fromLatin1("duedate"))
        {
          todo->setHasDueDate(true);
          QDate date(el.text().left(4).toInt(), el.text().mid(4, 2).toInt(), el.text().mid(6, 2).toInt());
          QDateTime datTime(date);
          todo->setDtDue(datTime);
        }
        else if (el.tagName() == QString::fromLatin1("alarmdate"))
        {
          if (el.text() != "")
          {
            // Set alarm date
            alarmDate.setYMD(el.text().left(4).toInt(), el.text().mid(4, 2).toInt(),
                                   el.text().mid(6, 2).toInt());
          }
        }
        else if (el.tagName() == QString::fromLatin1("alarmtime"))
        {
          if (el.text() != "")
          {
            // Get time. Put date and time to a QDateTime
            // Time should be calculated depending on timezone -> dirty hack! xxx
            QTime alarmTime(el.text().left(2).toInt(), el.text().right(2).toInt());
            QDateTime datTime(alarmDate, alarmTime);

            // Calculate offset
            int offset = datTime.secsTo(todo->dtDue());
            // Add alarm as an incidence of this todo
            KCal::Alarm *al = new KCal::Alarm(todo);
            al->setDisplayAlarm(todo->description());
            al->setStartOffset(offset * -1);  // * -1 -> alarm has to be before event
            al->setEnabled(true);
            todo->addAlarm(al);
          }
        }
        else if (el.tagName() == QString::fromLatin1("checkdate"))
        {
          if (!el.text().isEmpty())
          {
            QDate date(el.text().left(4).toInt(), el.text().mid(4, 2).toInt(), el.text().mid(6, 2).toInt());
            QDateTime datTime(date);
            todo->setCompleted(datTime);
          }
        }
        else if (el.tagName() == QString::fromLatin1("priority"))
        {
          todo->setPriority(el.text().toInt());
        }
        else if (el.tagName() == QString::fromLatin1("category"))
        {
          if (el.text() == "0")
            todo->setCategories(QString("A"));
          else if (el.text() == "1")
            todo->setCategories(QString("B"));
          else if (el.text() == "2")
            todo->setCategories(QString("C"));
          else if (el.text() == "3")
            todo->setCategories(QString("D"));
          else if (el.text() == "4")
            todo->setCategories(QString("E"));
        }
        else if (el.tagName() == QString::fromLatin1("description"))
        {
          todo->setDescription(el.text());
          // Summary = first line of description! Because PV has no summary!
          int i = el.text().find('\n');
          if (i)
          {
            todo->setSummary(el.text().left(i));
          }
          else
          {
            todo->setSummary(el.text());
          }
        }
        // Go to next entry
        n = n.nextSibling();
      }  // end of while
      TodoSyncEntry* entry = new TodoSyncEntry(todo);
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
      kdDebug(5205) << "PVHelper::XML2Syncee -> contact not found" << endl;
      return 0l; // xxx bessere fehlerbehandlung!
    }
    n = n.nextSibling(); // jump to next element
  }  // end of while
  kdDebug(5205) << "End::Todo::toTodoSyncee" << endl;
  return syncee;

}

/**
  * Converts a TodoSyncee* to a QString which represents a
  * DOM node.
  * @param syncee The syncee to be converted
  * @return QString The converted todos as an XML string
  */
QString Todo::toXML(TodoSyncee* syncee)
{
  // Define the helper
  KonnectorUIDHelper helper(QDir::homeDirPath() + "/.kitchensync/meta/idhelper");

  QStringList categories;

  QString str;

  str.append("<todos>\n");

  // for all entries
  KCal::Todo* todo;
  KSync::TodoSyncEntry *entry;

  for (entry = (KSync::TodoSyncEntry*)syncee->firstEntry(); entry != 0l;
                           entry = (KSync::TodoSyncEntry*)syncee->nextEntry())
  {
    QString id;
    QString state;
    // Get todo from entry
    todo = entry->todo();

    // Check if id is new
    id =  helper.konnectorId("Todo", todo->uid());
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
    str.append("<todo uid='" + id + "' state='" + state + "'>\n");

    if (todo->isCompleted())
    {
      str.append("<check>1</check>\n");
    }
    else
    {
      str.append("<check>0</check>\n");
    }
    if (todo->hasDueDate())
    {
      QDateTime datTime = todo->dtDue();
      str.append("<duedate>" + datTime.toString("yyyyMMdd") + "</duedate>\n");
    }
    else
    {
      str.append("<duedate></duedate>\n");
    }

    // alarm
    KCal::Alarm *al = todo->alarms().first();
    if (al)
    {
    // Time should be calculated depending on timezone -> dirty hack! xxx
      QDateTime datTime = al->time();
      str.append("<alarmdate>" + datTime.toString("yyyyMMdd") + "</alarmdate>\n");
      str.append("<alarmtime>" + datTime.toString("hhmm") + "</alarmtime>\n");
    }
    else
    {
      str.append("<alarmdate></alarmdate>\n");
      str.append("<alarmtime></alarmtime>\n");
    }
    if (todo->isCompleted())
    {
      QDateTime datTime = todo->completed();
      str.append("<checkdate>" + datTime.toString("yyyyMMdd") + "</checkdate>\n");
    }
    else
    {
      str.append("<checkdate></checkdate>\n");
    }

    // Check priority -> has to be 1..3 (as defined on PV)
    unsigned int prio = todo->priority();
    if (prio > 3)
    {
      prio = 3;
    }
    str.append("<priority>" + QString::number(prio) + "</priority>\n");

    QStringList categories;
    categories = todo->categories(); // xxx only one category supported yet!
    if (categories[0] != "A" && categories[0] != "B" && categories[0] != "C" &&
         categories[0] != "D" && categories[0] != "E")
    {
      kdDebug(5205) << "setting default category!!" << endl;
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

    // If no description, insert summary instead
    QString description = todo->description();
    if (description.isEmpty())
    {
      description = todo->summary();
    }
    str.append("<description>" + description + "</description>\n");
    str.append("</todo>\n");
  } // end for
  str.append("</todos>\n");

  return str;
}
