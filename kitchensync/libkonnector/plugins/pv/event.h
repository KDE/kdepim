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

/** This is a class to handle conversions of events. The events
  * can be converted from a QDomNode to a EventSyncee* and vice versa.
  * @author Maurus Erni
  */

namespace PVHelper
{
  class Event
  {
    public:
      /**
        * Converts a QDomNode to an EventSyncee*.
        * @param node The node (part of an XML document) to be converted
        * @return KSync::EventSyncee* The converted events
        */
      static KSync::EventSyncee* toEventSyncee(QDomNode& n);

      /**
        * Converts an EventSyncee* to a QString which represents a
        * DOM node.
        * @param syncee The syncee to be converted
        * @return QString The converted events as an XML string
        */
      static QString toXML(KSync::EventSyncee* syncee);

    private:
      /**
        * Handles the recurrence of an event. The recurrence will be set
        * depending on the start date and the type (daily, weekly, ...)
        * @param rec The recurrence pointer. Will be modified inside the method
        * @param startDate The start date of the event
        * @param type The type of the recurrence used in the PV
        */
      static void setRecurrence(KCal::Recurrence *rec, QDateTime startDate, int type);

      /**
        * Gets the type of the recurrence of an event.
        * @param rec The recurrence pointer of the event
        * @return QString The type of the recurrence used in PV
        */
      static QString getType(KCal::Recurrence *rec);
  };
}

#endif
