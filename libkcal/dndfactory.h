/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#ifndef KCAL_DNDFACTORY_H
#define KCAL_DNDFACTORY_H

#include "libkcal_export.h"

namespace KCal {

class ICalDrag;
class Event;
class Todo;
class Calendar;

/**
  This class implements functions to create Drag and Drop objects used for
  Drag-and-Drop and Copy-and-Paste.

  @short vCalendar/iCalendar Drag-and-Drop object factory.
*/
class LIBKCAL_EXPORT DndFactory
{
  public:
    DndFactory( Calendar * );

    /**
      Create a drag object.
    */
    ICalDrag *createDrag( Incidence *incidence, QWidget *owner );

    /** Create Todo object from drop event */
    Todo *createDropTodo(QDropEvent *de);
    /** Create Event object from drop event */
    Event *createDrop(QDropEvent *de);

    /** cut incidence to clipboard */
    void cutIncidence( Incidence * );
    /** copy the incidence to clipboard */
    bool copyIncidence( Incidence * );
    /** pastes the event or todo and returns a pointer to the new incidence pasted. */
    Incidence *pasteIncidence( const QDate &, const QTime *newTime = 0 );

  private:
    Calendar *mCalendar;

    class Private;
    Private *d;
};

}

#endif
