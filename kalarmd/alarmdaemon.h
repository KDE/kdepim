/*
    KDE Panel docking window for KDE Alarm Daemon.

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

class AlarmDialog;
class AlarmDockWindow;

using namespace KCal;


struct ClientInfo
{
    ClientInfo() { }
    ClientInfo(const QString& titl, const QString& dcopObj, bool cmdLine, bool disp, bool wait);
    QString  title;               // application title for display purposes
    QString  dcopObject;          // object to receive DCOP messages (if applicable)
    int      menuIndex;           // context menu index to this client's entry
    bool     commandLineNotify;   // notify events using command line if client app isn't running
    bool     displayCalName;      // true to display calendar name in tooltip
    bool     waitForRegistration; // don't notify any events until client has registered
};

typedef QMap<QString, ClientInfo> ClientMap;   // maps client names against client data

// The ClientIteration class gives secure public access to AlarmDaemon::mClients
class ClientIteration
{
  public:
    ClientIteration(ClientMap& c)     : clients(c) { iter = clients.begin(); }
    bool           ok() const         { return iter != clients.end(); }
    bool           next()             { return ++iter != clients.end(); }
    const QString& appName() const    { return iter.key(); }
    const QString& title() const      { return iter.data().title; }
    int            menuIndex() const  { return iter.data().menuIndex; }
    void           menuIndex(int n)   { iter.data().menuIndex = n; }
  private:
    ClientMap&          clients;
    ClientMap::Iterator iter;
};

// Class for Alarm Daemon calendar access
class ADCalendar : public CalendarLocal
{
  public:
    enum Type { KORGANISER = 0, KALARM = 1 };
    ADCalendar(const QString& url, const QString& appname, Type type, bool quiet = false);
    ~ADCalendar()  { }
    const QString& urlString() const   { return urlString_; }
    const QString& appName() const     { return appName_; }
    bool           loaded() const      { return loaded_; }
    bool           enabled() const     { return enabled_ && !unregistered; }
    bool           available() const   { return loaded_ && !unregistered; }
    Type           actionType() const  { return actionType_; }
    static bool    eventHandled(const Event*, const QValueList<QDateTime>& alarmtimes);
    void           setEventHandled(const Event*, const QValueList<QDateTime>& alarmtimes);
    static void    clearEventsHandled(const QString& calendarURL);
    void           setEventPending(const QString& ID);
    bool           getEventPending(QString& ID);
    bool           loadFile(bool quiet = false);

    bool             enabled_;       // events are currently manually enabled
    bool             unregistered;   // client has registered since calendar was
                                     // constructed, but has not since added the
                                     // calendar. Monitoring is disabled.
  private:
    ADCalendar(const ADCalendar&);             // prohibit copying
    ADCalendar& operator=(const ADCalendar&);  // prohibit copying

    QString          urlString_;     // calendar file URL
    QString          appName_;       // name of application owning this calendar
    QPtrList<QString>   eventsPending_; // IDs of pending KALARM type events
    Type             actionType_;    // action to take on event
    bool             loaded_;        // true if calendar file is currently loaded
    struct EventItem
    {
      EventItem() : eventSequence(0) { }
      EventItem(const QString& url, int seqno, const QValueList<QDateTime>& alarmtimes);
      QString               calendarURL;
      int                   eventSequence;
      QValueList<QDateTime> alarmTimes;
    };
    typedef QMap<QString, EventItem>  EventsMap;   // event ID, calendar URL/event sequence num
    static EventsMap  eventsHandled_; // IDs of displayed KALARM type events
};

typedef QPtrList<ADCalendar> CalendarList;

// The CalendarIteration class gives secure public access to AlarmDaemon::mCalendars
class CalendarIteration
{
  public:
    CalendarIteration(CalendarList& c)  : calendars(c) { calendar = calendars.first(); }
    bool           ok() const           { return !!calendar; }
    bool           next()               { return !!(calendar = calendars.next()); }
    bool           loaded() const       { return calendar->loaded(); }
    bool           available() const    { return calendar->available(); }
    bool           enabled() const      { return calendar->enabled(); }
    void           enabled(bool tf)     { calendar->enabled_ = tf; }
    const QString& urlString() const    { return calendar->urlString(); }
  private:
    CalendarList&  calendars;
    ADCalendar*    calendar;
};



class AlarmDaemon : public QObject, virtual public AlarmDaemonIface
{
    Q_OBJECT
  public:
    AlarmDaemon(QObject *parent = 0L, const char *name = 0L);
    virtual ~AlarmDaemon();

    const ClientInfo* getClientInfo(const QString& appName) const;
    ClientIteration   getClientIteration()    { return ClientIteration(mClients); }
    CalendarIteration getCalendarIteration()  { return CalendarIteration(mCalendars); }
    int               clientCount() const     { return mClients.count(); }
    void              setDefaultClient(int menuIndex);
    void              setAutostart(bool on);
    int               calendarCount() const   { return mCalendars.count(); }

  public slots:
    void    suspend(int minutes);
    void    toggleAutostart();
  private slots:
    void    checkAlarmsSlot();
    void    showAlarmDialog();
    void    checkIfSessionStarted();

  private:
    // DCOP interface
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
    void    quit();

  private:
    void        addCal_(const QString& appname, const QString& urlString, bool msgCal);
    void        reloadCal_(const QString& appname, const QString& urlString, bool msgCal);
    void        reloadCal_(ADCalendar*);
    void        resetMsgCal_(const QString& appname, const QString& urlString);
    void        removeCal_(const QString& urlString);
    void        checkAlarms();
    bool        checkAlarms(ADCalendar*, bool showDialog);
    void        checkAlarms(const QString& appName);
    void        checkEventAlarms(const Event&, QValueList<QDateTime>& alarmtimes);
    bool        notifyEvent(const ADCalendar*, const QString& eventID);
    void        notifyPendingEvents(const QString& appname);
    QString     readConfig();
    void        writeConfigClient(const QString& appName, const ClientInfo&);
    void        writeConfigCalendar(const QString& appName, const ADCalendar*);
    void        deleteConfigCalendar(const ADCalendar*);
    QString     checkDefaultClient();
    ADCalendar* getCalendar(const QString& calendarURL);
    void        setToolTipStartTimer();
    void        removeDialogEvents(const Calendar*);
    static QString expandURL(const QString& urlString);

    AlarmDockWindow*  mDocker;              // the panel icon
    ClientMap         mClients;             // client application names and data
    CalendarList      mCalendars;           // the calendars being monitored
    AlarmDialog*      mAlarmDialog;
    QTimer*           mAlarmTimer;
    QTimer*           mSuspendTimer;
    QTimer*           mSessionStartTimer;   // timer waiting for session startup to complete
    QString           mClientDataFile;      // path of file containing client data
    bool              mAlarmTimerSyncing;   // true while alarm timer interval < 1 minute
    bool              mRevisingAlarmDialog; // true while mAlarmDialog is being revised
    bool              mDrawAlarmDialog;     // true to show mAlarmDialog after revision is complete
};

#endif
