/*
    KDE Panel docking window for KDE Alarm Daemon GUI.

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
#ifndef ALARMDOCKWINDOW_H
#define ALARMDOCKWINDOW_H
// $Id$

#include <ksystemtray.h>
#include <kpopupmenu.h>

#include "koalarmclient.h"

class AlarmDockWindow : public KSystemTray
{
    Q_OBJECT
  public:
    AlarmDockWindow(KOAlarmClient *, QWidget *parent = 0L, const char *name = 0L);
    virtual ~AlarmDockWindow();

    bool alarmsOn()        { return contextMenu()->isItemChecked(mAlarmsEnabledId); }
    bool autostartGuiOn()  { return contextMenu()->isItemChecked(mAutostartGuiId); }

    void setGuiAutostart(bool on);

  protected:
    void mousePressEvent(QMouseEvent*);
    void closeEvent(QCloseEvent*);

  public slots:
    void toggleAlarmsEnabled();
    void toggleGuiAutostart()     { setGuiAutostart(!autostartGuiOn()); }
  
  protected slots:
    void configureAlarmDaemon();

  protected:
    QPixmap    mPixmapEnabled, mPixmapDisabled;
    int        mAlarmsEnabledId;     // alarms enabled item in menu
    int        mAutostartGuiId;      // GUI autostart item in menu

  private:
    // DCOP interface
    void       handleEvent(const QString& calendarURL, const QString& eventID);

    KOAlarmClient   *mAlarmGui;
};

#endif
