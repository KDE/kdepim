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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef _ALARMDAEMONGUI_H
#define _ALARMDAEMONGUI_H

#include <qfont.h>
#include <qstrlist.h>
#include <qtimer.h>

#include <kaboutdata.h>

#include <libkcal/calendarlocal.h>

#include "alarmguiiface.h"
#include "clientinfo.h"
#include "adcalendar_gui.h"
#include "adconfigdatabase.h"

class AlarmDialog;
class AlarmDockWindow;

#define DCOP_OBJECT_NAME   "dcop"
#define DAEMON_APP_NAME    "kalarmd"
#define DAEMON_DCOP_OBJECT "ad"


// Provides read-only access to the Alarm Daemon config files
class ADConfigData : public ADConfigDataBase
{
  public:
    ADConfigData()  : ADConfigDataBase(false) { }
    void readDaemonData(bool& deletedClients, bool& deletedCalendars);
};


class AlarmGui : public QObject, public ADConfigData, virtual public AlarmGuiIface
{
    Q_OBJECT
  public:
    explicit AlarmGui(QObject *parent = 0L, const char *name = 0L);
    virtual ~AlarmGui();

    bool              isDaemonRunning(bool updateDockWindow = true);
    
    bool              autostartDaemon() const  { return mAutostartDaemon; }
    QCString          defaultClient() const    { return mDefaultClient; }
    void              setDefaultClient(int menuIndex);
    void              setFastDaemonCheck();
    void              readDaemonConfig();

  public slots:
    void              suspend(int minutes);
  private slots:
    void              showAlarmDialog();
    void              checkDaemonRunning();

  private:
    // DCOP interface
    void              alarmDaemonUpdate(int alarmGuiChangeType,
                                        const QString& calendarURL,
                                        const QCString& appName);
    void              handleEvent(const QString& calendarURL,
                                  const QString& eventID);
    void              handleEvent( const QString & ) {}

  private:
    void              checkDefaultClient();
    void              setToolTip();
    void              removeDialogEvents(const Calendar*);
    void              notifyDaemon(const QString& calendarURL, bool enable);
    void              registerWithDaemon();

    AlarmDockWindow*  mDocker;              // the panel icon
    AlarmDialog*      mAlarmDialog;
    QTimer            mSuspendTimer;
    QTimer*           mSessionStartTimer;   // timer waiting for session startup to complete
    QTimer            mDaemonStatusTimer;   // timer for checking daemon status
    int               mDaemonStatusTimerCount; // countdown for fast status checking
    QString           mDaemonDataFile;      // path of daemon's config file
    QCString          mDefaultClient;       // daemon client application to call on left click
    bool              mDaemonRunning;       // whether the alarm daemon is currently running
    bool              mAutostartDaemon;     // alarm daemon autostart status
    bool              mRevisingAlarmDialog; // true while mAlarmDialog is being revised
    bool              mDrawAlarmDialog;     // true to show mAlarmDialog after revision is complete
};

#endif
