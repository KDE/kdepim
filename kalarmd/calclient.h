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

#ifndef _CALCLIENT_H
#define _CALCLIENT_H

#define QPtrList QList

#include <qstrlist.h>
#include <libkcal/calendarlocal.h>

using namespace KCal;


// Alarm Daemon client which receives calendar events
struct ClientInfo
{
    enum NotificationType       // how to notify client about events if client isn't running
    {
      NO_START_NOTIFY     = 0,  // don't start client if it isn't running
      DCOP_NOTIFY         = 1,  // start client and use DCOP to notify about events
      COMMAND_LINE_NOTIFY = 2   // start client and use command line arguments to notify about events
    };
    ClientInfo() { }
    ClientInfo(const QString& titl, const QString& dcopObj, int notifyType, bool disp, bool wait);
    void             setNotificationType(int type);
    QString          title;             // application title for display purposes
    QString          dcopObject;        // object to receive DCOP messages (if applicable)
    NotificationType notificationType;  // whether and how to notify events if client app isn't running
    bool             displayCalName;    // true to display calendar name in tooltip
    // Data which is not used by all alarm daemon applications
    bool     waitForRegistration; // don't notify any events until client has registered
    int      menuIndex;           // context menu index to this client's entry
};

typedef QMap<QString, ClientInfo> ClientMap;   // maps client names against client data

// Base class for Alarm Daemon calendar access
class ADCalendarBase : public CalendarLocal
{
  public:
    enum Type { KORGANISER = 0, KALARM = 1 };
    ADCalendarBase(const QString& url, const QString& appname, Type);
    ~ADCalendarBase()  { }
    virtual ADCalendarBase* create(const QString& url, const QString& appname, Type) = 0;
    const QString&  urlString() const   { return urlString_; }
    const QString&  appName() const     { return appName_; }
    bool            loaded() const      { return loaded_; }
    Type            actionType() const  { return actionType_; }
  protected:
    bool            loadFile_(const QString& appNamebool);

  private:
    ADCalendarBase(const ADCalendarBase&);             // prohibit copying
    ADCalendarBase& operator=(const ADCalendarBase&);  // prohibit copying

    QString           urlString_;     // calendar file URL
    QString           appName_;       // name of application owning this calendar
    Type              actionType_;    // action to take on event
    bool              loaded_;        // true if calendar file is currently loaded
};

typedef QPtrList<ADCalendarBase> CalendarList;
class ADCalendar;     // this class must be derived from ADCalendarBase


// Provides read-only access to the Alarm Daemon config data files
class ADConfigDataBase
{
  public:
    explicit ADConfigDataBase(bool daemon);
    const ClientInfo* getClientInfo(const QString& appName) const;
    int               clientCount() const     { return mClients.count(); }

  protected:
    QString           readConfigData(bool sessionStarting, bool& deletedClients, bool& deletedCalendars);
    virtual void      deleteConfigCalendar(const ADCalendar*) = 0;
    ADCalendarBase*   getCalendar(const QString& calendarURL);
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
