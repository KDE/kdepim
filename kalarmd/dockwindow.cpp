/*
    KDE Panel docking window for KDE Alarm Daemon GUI.

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

#include <stdlib.h>

#include <qtooltip.h>

#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <dcopclient.h>

#include "daemongui.h"
#include "dockwindow.h"
#include "dockwindow.moc"

AlarmDockWindow::AlarmDockWindow(AlarmGui& ad, QWidget *parent, const char *name)
  : KSystemTray(parent, name),
    alarmDaemon(ad),
    nClientIds(0L),
    nCalendarIds(0L)
{
  // Set up GUI icons
  KGlobal::iconLoader()->addAppDir(kapp->aboutData()->appName());
  dPixmap1 = BarIcon("kalarmdgui");
  dPixmap2 = BarIcon("kalarmdgui_disabled");

  if (dPixmap1.isNull() || dPixmap2.isNull())
    KMessageBox::sorry(this, i18n("Can't load docking tray icon!"),
                             i18n("%1 Error").arg(kapp->aboutData()->programName()));
  setPixmap(dPixmap1);

  // Read the GUI autostart status from the config file
  KConfig* config = kapp->config();
  config->setGroup("General");
  bool autostartGui = config->readBoolEntry("Autostart", true);

  // Set up the context menu
  alarmsEnabledId = contextMenu()->insertItem(i18n("Alarms Enabled"),
                                              this, SLOT(toggleAlarmsEnabled()));
  autostartDaemonId = contextMenu()->insertItem(i18n("Start alarm daemon automatically at login"),
                                                this, SLOT(toggleDaemonAutostart()));
  autostartGuiId = contextMenu()->insertItem(i18n("Display this tray icon at login"),
                                                this, SLOT(toggleGuiAutostart()));
  contextMenu()->setItemChecked(alarmsEnabledId, true);
  contextMenu()->setItemChecked(autostartGuiId, autostartGui);
  contextMenu()->setItemChecked(autostartDaemonId, alarmDaemon.autostartDaemon());
  clientIndex = contextMenu()->count();
  updateMenuClients();
  updateMenuCalendars(true);

  addToolTip(QString());
}

AlarmDockWindow::~AlarmDockWindow()
{
}

/*
 * Update the context menu to display the current client applications
 * (if more than one)
 */
void AlarmDockWindow::updateMenuClients()
{
  KPopupMenu* menu = contextMenu();
  if (nClientIds)
  {
    // Client applications are already on the menu, so delete them
    for ( ;  nClientIds > 0;  --nClientIds)
      menu->removeItemAt(clientIndex);
  }
  if (alarmDaemon.clientCount() > 1)
  {
    // More than one client is installed, so provide a choice
    // as to which program to activate when the panel icon is clicked.
    int index = clientIndex;
    menu->insertSeparator(index++);
    for (ClientIteration client = alarmDaemon.getClientIteration();  client.ok();  client.next())
    {
      int id = menu->insertItem(i18n("Click starts %1").arg(client.title()),
                                this, SLOT(selectClient(int)), 0, -1, index);
      menu->setItemParameter(id, index);    // set parameter for selectClient()
      menu->setItemChecked(id, (client.appName() == alarmDaemon.defaultClient()));
      client.menuIndex(index++);
    }
    nClientIds = index - clientIndex;
  }
}

/*
 * Update the context menu to display the current calendar URLs.
 * If 'recreate' is true, the calendar menu items are recreated from scratch.
 * If 'recreate' is false, the existing menu items' statuses are simply updated.
 */
void AlarmDockWindow::updateMenuCalendars(bool recreate)
{
  bool enable = AlarmGui::isDaemonRunning();
  KPopupMenu* menu = contextMenu();
  int calendarIndex = clientIndex + nClientIds;   // index to separator before calendars
  if (recreate)
  {
    // Recreate the calendar menu items
    if (nCalendarIds)
    {
      // Client applications are already on the menu, so delete them
      for ( ;  nCalendarIds > 0;  --nCalendarIds)
        menu->removeItemAt(calendarIndex);
    }
    if (alarmDaemon.calendarCount() > 0)
    {
      int index = calendarIndex;
      menu->insertSeparator(index++);
      for (CalendarIteration cal = alarmDaemon.getCalendarIteration();  cal.ok();  cal.next())
      {
        int id = menu->insertItem(KURL(cal.urlString()).prettyURL(), this,
                                       SLOT(selectCal(int)), 0, -1, index);
        menu->setItemParameter(id, index++);    // set parameter for selectCal()
        menu->setItemEnabled(id, enable && cal.available());
        menu->setItemChecked(id, cal.enabled());
      }
      nCalendarIds = index - calendarIndex;
    }
  }
  else
  {
    // Update the state of the existing menu items
    int index = calendarIndex;
    for (CalendarIteration cal = alarmDaemon.getCalendarIteration();  cal.ok();  cal.next())
    {
      int id = menu->idAt(++index);
      menu->setItemEnabled(id, enable && cal.available());
      menu->setItemChecked(id, cal.enabled());
    }
  }
}

/*
 * Called when the Alarms Enabled context menu item is selected.
 * The alarm daemon is stopped or started as appropriate.
 */
