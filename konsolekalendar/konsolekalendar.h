/***************************************************************************
        konsolekalendar.h  -  description
           -------------------
    begin                : Sun Jan  6 11:50:14 EET 2002
    copyright            : (C) 2002 by Tuukka Pasanen
    email                : illuusio@mailcity.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KONSOLEKALENDAR_H
#define KONSOLEKALENDAR_H

#include <kapplication.h>
#include <qdatetime.h>
#include "konsolekalendar.h"
#include "kalendarVariables.h"
#include "calendarlocal.h"
#include "calendar.h"
#include "event.h"

namespace KCal {

/** KonsoleKalendar is the base class of the project */
class KonsoleKalendar
{
  public:
    KonsoleKalendar(KalendarVariables &variables);
    ~KonsoleKalendar();

    void showInstance();

  private:
    void showNext();
    void showDate( QDate date );
    void printEventTime(Event *event);
    bool isHappened(Event *event);
      
    KalendarVariables m_variables;
    Calendar *m_Calendar;
};

}
#endif
