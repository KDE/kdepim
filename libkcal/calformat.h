/*
    This file is part of libkcal.

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
#ifndef KCAL_CALFORMAT_H
#define KCAL_CALFORMAT_H

#include <qstring.h>
#include <qdatetime.h>
#include <qevent.h>

#include "exceptions.h"
#include "event.h"
#include "libkcal_export.h"

namespace KCal {

class VCalDrag;
class Calendar;

/**
  This is the base class for calendar formats. It provides an interface for the
  generation/interpretation of a textual representation of a calendar.

  @short Class providing in interface to a calendar format
*/
class LIBKCAL_EXPORT CalFormat
{
  public:
    /** Constructs a new format. */
    CalFormat();
    /** Destruct calendar format. */
    virtual ~CalFormat();

    /**
      loads a calendar on disk into the calendar associated with this format.
      Returns TRUE if successful,else returns FALSE.
      @param fileName the name of the calendar on disk.
    */
    virtual bool load(Calendar *, const QString &fileName) = 0;
    /** writes out the calendar to disk. Returns true if
     * successful and false on error.
     * @param fileName the name of the file
     */
    virtual bool save(Calendar *, const QString &fileName) = 0;

    /**
      Parse string and populate calendar with that information.
    */
    virtual bool fromString(Calendar *, const QString & ) = 0;
    /**
      Return calendar information as string.
    */
    virtual QString toString(Calendar *) = 0;

    /** Clear exception status of this format object */
    void clearException();
    /**
      Return exception, if there is any, containing information about the last
      error that occurred.
    */
    ErrorFormat *exception();

    /** Set the application name for use in unique IDs and error messages,
     *  and product ID for incidence PRODID property
     */
    static void setApplication(const QString& app, const QString& productID);
    /** Return the application name used in unique IDs and error messages */
    static const QString& application()  { return mApplication; }
    /** Return the PRODID string to write into calendar files */
    static const QString& productId()  { return mProductId; }
    /** Return the KDE calendar format version indicated by a PRODID property */
    static int calendarVersion(const char* prodId);
    /** Return the PRODID string loaded from calendar file */
    const QString &loadedProductId()  { return mLoadedProductId; }

    /** Create a unique id string. */
    static QString createUniqueId();

    /**
      Set exception for this object. This is used by the functions of this
      class to report errors.
    */
    void setException(ErrorFormat *error);

  protected:
    QString mLoadedProductId;         // PRODID string loaded from calendar file

  private:
    ErrorFormat *mException;

    static QString mApplication;      // name of application for unique ID strings
    static QString mProductId;        // PRODID string to write to calendar files

    class Private;
    Private *d;
};

}

#endif
