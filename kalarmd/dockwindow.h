/*
    This file is part of the KDE alarm daemon.
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

#include "alarmdaemon.h"

class AlarmDockWindow : public KSystemTray
{
    Q_OBJECT
  public:
    AlarmDockWindow(AlarmDaemon&, const QString& defaultClient,
                    QWidget *parent = 0L, const char *name = 0L);
    virtual ~AlarmDockWindow();

    bool alarmsOn()     { return contextMenu()->isItemChecked(alarmsEnabledId); }
    bool autostartOn()  { return contextMenu()->isItemChecked(autostartId); }

    void setAutostart(bool on)   { contextMenu()->setItemChecked(autostartId, on); }
    void updateMenuClients();
    void updateMenuCalendars(bool recreate);
    void addToolTip(const QString&);

  protected:
    void mousePressEvent(QMouseEvent*);
    void closeEvent(QCloseEvent*);

  public slots:
    void toggleAlarmsEnabled()
    {
      contextMenu()->setItemChecked(alarmsEnabledId,
              !contextMenu()->isItemChecked(alarmsEnabledId));
      setPixmap(contextMenu()->isItemChecked(alarmsEnabledId) ? dPixmap1 : dPixmap2);
    }
    void select(int menuIndex);
    void selectCal(int menuIndex);

  protected:
    QPixmap       dPixmap1, dPixmap2;
    int           alarmsEnabledId;  // alarms enabled item in menu
    int           autostartId;      // autostart item in menu

  private:
    AlarmDaemon&  alarmDaemon;
    QString       defaultClient;    // default application name
    int           clientIndex;      // menu index to client names separator
    int           nClientIds;       // number of client names + 1 in menu
    int           nCalendarIds;     // number of calendar URLs + 1 in menu
};

#endif
