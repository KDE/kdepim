/*
    This file is part of libkcal.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef _CALFORMAT_H
#define _CALFORMAT_H
// $Id$

#include <qstring.h>
#include <qdatetime.h>
#include <qevent.h>

#include "exceptions.h"
#include "event.h"

namespace KCal {

class VCalDrag;
class Calendar;

/**
  This is the base class for calendar formats. It provides an interface for the
  generation/interpretation of a textual representation of a calendar.

  @short Class providing in interface to a calendar format
*/
class CalFormat {
  public:
    /** Constructs a new format for the calendar given as argument. */
    CalFormat(Calendar *);
    /** Destruct calendar format. */
    virtual ~CalFormat();

    /** Associate a widget with this format */
    void setTopwidget(QWidget *topWidget);
    
    /**
      loads a calendar on disk into the calendar associated with this format.
      Returns TRUE if successful,else returns FALSE.
      @param fileName the name of the calendar on disk.
    */
    virtual bool load(const QString &fileName) = 0;
    /** writes out the calendar to disk. Returns true if
     * successful and false on error.
     * @param fileName the name of the file
     */
    virtual bool save(const QString &fileName) = 0;
  
    /**
      Parse string and populate calendar with that information.
    */
    virtual bool fromString( const QString & ) = 0;  
    /**
      Return calendar information as string.
    */
    virtual QString toString() = 0;
  
    /** /deprecated */
    void showDialogs(bool);

    /** Clear exception status of this format object */
    void clearException();
    /**
      Return exception, if there is any, containing information about the last
      error that occured.
    */
    ErrorFormat *exception();

    /** Set the application name for use in unique IDs and error messages,
     *  and product ID for incidence PRODID property
     */
    static void setApplication(const QString& app, const QString& productID);
    /** Return the application name used in unique IDs and error messages */
    static const QString& application()  { return mApplication; }
    /** Return the PRODID string for calendar files */
    static const QString& productId()  { return mProductId; }

    /** Create a unique id string. */
    static QString createUniqueId();
  
    /**
      Set exception for this object. This is used by the functions of this
      class to report errors.
    */
    void setException(ErrorFormat *error);
  
  protected:  
    QWidget *mTopWidget;      // topWidget used for message boxes
    bool mEnableDialogs;      // display various GUI dialogs?

    Calendar *mCalendar;
  
  private:
    QPtrList<Event> mEventsRelate;           // events with relations
    QPtrList<Event> mTodosRelate;            // todos with relations
    
    ErrorFormat *mException;

    static QString mApplication;   // name of application for unique ID strings
    static QString mProductId;     // PRODID string for calendar file
};

}

#endif
