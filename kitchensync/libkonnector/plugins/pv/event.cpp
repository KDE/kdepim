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
#include <qbitarray.h>
#include <qdir.h>

#include <idhelper.h>

#include <kdebug.h>

#include "libpv/pvdataentry.h"
#include "utils.h"

#include "event.h"

using namespace KSync;
using namespace PVHelper;

/**
  * Converts a QDomNode to an EventSyncee*.
  * @param node The node (part of an XML document) to be converted
  * @return KSync::EventSyncee* The converted events
  */
EventSyncee* Event::toEventSyncee(QDomNode& n)
{
  kdDebug(5205) << "Begin::Event::toEventSyncee" << endl;
  EventSyncee* syncee = new KSync::EventSyncee();
  // Define the helper
  KonnectorUIDHelper helper(QDir::homeDirPath() + "/.kitchensync/meta/idhelper");

  QString id;

  while(!n.isNull())
  {
    QDomElement e = n.toElement();
    // Check the elements and fill the contents to the event
    if( e.tagName() == QString::fromLatin1("event"))
    {
      KCal::Event* event = new KCal::Event();
      // Dates of the event
      QDateTime startDate;
      QDateTime endDate;
      // Recurrence and type of recurrence
      KCal::Recurrence *rec = event->recurrence();
      int type = 0xFF;

      // Register Id of entry
      helper.addId(e.attribute("category"), e.attribute("uid"), event->uid());

      // Get subentries
      QDomNode n = e.firstChild();
      QDomElement el = n.toElement();

      while(!n.isNull()) // get all sub entries of element event
      {
        QDomElement el = n.toElement();
        // Check the elements and fill the contents to the event
        if (el.tagName() == QString::fromLatin1("type"))
        {
          // Handle recurrence
          type = el.text().toInt();
        }
        else if (el.tagName() == QString::fromLatin1("date"))
        {
          if (el.text() != "")
          {
            // Convert the string to a QDate
            QDate dat;
            dat.setYMD(el.text().left(4).toInt(), el.text().mid(4, 2).toInt(),
                        el.text().mid(6, 2).toInt());
            startDate.setDate(dat);

            if (type != 0xFF)
            {
              // Set recurrence depending on type
              setRecurrence(rec, startDate, type);
            }
          }
        }
        else if (el.tagName() == QString::fromLatin1("enddate"))
        {
          if (el.text() != "")
          {
            // Convert the string to a QDate
            QDate dat;
            dat.setYMD(el.text().left(4).toInt(), el.text().mid(4, 2).toInt(),
                        el.text().mid(6, 2).toInt());
            QDateTime datTime(dat);
            event->setDtEnd(datTime);
          }
        }
        else if (el.tagName() == QString::fromLatin1("starttime"))
        {
          if (el.text() != "")
          {
            // Time should be calculated depending on timezone -> dirty hack! xxx
            QTime startTime(el.text().left(2).toInt() - 1, el.text().right(2).toInt());
            startDate.setTime(startTime);
            event->setFloats(false);
          }
        }
        else if (el.tagName() == QString::fromLatin1("endtime"))
        {
          if (el.text() != "")
          {
            // Time should be calculated depending on timezone -> dirty hack! xxx
            QTime endTime(el.text().left(2).toInt() - 1, el.text().right(2).toInt());
            endDate.setTime(endTime);
            endDate.setDate(startDate.date());
            event->setDtEnd(endDate);
          }
        }
        if (el.tagName() == QString::fromLatin1("alarmtime"))
        {
          if (el.text() != "")
          {
            // Get time.
            // Time should be calculated depending on timezone -> dirty hack! xxx
            QTime tim(el.text().left(2).toInt() - 1, el.text().right(2).toInt());
            QTime alarmTime(tim);
            // Calculate offset
            int offset = alarmTime.secsTo(startDate.time());
            // Add alarm as an incidence of this todo
            KCal::Alarm *al = new KCal::Alarm(event);
            al->setDisplayAlarm(event->description());
            al->setStartOffset(offset * -1);  // * -1 -> alarm has to be before event
            al->setEnabled(true);
            event->addAlarm(al);
          }
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

      // Set start date
      event->setDtStart(startDate);

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
      kdDebug(5205) << "PVHelper::XML2Syncee -> event not found" << endl;
      return 0l; // xxx bessere fehlerbehandlung!
    }
    n = n.nextSibling(); // jump to next element
  }  // end of while
  kdDebug(5205) << "End::Todo::toTodoSyncee" << endl;
  return syncee;
}

/**
  * Converts an EventSyncee* to a QString which represents a
  * DOM node.
  * @param syncee The syncee to be converted
  * @return QString The converted events
  */
QString Event::toXML(EventSyncee* syncee)
{
  // Define the helper
  KonnectorUIDHelper helper(QDir::homeDirPath() + "/.kitchensync/meta/idhelper");

  QStringList categories;

  QString str;

  str.append("<events>\n");

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

    // Recurrence
    KCal::Recurrence *rec;
    rec = event->recurrence();

    // determine the category
    if (rec->doesRecur())
    {
      category = "Schedule Reminder";
    }
    else if (event->isMultiDay())
    {
      category = "Schedule Multi Date";
    }
    else
    {
      category = "Schedule";
    }

    // Check if id is new
    id =  helper.konnectorId(category, event->uid());
    if (id.isEmpty())
    {
      // New entry -> set state = added
      state = "added";
    }
    else
    {
      state = "undefined";
    }

    // Get start and end date;
    // Time should be calculated depending on timezone -> dirty hack! xxx
    QDateTime datStart = event->dtStart().addSecs(3600);
    // Time should be calculated depending on timezone -> dirty hack! xxx
    QDateTime datEnd = event->dtEnd().addSecs(3600);

    // Convert to XML stream
    str.append("<event uid='" + id + "' state='" + state + "' ");
    str.append("category='" + category + "'>\n");

    // Type is only defined on "Schedule Reminder"
    if (category == "Schedule Reminder")
    {
      QString type = getType(rec);
      str.append("<type>" + type + "</type>\n");
    }

    str.append("<date>" + datStart.toString("yyyyMMdd") + "</date>\n");

    // End Date is only defined on "Schedule Multi Date"
    if (category == "Schedule Multi Date")
    {
      str.append("<enddate>" + datEnd.toString("yyyyMMdd") + "</enddate>\n");
    }
    else  // Categories "Schedule" and "Schedule Reminder"
    {
      str.append("<starttime>" + datStart.toString("hhmm") + "</starttime>\n");
      str.append("<endtime>" + datEnd.toString("hhmm") + "</endtime>\n");

      // Alarm
      KCal::Alarm *al = event->alarms().first();
      if (al)
      {

        QDateTime datTime = al->time().addSecs(3600);
        str.append("<alarmtime>" + datTime.toString("hhmm") + "</alarmtime>\n");
      }
      else
      {
        str.append("<alarmtime></alarmtime>\n");
      }
    }

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

/**
  * Handles the recurrence of an event. The recurrence will be set
  * depending on the start date and the type (daily, weekly, ...)
  * @param rec The recurrence pointer. Will be modified inside the method
  * @param startDate The start date of the event
  * @param type The type of the recurrence used in the PV
  */
void Event::setRecurrence(KCal::Recurrence *rec, QDateTime startDatTime, int type)
{
  QBitArray bits(7);  // Weekdays are used as a bitarray
  int week;       // Week of month

  QDate startDate = startDatTime.date();

  // Set recurrence start date
  rec->setRecurStart(startDatTime);

  // Set recurrence depending on type
  switch (type)
  {
    case 0:
      // Daily
      rec->setDaily(1, -1);  // -1 stands for "no end date"
      break;
    case 1:
      // Weekly
      bits.fill(false); // clear bits
      bits.setBit(startDate.dayOfWeek() - 1);  // -1 needed because dayOfWeek returns 1..7
      rec->setWeekly(1, bits, -1);
      break;
    case 2:
      // Monthly
      rec->setMonthly(KCal::Recurrence::rMonthlyDay, 1, -1);
      rec->addMonthlyDay(startDate.day());
      break;
    case 3:
      // Monthly day (e.g. the 1st Saturday of the month)
      // Calculate the position (week of month)
      // e.g. 15. = week 2 -> position = 2
      week = (((startDate.day() - 1) / 7) + 1);
      rec->setMonthly(KCal::Recurrence::rMonthlyPos, 1, -1);
      bits.fill(false); // clear bits
      bits.setBit(startDate.dayOfWeek() - 1);
      // Set position of recurrence
      // e.g. each 2nd Wednesday -> pos. = 2; bit Wednesday is set
      rec->addMonthlyPos(week, bits);
      break;
    case 4:
      // yearly
      rec->setYearly(KCal::Recurrence::rYearlyDay, 1, -1);
      rec->addYearlyNum(startDate.dayOfYear());
      break;
    case 5:
      // Yearly day (e.g. the 1st Saturday of july)
      // Calculate the position (week of month)
      // e.g. 15. = week 2 -> position = 2
      week = (((startDate.day() - 1) / 7) + 1);
      // set frequency to 12 -> Cheat to get this category to work!
      rec->setMonthly(KCal::Recurrence::rMonthlyPos, 12, -1);

      bits.fill(false); // clear bits
      bits.setBit(startDate.dayOfWeek() - 1);
      // Set position of recurrence
      // e.g. each 2nd Wednesday -> pos. = 2; bit Wednesday is set
      rec->addMonthlyPos(week, bits);
      break;
    default:
      break;
  }  // switch
}

/**
  * Gets the type of the recurrence of an event.
  * @param rec The recurrence pointer of the event
  * @return QString The type of the recurrence used in PV
  */
QString Event::getType(KCal::Recurrence *rec)
{
  QString type;
  switch(rec->doesRecur())
  {
    case KCal::Recurrence::rDaily :
      // Daily recurrence
      type = "0";
      break;
    case KCal::Recurrence::rWeekly :
      // Weekly recurrence
      type = "1";
      break;
    case KCal::Recurrence::rMonthlyDay :
      // Monthly recurrence (same day each month e.g. 15.)
      type = "2";
      break;
    case KCal::Recurrence::rMonthlyPos :
      if (rec->frequency() == 12)
      {
        // Yearly recurrence at a specified day (e.g. 2nd friday in july)
        type = "5";
      }
      else
      {
        // Monthly recurrence (same weekday each month e.g. 2nd friday)
        type = "3";
      }
      break;
    case KCal::Recurrence::rYearlyDay :
      // Yearly recurrence (each july 15th)
      type = "4";
      break;

    case KCal::Recurrence::rNone : // fall through
    default :
      break;
  }
  return type;
}
