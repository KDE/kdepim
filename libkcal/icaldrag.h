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

#ifndef KCAL_ICALDRAG_H
#define KCAL_ICALDRAG_H

#include <qdragobject.h>
#include "calendar.h"
#include "libkcal_export.h"

namespace KCal {

/**
  iCalendar drag&drop class.
*/
class LIBKCAL_EXPORT ICalDrag : public QStoredDrag
{
  public:
    /**
      Create a drag&drop object for iCalendar component \a ical.
    */
    ICalDrag( Calendar *cal, QWidget *parent = 0, const char *name = 0 );
    ~ICalDrag() {};

    /**
      Return, if drag&drop object can be decode to iCalendar.
    */
    static bool canDecode( QMimeSource * );
    /**
      Decode drag&drop object to iCalendar component \a cal.
    */
    static bool decode( QMimeSource *e, Calendar *cal );

  private:
    class Private;
    Private *d;
};

}

#endif
