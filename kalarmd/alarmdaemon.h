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

#include <ksimpleconfig.h>

#include <libkcal/calendarlocal.h>

#include "alarmdaemoniface.h"
#include "calclient.h"
#include "adcalendar.h"
#include "adconfigdatarw.h"

class AlarmDaemon : public QObject, public ADConfigDataRW, virtual public AlarmDaemonIface
{
    Q_OBJECT
  public:
    AlarmDaemon(QObject *parent = 0L, const char *name = 0L);
    virtual ~AlarmDaemon();

  private slots:
    void    checkAlarmsSlot();
    void    checkIfSessionStarted();

    void    checkAlarms();

  private:
    // DCOP interface
    void    enableAutoStart(bool enable);
    void    enableCal(const QString& urlString, bool enable)
                       { enableCal_(expandURL(urlString), enable); }
    void    reloadCal(const QCString& appname, const QString& urlString)
                       { reloadCal_(appname, expandURL(urlString), false); }
    void    reloadMsgCal(const QCString& appname, const QString& urlString)
                       { reloadCal_(appname, expandURL(urlString), true); }
    void    addCal(const QCString& appname, const QString& urlString)
                       { addCal_(appname, expandURL(urlString), false); }
    void    addMsgCal(const QCString& appname, const QString& urlString)
                       { addCal_(appname, expandURL(urlString), true); }
    void    removeCal(const QString& urlString)
                       { removeCal_(expandURL(urlString)); }
    void    resetMsgCal(const QCString& appname, const QString& urlString)
                       { resetMsgCal_(appname, expandURL(urlString)); }
    void    registerApp(const QCString& appName, const QString& appTitle,
                        const QCString& dcopObject, int notificationType,
                        bool displayCalendarName);
    void    registerGui(const QCString& appName, const QCString& dcopObject);
    void    quit();
    void    forceAlarmCheck() { checkAlarms(); }
    void    dumpDebug();
    void    dumpAlarms();

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
    struct GuiInfo
    {
      GuiInfo()  { }
      explicit GuiInfo(const QCString &dcopObj) : dcopObject(dcopObj) { }
      QCString  dcopObject;     // DCOP object name
    };
    typedef QMap<QString, GuiInfo> GuiMap;  // maps GUI client names against their data

    void        enableCal_(const QString& urlString, bool enable);
    void        addCal_(const QCString& appname, const QString& urlString, bool msgCal);
    void        reloadCal_(const QCString& appname, const QString& urlString, bool msgCal);
    void        reloadCal_(ADCalendarBase*);
    void        resetMsgCal_(const QCString& appname, const QString& urlString);
    void        removeCal_(const QString& urlString);
    void        checkAlarms(ADCalendarBase*, const QDateTime &from, const QDateTime &to);
    void        checkAlarms(const QCString& appName);
    void        checkEventAlarms(const Event& event, QValueList<QDateTime>& alarmtimes);
    void        notifyPendingEvents(const QCString& appname);
    bool        notifyEvent(const ADCalendarBase*, const QString& eventID);
    void        notifyGuiCalStatus(const ADCalendarBase*);
    void        notifyGui(GuiChangeType, const QString& calendarURL = QString::null);
    void        notifyGui(GuiChangeType, const QString& calendarURL, const QCString &appname);
//    void        writeConfigClientGui(const QCString& appName, const QString& dcopObject);
    const GuiInfo* getGuiInfo(const QCString &appName) const;
    void        addConfigClient(KSimpleConfig&, const QCString& appName, const QString& key);
    bool        isSessionStarted();
    void        setTimerStatus();

    GuiMap            mGuis;                // client GUI application names and data
    QTimer*           mAlarmTimer;
    QTimer*           mSessionStartTimer;   // timer waiting for session startup to complete
    QString           mClientDataFile;      // path of file containing client data
    bool              mEnabled;             // true if the alarm daemon is enabled
    bool              mAlarmTimerSyncing;   // true while alarm timer interval < 1 minute
    bool              mSessionStarted;      // true once session startup is complete

    QDateTime mLastCheck;
};

#endif
