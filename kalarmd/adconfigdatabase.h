/*
    Calendar and client access for KDE Alarm Daemon.

    This file is part of the KDE alarm daemon.
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>
    Based on the original, (c) 1998, 1999 Preston Brown

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

#ifndef ADCONFIGDATABASE_H
#define ADCONFIGDATABASE_H

#ifdef KALARMD
  #include "adcalendar.h"
#else
  #include "adcalendar_gui.h"
#endif
#include "calclient.h"

// Provides read-only access to the Alarm Daemon config data files
class ADConfigDataBase
{
  public:
    explicit ADConfigDataBase(bool daemon);
    virtual ~ADConfigDataBase() {}
    const ClientInfo* getClientInfo(const QString& appName) const;
    int               clientCount() const     { return mClients.count(); }

  protected:
    QString           readConfigData(bool sessionStarting, bool& deletedClients,
                                     bool& deletedCalendars);
    virtual void      deleteConfigCalendar(const ADCalendar*);
    ADCalendar*       getCalendar(const QString& calendarURL);
    static QString    expandURL(const QString& urlString);
    const QString&    clientDataFile() const  { return mClientDataFile; }

    static const QString CLIENT_KEY;
    static const QString CLIENTS_KEY;
    static const QString GUI_KEY;
    static const QString GUIS_KEY;
    static const QString CALENDAR_KEY;

    ClientMap         mClients;             // client application names and data
    CalendarList      mCalendars;           // the calendars being monitored
    bool              mIsAlarmDaemon;       // true if the instance is being used by the alarm daemon

  private:
    QString           mClientDataFile;      // path of file containing client data
};

#endif
