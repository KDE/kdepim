/*
    Calendar access for KDE Alarm Daemon GUI.

    This file is part of the GUI interface for the KDE alarm daemon.
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef ADCALENDAR_GUI_H
#define ADCALENDAR_GUI_H

#include <kaboutdata.h>

#include "adcalendarbase.h"

// Alarm Daemon GUI calendar access
class ADCalendar : public ADCalendarBase
{
  public:
    ADCalendar(const QString& url, const QString& appname, Type);
    ~ADCalendar()  { }
    static ADCalendar *create(const QString& url, const QString& appname, Type);
    bool           enabled() const     { return enabled_; }
    bool           available() const   { return available_; }
    bool           loadFile()          { return loadFile_(kapp->aboutData()->programName()); }

    bool           available_;     // calendar is available for monitoring
    bool           enabled_;       // monitoring is currently manually enabled
};

typedef QPtrList<ADCalendar> CalendarList;

// The CalendarIteration class gives secure public access to AlarmGui::mCalendars
class ADCalendarIteration
{
  public:
    ADCalendarIteration(CalendarList& c)  : calendars(c) { calendar = calendars.first(); }
    bool           ok() const           { return !!calendar; }
    bool           next()               { return !!(calendar = calendars.next()); }
    bool           available() const    { return calendar->available(); }
    bool           enabled() const      { return calendar->enabled(); }
    void           enabled(bool tf)     { calendar->enabled_ = tf; }
    const QString& urlString() const    { return calendar->urlString(); }
  private:
    CalendarList&  calendars;
    ADCalendar*    calendar;
};

#endif
