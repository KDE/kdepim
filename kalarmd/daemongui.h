/*
    KDE Alarm Daemon GUI.

    This file is part of the GUI interface for the KDE alarm daemon.
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

#ifndef _ALARMDAEMONGUI_H
#define _ALARMDAEMONGUI_H

#include <qfont.h>
#include <qstrlist.h>

#include <libkcal/calendarlocal.h>

#include "DaemonGuiiface.h"
#include "calclient.h"

class AlarmDialog;
class AlarmDockWindow;

using namespace KCal;

#define DCOP_OBJECT_NAME   "dcop"
#define DAEMON_APP_NAME    "kalarmd"
#define DAEMON_DCOP_OBJECT "ad"


// Provides read-only access to the Alarm Daemon config files
class ADConfigData : public ADConfigDataBase
{
  public:
    ADConfigData()  : ADConfigDataBase(false) { }
    void   readDaemonData(bool& deletedClients, bool& deletedCalendars)
                                      { readConfigData(false, deletedClients, deletedCalendars); }
};

// The ClientIteration class gives secure public access to AlarmGui::mClients
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

// Alarm Daemon calendar access
class ADCalendar : public ADCalendarBase
{
  public:
    ADCalendar(const QString& url, const QString& appname, Type);
    ~ADCalendar()  { }
    virtual ADCalendarBase* create(const QString& url, const QString& appname, Type);
    bool           enabled() const     { return enabled_; }
    bool           available() const   { return available_; }
    bool           loadFile()          { return loadFile_(kapp->aboutData()->programName()); }

    bool           available_;     // calendar is available for monitoring
    bool           enabled_;       // monitoring is currently manually enabled
};

typedef QPtrList<ADCalendar> CalendarList;

// The CalendarIteration class gives secure public access to AlarmGui::mCalendars
class CalendarIteration
{
  public:
    CalendarIteration(CalendarList& c)  : calendars(c) { calendar = calendars.first(); }
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



class AlarmGui : public QObject, public ADConfigData, virtual public AlarmGuiIface
{
    Q_OBJECT
  public:
    explicit AlarmGui(QObject *parent = 0L, const char *name = 0L);
    virtual ~AlarmGui();

    static bool       isDaemonRunning() const;
    ClientIteration   getClientIteration()     { return ClientIteration(mClients); }
    CalendarIteration getCalendarIteration()   { return CalendarIteration(mCalendars); }
    int               calendarCount() const    { return mCalendars.count(); }
    bool              autostartDaemon() const  { return mAutostartDaemon; }
    const QString&    defaultClient() const    { return mDefaultClient; }
    void              setDefaultClient(int menuIndex);
    void              setAutostart(bool on);
    void              readDaemonConfig();

  public slots:
    void              suspend(int minutes);
    void              toggleGuiAutostart();
    void              toggleDaemonAutostart();
  private slots:
    void              showAlarmDialog();

  private:
    // DCOP interface
    void              alarmDaemonUpdate(const QString& change, const QString& calendarURL, const QString& appName);
    void              handleEvent(const QString& calendarURL, const QString& eventID);

  private:
    QString           checkDefaultClient();
    void              setToolTip();
    void              removeDialogEvents(const Calendar*);
    void              notifyDaemon(const QString& calendarURL, bool enable);

    CalendarList      mCalendars;           // the calendars being monitored
    AlarmDockWindow*  mDocker;              // the panel icon
    AlarmDialog*      mAlarmDialog;
    QTimer*           mSuspendTimer;
    QTimer*           mSessionStartTimer;   // timer waiting for session startup to complete
    QString           mDaemonDataFile;      // path of daemon's config file
    QString           mDefaultClient;       // daemon client application to call on left click
    bool              mAutostartDaemon;     // alarm daemon autostart status
    bool              mRevisingAlarmDialog; // true while mAlarmDialog is being revised
    bool              mDrawAlarmDialog;     // true to show mAlarmDialog after revision is complete
};

#endif
