/*
    KDE Alarm Daemon.

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

#ifndef _ALARMDAEMON_H
#define _ALARMDAEMON_H

#include <qfont.h>
#include <qstrlist.h>

#include <libkcal/calendarlocal.h>

#include "alarmdaemoniface.h"

using namespace KCal;


// Provides read-write access to the Alarm Daemon config files
class ADConfigDataRW : public ADConfigDataBase
{
  public:
    ADConfigDataRW()  : ADConfigDataBase(true) { }
    void           readDaemonData(bool sessionStarting);
    void           writeConfigClient(const QString& appName, const ClientInfo&);
    void           writeConfigClientGui(const QString& appName, const QString& dcopObject);
    void           addConfigClient(KSimpleConfig&, const QString& appName, const QString& key);
    void           writeConfigCalendar(const QString& appName, const ADCalendar*);

    typedef QMap<QString, QString> GuiMap;  // maps GUI client names against DCOP object names

    GuiMap         mGuis;                // client GUI application names and data

  private:
    virtual void   deleteConfigCalendar(const ADCalendar*);
};

// Alarm Daemon calendar access
class ADCalendar : public ADCalendarBase
{
  public:
    ADCalendar(const QString& url, const QString& appname, Type);
    ~ADCalendar()  { }
    virtual ADCalendarBase* create(const QString& url, const QString& appname, Type);
    bool           enabled() const     { return enabled_ && !unregistered; }
    bool           available() const   { return loaded_ && !unregistered; }
    static bool    eventHandled(const Event*);
    void           setEventHandled(const Event*);
    static void    clearEventsHandled(const QString& calendarURL);
    void           setEventPending(const QString& ID);
    bool           getEventPending(QString& ID);
    bool           loadFile()          { return loadFile_(QString()); }

    bool              enabled_;       // events are currently manually enabled
    bool              unregistered;   // client has registered since calendar was
                                      // constructed, but has not since added the
                                      // calendar. Monitoring is disabled.
  private:
    QPtrList<QString> eventsPending_; // IDs of pending KALARM type events
    struct EventItem
    {
      EventItem() : eventSequence(0) { }
      EventItem(const QString& url, int seqno)  : calendarURL(url), eventSequence(seqno) { }
      QString   calendarURL;
      int       eventSequence;
    };
    typedef QMap<QString, EventItem>  EventsMap;   // event ID, calendar URL/event sequence num
    static EventsMap  eventsHandled_; // IDs of displayed KALARM type events
};


class AlarmDaemon : public QObject, public ADConfigDataRW, virtual public AlarmDaemonIface
{
    Q_OBJECT
  public:
    AlarmDaemon(QObject *parent = 0L, const char *name = 0L);
    virtual ~AlarmDaemon();

    ClientIteration   getClientIteration()    { return ClientIteration(mClients); }
    CalendarIteration getCalendarIteration()  { return CalendarIteration(mCalendars); }
    int               calendarCount() const   { return mCalendars.count(); }
    int               calendarCount() const   { return mCalendars.count(); }

  public slots:
    void    suspend(int minutes);
    void    toggleAutostart();
  private slots:
    void    checkAlarmsSlot();
    void    checkIfSessionStarted();

  private:
    // DCOP interface
    void    enableAutoStart(bool enable);
    void    enableCal(const QString& urlString, bool enable)
                       { enableCal_(expandURL(urlString), enable); }
    void    reloadCal(const QString& appname, const QString& urlString)
                       { reloadCal_(appname, expandURL(urlString), false); }
    void    reloadMsgCal(const QString& appname, const QString& urlString)
                       { reloadCal_(appname, expandURL(urlString), true); }
    void    addCal(const QString& appname, const QString& urlString)
                       { addCal_(appname, expandURL(urlString), false); }
    void    addMsgCal(const QString& appname, const QString& urlString)
                       { addCal_(appname, expandURL(urlString), true); }
    void    removeCal(const QString& urlString)
                       { removeCal_(expandURL(urlString)); }
    void    resetMsgCal(const QString& appname, const QString& urlString)
                       { resetMsgCal_(appname, expandURL(urlString)); }
    void    registerApp(const QString& appName, const QString& appTitle,
                        const QString& dcopObject, bool commandLineNotify,
                        bool displayCalendarName);
    void    registerGui(const QString& appName, const QString& dcopObject);
    void    quit();

  private:

    enum GuiChangeType          // parameters to GUI client notification
    {
      CHANGE_STATUS,           // change of alarm daemon or calendar status
      CHANGE_CLIENT,           // change to client application list
      CHANGE_GUI,              // change to GUI client list
      ADD_CALENDAR,            // addition to calendar list (KOrganizer-type calendar)
      ADD_MSG_CALENDAR,        // addition to calendar list (KAlarm-type calendar)
      DELETE_CALENDAR,         // deletion from calendar list
      ENABLE_CALENDAR,         // calendar is now being monitored
      DISABLE_CALENDAR,        // calendar is available but not being monitored
      CALENDAR_UNAVAILABLE     // calendar is unavailable for monitoring
    };
    void        enableCal_(const QString& urlString, bool enable);
    void        addCal_(const QString& appname, const QString& urlString, bool msgCal);
    void        reloadCal_(const QString& appname, const QString& urlString, bool msgCal);
    void        reloadCal_(ADCalendar*);
    void        resetMsgCal_(const QString& appname, const QString& urlString);
    void        removeCal_(const QString& urlString);
    void        checkAlarms();
    bool        checkAlarms(ADCalendar*);
    void        checkAlarms(const QString& appName);
    bool        notifyEvent(const ADCalendar*, const QString& eventID);
    void        notifyGuiCalStatus(const ADCalendar*);
    void        notifyGui(GuiChangeType, const QString& calendarURL = QString::null);
    void        writeConfigClient(const QString& appName, const ClientInfo&);
    void        writeConfigClientGui(const QString& appName, const QString& dcopObject);
    void        addConfigClient(KSimpleConfig&, const QString& appName, const QString& key);
    void        writeConfigCalendar(const QString& appName, const ADCalendar*);
    void        deleteConfigCalendar(const ADCalendar*);
    bool        isSessionStarted();
    void        setTimerStatus();

    typedef QMap<QString, QString> GuiMap;  // maps GUI client names against DCOP object names

    GuiMap            mGuis;                // client GUI application names and data
    QTimer*           mAlarmTimer;
    QTimer*           mSessionStartTimer;   // timer waiting for session startup to complete
    QString           mClientDataFile;      // path of file containing client data
    bool              mEnabled;             // true if the alarm daemon is enabled
    bool              mAlarmTimerSyncing;   // true while alarm timer interval < 1 minute
    bool              mSessionStarted;      // true once session startup is complete
};

#endif
