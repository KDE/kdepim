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
#ifndef _DOCKWINDOW_H
#define _DOCKWINDOW_H
// $Id$

#include <ksystemtray.h>
#include <kpopupmenu.h>

#include "daemongui.h"

class AlarmDockWindow : public KSystemTray
{
    Q_OBJECT
  public:
    AlarmDockWindow(AlarmGui&, QWidget *parent = 0L, const char *name = 0L);
    virtual ~AlarmDockWindow();

    bool alarmsOn()        { return contextMenu()->isItemChecked(alarmsEnabledId); }
    bool autostartGuiOn()  { return contextMenu()->isItemChecked(autostartGuiId); }

    void setGuiAutostart(bool on);
    void setDaemonAutostart(bool on)   { contextMenu()->setItemChecked(autostartDaemonId, on); }
    void updateMenuClients();
    void updateMenuCalendars(bool recreate);
    void setDaemonStatus(bool running);
    void addToolTip(const QString&);

  protected:
    virtual void contextMenuAboutToShow(KPopupMenu*);
    void mousePressEvent(QMouseEvent*);
    void closeEvent(QCloseEvent*);

  public slots:
    void toggleAlarmsEnabled();
    void toggleGuiAutostart()     { setGuiAutostart(!autostartGuiOn()); }
    void toggleDaemonAutostart();
    void selectClient(int menuIndex);
    void selectCal(int menuIndex);

  protected:
    QPixmap    dPixmapEnabled, dPixmapDisabled;
    int        alarmsEnabledId;     // alarms enabled item in menu
    int        autostartGuiId;      // GUI autostart item in menu
    int        autostartDaemonId;   // daemon autostart item in menu

  private:
    // DCOP interface
    void       handleEvent(const QString& calendarURL, const QString& eventID);

    AlarmGui&  alarmGui;
    QString    defaultClient;    // default application name
    int        clientIndex;      // menu index to client names separator
    int        nClientIds;       // number of client names + 1 in menu
    int        nCalendarIds;     // number of calendar URLs + 1 in menu
};

#endif
