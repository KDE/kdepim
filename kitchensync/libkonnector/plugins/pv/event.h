/***************************************************************************
                           event.h  -  description
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

#ifndef event_h
#define event_h

#include <qdom.h>
#include <qbuffer.h>

#include <libkcal/event.h>
#include <eventsyncee.h>

namespace PVHelper
{
  class Event
  {
    public:
      static KSync::EventSyncee* toEventSyncee(QDomNode& n);

      static QString toXML(KSync::EventSyncee* syncee);

    private:
      static void setRecurrence(KCal::Recurrence *rec, QDateTime startDate, int type);

      static QString getType(KCal::Recurrence *rec);
  };
}

#endif
