/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_VCALFORMAT_H
#define KCAL_VCALFORMAT_H

#include "calformat.h"

#include "todo.h"
#include "event.h"
#include "libkcal_export.h"

#define _VCAL_VERSION "1.0"

class VObject;

namespace KCal {

/**
  This class implements the vCalendar format. It provides methods for
  loading/saving/converting vCalendar format data into the internal KOrganizer
  representation as Calendar and Events.

  @short vCalendar format implementation
*/
class LIBKCAL_EXPORT VCalFormat : public CalFormat
{
  public:
    VCalFormat();
    virtual ~VCalFormat();

    /**
      Loads a calendar on disk in vCalendar format into the given calendar.

      @param calendar Calendar object the loaded data is stored into.
      @param fileName Name of the vCalendar file on disk.
      @return true on success, otherwise false
    */
    bool load( Calendar *calendar, const QString &fileName );
    /**
      Writes out the given calendar to disk in vCalendar format.

      @param calendar Calendar object holding data to be written
      @param fileName the name of the file
      @return true on success, otherwise false
    */
    bool save(Calendar *calendar, const QString &fileName);

    /**
      Parse string and populate calendar with that information.
    */
    bool fromString( Calendar *, const QString & );
    /**
      Return calendar information as string.
    */
    QString toString( Calendar * );

  protected:
    /** translates a VObject of the TODO type into a Event */
    Todo *VTodoToEvent(VObject *vtodo);
    /** translates a VObject into a Event and returns a pointer to it. */
    Event *VEventToEvent(VObject *vevent);
    /** translate a Event into a VTodo-type VObject and return pointer */
    VObject *eventToVTodo(const Todo *anEvent);
    /** translate a Event into a VObject and returns a pointer to it. */
    VObject* eventToVEvent(const Event *anEvent);

    /** takes a QDate and returns a string in the format YYYYMMDDTHHMMSS */
    QString qDateToISO(const QDate &);
    /** takes a QDateTime and returns a string in format YYYYMMDDTHHMMSS */
    QString qDateTimeToISO(const QDateTime &, bool zulu=TRUE);
    /** takes a string in the format YYYYMMDDTHHMMSS and returns a
     * valid QDateTime. */
    QDateTime ISOToQDateTime(const QString & dtStr);
    /** takes a string in the format YYYYMMDD and returns a
     * valid QDate. */
    QDate ISOToQDate(const QString & dtStr);
    /** takes a vCalendar tree of VObjects, and puts all of them that have
     * the "event" property into the dictionary, todos in the todo-list, etc. */
    void populate(VObject *vcal);

    /** takes a number 0 - 6 and returns the two letter string of that day,
      * i.e. MO, TU, WE, etc. */
    const char *dayFromNum(int day);
    /** the reverse of the above function. */
    int numFromDay(const QString &day);

    Attendee::PartStat readStatus(const char *s) const;
    QCString writeStatus(Attendee::PartStat status) const;

  private:
    Calendar *mCalendar;

    Event::List mEventsRelate;           // events with relations
    Todo::List mTodosRelate;             // todos with relations

    class Private;
    Private *d;
};

}

#endif
