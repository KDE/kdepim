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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_HTMLEXPORT_H
#define KCAL_HTMLEXPORT_H

#include <qstring.h>
#include <qdatetime.h>
#include <qmap.h>

#include <libkcal/calendar.h>
#include <libkcal/htmlexportsettings.h>

class QFile;
class QTextStream;

namespace KCal {

/**
  This class provides the functions to export a calendar as a HTML page.
*/
class HtmlExport
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
    bool save( const QString &fileName = QString::null );

    /**
      Writes out calendar to text stream.
    */
    bool save( QTextStream * );

    void addHoliday( const QDate &date, const QString &name );

  protected:
    void createWeekView( QTextStream *ts );
    void createMonthView( QTextStream *ts );
    void createEventList( QTextStream *ts );
    void createTodoList( QTextStream *ts );
    void createJournalView( QTextStream *ts );
    void createFreeBusyView( QTextStream *ts );

    void createTodo( QTextStream *ts, Todo *todo);
    void createEvent( QTextStream *ts, Event *event, QDate date, 
                      bool withDescription = true);
    void createFooter( QTextStream *ts );

    bool checkSecrecy( Incidence * );

    void formatCategories( QTextStream *ts, Incidence *event );
    void formatAttendees( QTextStream *ts, Incidence *event );
 
    QString breakString( const QString &text );
    
    QDate fromDate() const;
    QDate toDate() const;
    QString styleSheet() const;

  private:
    QString cleanChars( const QString &txt );

    Calendar *mCalendar;
    HTMLExportSettings *mSettings;
    QMap<QDate,QString> mHolidayMap;

    class Private;
    Private *d;
};

}

#endif