void AlarmDockWindow::toggleAlarmsEnabled()
{
  bool newstate = !contextMenu()->isItemChecked(alarmsEnabledId);
  if (newstate)
  {
    // Start monitoring alarms - start the alarm daemon
    if (!AlarmGui::isDaemonRunning())
    {
      // The daemon is not running
      QString execStr = locate("exe", DAEMON_APP_NAME);
      if (execStr.isEmpty())
      {
        KMessageBox::error(this, i18n("Alarm Daemon not found"), i18n("%1 Error").arg(kapp->aboutData()->programName()));
        kdError() << "AlarmDaemon::notifyEvent(): kalarmd not found" << endl;
        return;
      }
      system(execStr.latin1());
    }
  }
  else
  {
    // Stop monitoring alarms - tell the alarm daemon to quit
    QByteArray data;
    QDataStream arg(data, IO_WriteOnly);
    //call()?
    if (!kapp->dcopClient()->send(DAEMON_APP_NAME, DAEMON_DCOP_OBJECT, "quit()", data))
      kdDebug() << "AlarmDockWindow::toggleAlarmsEnabled(): quit dcop send failed\n";
  }
  setPixmap(newstate ? dPixmap1 : dPixmap2);
  contextMenu()->setItemChecked(alarmsEnabledId, AlarmGui::isDaemonRunning());
  contextMenuAboutToShow(contextMenu());
}

/*
 * Called when a client application is selected in the context menu.
 * The config file is updated with the new default client.
 */
void AlarmDockWindow::selectClient(int menuIndex)
{
  KPopupMenu* menu = contextMenu();
  kdDebug() << "AlarmDockWindow::selectClient(): " << menuIndex << endl;
  if (!menu->isItemChecked(menu->idAt(menuIndex)))
  {
    for (int i = clientIndex;  i < clientIndex + nClientIds;  ++i)
      menu->setItemChecked(menu->idAt(i), (i == menuIndex));

    alarmDaemon.setDefaultClient(menuIndex);
  }
}

/*
 * Called when a calendar is selected in the context menu.
 * The calendar's new enable state is sent to the daemon via DCOP.
 * The menu item's status is changed only when the daemon notifies
 * its new status.
 */
void AlarmDockWindow::selectCal(int menuIndex)
{
  KPopupMenu* menu = contextMenu();
  int id = menu->idAt(menuIndex);
  bool newstate = !menu->isItemChecked(id);
  kdDebug() << "AlarmDockWindow::selectCal(): "<< menuIndex << ": id=" << id << " newstate=" << (int)newstate << endl;
  int index = clientIndex + nClientIds;
  for (CalendarIteration cal = alarmDaemon.getCalendarIteration();  cal.ok();  cal.next())
  {
    if (++index == menuIndex)
    {
      QByteArray data;
      QDataStream arg(data, IO_WriteOnly);
      arg << cal.urlString() << (Q_INT8)newstate;
      if (!kapp->dcopClient()->send(DAEMON_APP_NAME, DAEMON_DCOP_OBJECT, "enableCal(QString,bool)", data))
        kdDebug() << "AlarmDockWindow::selectCal(): enableCal dcop send failed\n";
      break;
    }
  }
}

/*
 * Set GUI autostart at login on or off, and set the context menu accordingly.
 */
void AlarmDockWindow::setGuiAutostart(bool on)
{
  KConfig* config = kapp->config();
  config->setGroup("General");
  config->writeEntry("Autostart", on);
  config->sync();
  contextMenu()->setItemChecked(autostartGuiId, on);
}

/*
 * Called when the daemon autostart menu option is selected, to toggle
 * the daemon's autostart state. This is done by sending a DCOP message
 * to the daemon. The menu item state is changed only when the daemon
 * notifies its new autostart status.
 */
void AlarmDockWindow::toggleDaemonAutostart()
{
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);
  arg << (Q_INT8)!alarmDaemon.autostartDaemon();
  if (!kapp->dcopClient()->send(DAEMON_APP_NAME, DAEMON_DCOP_OBJECT, "enableAutoStart(bool)", data))
    kdDebug() << "AlarmDockWindow::toggleDaemonAutostart(): enableAutoStart dcop send failed\n";
  contextMenuAboutToShow(contextMenu());
}

/*
 * Called when the context menu is about to be displayed.
 * Enable or disable items which cause DCOP messages to be sent to the
 * alarm daemon, depending on whether the daemon is currently running.
 */
void AlarmDockWindow::contextMenuAboutToShow(KPopupMenu* menu)
{
  menu->setItemEnabled(autostartDaemonId, AlarmGui::isDaemonRunning());
  updateMenuCalendars(false);
}

/*
 * Called when the mouse is clicked over the panel icon.
 */
void AlarmDockWindow::mousePressEvent(QMouseEvent* e)
{
  if (e->button() == LeftButton)
  {
    // Left click: start up the default client application
    KProcess proc;
    proc << alarmDaemon.defaultClient();
    proc.start(KProcess::DontCare);
  }
  else
    KSystemTray::mousePressEvent(e);
}

void AlarmDockWindow::closeEvent(QCloseEvent*)
{
  kapp->quit();
}

void AlarmDockWindow::addToolTip(const QString& filename)
{
  QString apps;
  for (ClientIteration client = alarmDaemon.getClientIteration();  client.ok();  client.next())
  {
    if (!apps.isEmpty())
      apps += '/';
    apps += client.title();
  }
  apps += i18n(" Alarm Monitor");

  if (!filename.isEmpty())
    apps += "\n" + filename;

  QToolTip::add(this, apps);
}
