/*
    This file is part of libkcal.

    Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>

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
    HtmlExport( Calendar *calendar );
    virtual ~HtmlExport() {}

    /**
      Writes out the calendar in HTML format.
    */
    bool save( const QString &fileName );

    /**
      Writes out calendar to text stream.
    */
    bool save( QTextStream * );

    void setTitle( QString title ) { mTitle = title; }
    QString title() { return mTitle; }

    void setTitleTodo( QString title ) { mTitleTodo = title; }
    QString titleTodo() { return mTitleTodo; }

    void setFullName( QString name ) { mName = name; }
    QString fullName() { return mName; }

    void setEmail( QString email ) { mEmail = email; }
    QString email() { return mEmail; }

    void setCredit( QString name, QString url ) { mCreditName = name; mCreditURL = url; }
    QString creditName() { return mCreditName; }
    QString creditURL() { return mCreditURL; }

    void setMonthViewEnabled(bool enable=true) { mMonthViewEnabled = enable; }
    bool monthViewEnabled() { return mMonthViewEnabled; }

    void setEventsEnabled(bool enable=true) { mEventsEnabled = enable; }
    bool eventsEnabled() { return mEventsEnabled; }

    void setTodosEnabled(bool enable=true) { mTodosEnabled = enable; }
    bool todosEnabled() { return mTodosEnabled; }

    void setCategoriesTodoEnabled(bool enable=true) { mCategoriesTodoEnabled = enable; }
    bool categoriesTodoEnabled() { return mCategoriesTodoEnabled; }

    void setAttendeesTodoEnabled(bool enable=true) { mAttendeesTodoEnabled = enable; }
    bool attendeesTodoEnabled() { return mAttendeesTodoEnabled; }

    void setExcludePrivateTodoEnabled(bool enable=true) { mExcludePrivateTodoEnabled = enable; }
    bool excludePrivateTodoEnabled() { return mExcludePrivateTodoEnabled; }

    void setExcludeConfidentialTodoEnabled(bool enable=true) { mExcludeConfidentialTodoEnabled = enable; }
    bool excludeConfidentialTodoEnabled() { return mExcludeConfidentialTodoEnabled; }

    void setCategoriesEventEnabled(bool enable=true) { mCategoriesEventEnabled = enable; }
    bool categoriesEventEnabled() { return mCategoriesEventEnabled; }

    void setAttendeesEventEnabled(bool enable=true) { mAttendeesEventEnabled = enable; }
    bool attendeesEventEnabled() { return mAttendeesEventEnabled; }

    void setExcludePrivateEventEnabled(bool enable=true) { mExcludePrivateEventEnabled = enable; }
    bool excludePrivateEventEnabled() { return mExcludePrivateEventEnabled; }

    void setExcludeConfidentialEventEnabled(bool enable=true) { mExcludeConfidentialEventEnabled = enable; }
    bool excludeConfidentialEventEnabled() { return mExcludeConfidentialEventEnabled; }

    void setDueDateEnabled(bool enable=true) { mDueDateEnabled = enable; }
    bool dueDateEnabled() { return mDueDateEnabled; }

    void setDateRange(const QDate &from,const QDate &to) { mFromDate = from, mToDate = to; }
    QDate fromDate() { return mFromDate; }
    QDate toDate() { return mToDate; }

    void setStyleSheet( const QString & );
    QString styleSheet();

    void addHoliday( QDate date, QString name );

  protected:
    void createHtmlMonthView (QTextStream *ts);
    void createHtmlEventList (QTextStream *ts);
    void createHtmlTodoList (QTextStream *ts);

    void createHtmlTodo (QTextStream *ts,Todo *todo);
    void createHtmlEvent (QTextStream *ts,Event *event,QDate date, bool withDescription = true);

    bool checkSecrecy( Incidence * );

    void formatHtmlCategories (QTextStream *ts,Incidence *event);
    void formatHtmlAttendees (QTextStream *ts,Incidence *event);

    QString breakString(const QString &text);

  private:
    QString cleanChars(const QString &txt);

    Calendar *mCalendar;

    bool mMonthViewEnabled;
    bool mEventsEnabled;
    bool mTodosEnabled;
    bool mCategoriesTodoEnabled;
    bool mAttendeesTodoEnabled;
    bool mCategoriesEventEnabled;
    bool mAttendeesEventEnabled;
    bool mDueDateEnabled;
    bool mExcludePrivateTodoEnabled;
    bool mExcludeConfidentialTodoEnabled;
    bool mExcludePrivateEventEnabled;
    bool mExcludeConfidentialEventEnabled;

    QDate mFromDate;
    QDate mToDate;

    QString mStyleSheet;
    QString mTitle;
    QString mTitleTodo;
    QString mName;
    QString mEmail;
    QString mCreditName;
    QString mCreditURL;
    QMap<QDate,QString> mHolidayMap;

    class Private;
    Private *d;
};

}

#endif
