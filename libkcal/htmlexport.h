/*
    This file is part of libkcal.

    Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KCAL_HTMLEXPORT_H
#define KCAL_HTMLEXPORT_H

#include <tqstring.h>
#include <tqdatetime.h>
#include <tqmap.h>

#include <libkcal/calendar.h>
#include <libkcal/htmlexportsettings.h>

#include "libkcal_export.h"

class TQFile;
class TQTextStream;

namespace KCal {

/**
  This class provides the functions to export a calendar as an HTML page.
*/
class KDE_EXPORT HtmlExport
{
  public:
    /**
      Create new HTML exporter for calendar.
    */
    HtmlExport( Calendar *calendar, HTMLExportSettings *settings );
    virtual ~HtmlExport() {}

    /**
      Writes out the calendar in HTML format.
    */
    bool save( const TQString &fileName = TQString::null );

    /**
      Writes out calendar to text stream.
    */
    bool save( TQTextStream * );

    void addHoliday( const TQDate &date, const TQString &name );

  protected:
    void createWeekView( TQTextStream *ts );
    void createMonthView( TQTextStream *ts );
    void createEventList( TQTextStream *ts );
    void createTodoList( TQTextStream *ts );
    void createJournalView( TQTextStream *ts );
    void createFreeBusyView( TQTextStream *ts );

    void createTodo( TQTextStream *ts, Todo *todo);
    void createEvent( TQTextStream *ts, Event *event, TQDate date,
                      bool withDescription = true);
    void createFooter( TQTextStream *ts );

    bool checkSecrecy( Incidence * );

    void formatLocation( TQTextStream *ts, Incidence *event );
    void formatCategories( TQTextStream *ts, Incidence *event );
    void formatAttendees( TQTextStream *ts, Incidence *event );

    TQString breakString( const TQString &text );

    TQDate fromDate() const;
    TQDate toDate() const;
    TQString styleSheet() const;

  private:
    TQString cleanChars( const TQString &txt );

    Calendar *mCalendar;
    HTMLExportSettings *mSettings;
    TQMap<TQDate,TQString> mHolidayMap;

    class Private;
    Private *d;
};

}

#endif
