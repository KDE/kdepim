/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001,2002 Cornelius Schumacher <schumacher@kde.org>

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

namespace KCal {

class ICalDrag;
class Event;
class Todo;

/**
  This class implements functions to create Drag and Drop objects used for
  Drag-and-Drop and Copy-and-Paste.

  @short vCalendar/iCalendar Drag-and-Drop object factory.
*/
class DndFactory
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
  
    /** cut event to clipboard */
    void cutEvent(Event *);
    /** cut, copy, and paste operations follow. */
    bool copyEvent(Event *);
    /** pastes the event and returns a pointer to the new event pasted. */
    Event *pasteEvent(const QDate &, const QTime *newTime = 0);

  private:
    Calendar *mCalendar;
};

}

#endif
