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

#include <kapplication.h>
#include <kaboutdata.h>

#include "adcalendarbase.h"

// Alarm Daemon GUI calendar access
class ADCalendarGui : public ADCalendarBase
{
  public:
    ADCalendarGui(const QString& url, const QString& appname, Type);
    ~ADCalendarGui()  { }

    bool           loadFile()          { return loadFile_(kapp->aboutData()->programName()); }

    void setEnabled( bool e ) { mEnabled = e; }
    bool enabled() const { return mEnabled; }
    
    void setAvailable( bool a ) { mAvailable = a; }
    bool available() const { return mAvailable; }

    void setEventHandled(const Event*, const QValueList<QDateTime> &) {}
    bool eventHandled(const Event*, const QValueList<QDateTime> &) { return false; }

    void setEventPending(const QString&) {}
    bool getEventPending(QString&) { return false; }

  private:
    bool           mAvailable;     // calendar is available for monitoring
    bool           mEnabled;       // monitoring is currently manually enabled
};

typedef QPtrList<ADCalendarGui> CalendarGuiList;

/*
// The CalendarIteration class gives secure public access to AlarmGui::mCalendars
class ADCalendarIteration
{
  public:
    ADCalendarIteration(CalendarGuiList& c)  : calendars(c) { calendar = calendars.first(); }
    bool           ok() const           { return !!calendar; }
    bool           next()               { return !!(calendar = calendars.next()); }
    bool           available() const    { return calendar->available(); }
    bool           enabled() const      { return calendar->enabled(); }
    void           enabled(bool tf)     { calendar->enabled_ = tf; }
    const QString& urlString() const    { return calendar->urlString(); }
  private:
    CalendarGuiList&  calendars;
    ADCalendarGui*    calendar;
};
*/

class ADCalendarGuiFactory : public ADCalendarBaseFactory
{
  public:
    ADCalendarGui *create(const QString& url, const QString& appname,
                          ADCalendarBase::Type);
};


#endif
