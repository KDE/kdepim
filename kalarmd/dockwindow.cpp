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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <stdlib.h>

#include <qtooltip.h>
#include <qfile.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <dcopclient.h>

#include "alarmgui.h"
#include "alarmdaemoniface_stub.h"

#include "dockwindow.h"
#include "dockwindow.moc"


AlarmDockWindow::AlarmDockWindow(AlarmGui *ag, QWidget *parent, const char *name)
  : KSystemTray(parent, name),
    mAlarmGui(ag),
    mNumClientIds(0L),
    mNumCalendarIds(0L),
    mSettingDaemonStatus(false)
{
  // Set up GUI icons
  KGlobal::iconLoader()->addAppDir(kapp->aboutData()->appName());
  kdDebug() << "kalarmd, icon dir = " << kapp->aboutData()->appName() << endl;
  mPixmapEnabled  = loadIcon( "kalarmdgui" );
  mPixmapDisabled = loadIcon( "kalarmdgui_disabled" );

  if (mPixmapEnabled.isNull() || mPixmapDisabled.isNull())
    KMessageBox::sorry(this, i18n("Can't load docking tray icon!"),
                             i18n("%1 Error").arg(kapp->aboutData()->programName()));

  // Read the GUI autostart status from the config file
  KConfig* config = kapp->config();
  config->setGroup("General");
  bool mAutostartGui = config->readBoolEntry("Autostart", true);

  // Set up the context menu
  mAlarmsEnabledId = contextMenu()->insertItem(i18n("Alarms Enabled"),
                                              this, SLOT(toggleAlarmsEnabled()));
  mAutostartDaemonId = contextMenu()->insertItem(i18n("Start Alarm Daemon Automatically at Login"),
                                                this, SLOT(toggleDaemonAutostart()));
  mAutostartGuiId = contextMenu()->insertItem(i18n("Display this Tray Icon at Login"),
                                                this, SLOT(toggleGuiAutostart()));
  contextMenu()->setItemChecked(mAutostartDaemonId, mAlarmGui->autostartDaemon());
  contextMenu()->setItemChecked(mAutostartGuiId, mAutostartGui);
  setPixmap(mPixmapEnabled);
  contextMenu()->setItemChecked(mAlarmsEnabledId, true);

  mClientIndex = contextMenu()->count();
  updateMenuClients();
  updateMenuCalendars(true);

  setDaemonStatus(mAlarmGui->isDaemonRunning(false));

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
  if (mNumClientIds)
  {
    // Client applications are already on the menu, so delete them
    for ( ;  mNumClientIds > 0;  --mNumClientIds)
      menu->removeItemAt(mClientIndex);
  }
  if (mAlarmGui->clientCount() > 1)
  {
    // More than one client is installed, so provide a choice
    // as to which program to activate when the panel icon is clicked.
    int index = mClientIndex;
    menu->insertSeparator(index++);
    ClientList clients = mAlarmGui->clients();
    ClientList::Iterator client;
    for ( client = clients.begin();  client != clients.end();  ++client )
    {
      int id = menu->insertItem(i18n("Click Starts %1").arg((*client).title),
                                this, SLOT(selectClient(int)), 0, -1, index);
      menu->setItemParameter(id, index);    // set parameter for selectClient()
      menu->setItemChecked(id, ((*client).appName == mAlarmGui->defaultClient()));
      (*client).menuIndex = index++;
    }
    mNumClientIds = index - mClientIndex;
  }
}

/*
 * Update the context menu to display the current calendar URLs.
 * If 'recreate' is true, the calendar menu items are recreated from scratch.
 * If 'recreate' is false, the existing menu items' statuses are simply updated.
 */
void AlarmDockWindow::updateMenuCalendars(bool recreate)
{
  bool enable = mAlarmGui->isDaemonRunning(false);
  KPopupMenu* menu = contextMenu();
  int calendarIndex = mClientIndex + mNumClientIds;   // index to separator before calendars
  if (recreate)
  {
    // Recreate the calendar menu items
    if (mNumCalendarIds)
    {
      // Client applications are already on the menu, so delete them
      for ( ;  mNumCalendarIds > 0;  --mNumCalendarIds)
        menu->removeItemAt(calendarIndex);
    }
    if (mAlarmGui->calendarCount() > 0)
    {
      int index = calendarIndex;
      menu->insertSeparator(index++);
      CalendarList calendars = mAlarmGui->calendars();
      ADCalendarBase *cal;
      for( cal = calendars.first(); cal; cal = calendars.next() )
      {
        int id = menu->insertItem(KURL(cal->urlString()).prettyURL(), this,
                                  SLOT(selectCal(int)), 0, -1, index);
        menu->setItemParameter(id, index++);    // set parameter for selectCal()
        menu->setItemEnabled(id, enable && cal->available());
        menu->setItemChecked(id, cal->enabled());
      }
      mNumCalendarIds = index - calendarIndex;
    }
  }
  else
  {
    // Update the state of the existing menu items
    int index = calendarIndex;
    CalendarList calendars = mAlarmGui->calendars();
    ADCalendarBase *cal;
    for( cal = calendars.first(); cal; cal = calendars.next() )
    {
      int id = menu->idAt(++index);
      menu->setItemEnabled(id, enable && cal->available());
      menu->setItemChecked(id, cal->enabled());
    }
  }
}

/*
 * Called when the Alarms Enabled context menu item is selected.
 * The alarm daemon is stopped or started as appropriate.
 */
void AlarmDockWindow::toggleAlarmsEnabled()
{
  bool newstate = !contextMenu()->isItemChecked(mAlarmsEnabledId);
  if (newstate)
  {
    // Start monitoring alarms - start the alarm daemon
    if (!mAlarmGui->isDaemonRunning())
    {
      // The daemon is not running
      QString execStr = locate("exe", DAEMON_APP_NAME);
      if (execStr.isEmpty())
      {
        KMessageBox::error(this, i18n("Alarm Daemon not found"), i18n("%1 Error").arg(kapp->aboutData()->programName()));
        kdError() << "AlarmDockWindow::toggleAlarmsEnabled(): kalarmd not found" << endl;
        return;
      }
      kapp->kdeinitExecWait(execStr);
    }
  }
  else
  {
    AlarmDaemonIface_stub s( DAEMON_APP_NAME, DAEMON_DCOP_OBJECT );
    s.quit();
    if (!s.ok())
      kdDebug() << "AlarmDockWindow::toggleAlarmsEnabled(): quit dcop send failed\n";
  }
  mAlarmGui->setFastDaemonCheck();
}

/*
 * Called by a timer after the Alarms Enabled context menu item is selected,
 * to update the GUI status once the daemon has responded to the command.
 */
void AlarmDockWindow::setDaemonStatus(bool newstatus)
{
  bool oldstatus = contextMenu()->isItemChecked(mAlarmsEnabledId);
  kdDebug() << "AlarmDockWindow::setDaemonStatus(): "<<(int)oldstatus<<"->"<<(int)newstatus<<endl;
  if (newstatus != oldstatus)
  {
    setPixmap(newstatus ? mPixmapEnabled : mPixmapDisabled);
    contextMenu()->setItemChecked(mAlarmsEnabledId, newstatus);
    mSettingDaemonStatus = true;
    contextMenuAboutToShow(contextMenu());
    mSettingDaemonStatus = false;
  }
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
    for (int i = mClientIndex;  i < mClientIndex + mNumClientIds;  ++i) {
      menu->setItemChecked(menu->idAt(i), (i == menuIndex));
    }
    mAlarmGui->setDefaultClient(menuIndex);
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
  int index = mClientIndex + mNumClientIds;
  CalendarList calendars = mAlarmGui->calendars();
  ADCalendarBase *cal;
  for( cal = calendars.first(); cal; cal = calendars.next() )
  {
    if (++index == menuIndex)
    {
      AlarmDaemonIface_stub s( DAEMON_APP_NAME, DAEMON_DCOP_OBJECT );
      s.enableCal( cal->urlString(), newstate );
      break;
    }
  }
}

/*
 * Set GUI autostart at login on or off, and set the context menu accordingly.
 */
void AlarmDockWindow::setGuiAutostart(bool on)
{
kdDebug()<<"setGuiAutostart()="<<int(on)<<endl;
  KConfig* config = kapp->config();
  config->setGroup("General");
  config->writeEntry("Autostart", on);
  config->sync();
  contextMenu()->setItemChecked(mAutostartGuiId, on);
}

/*
 * Called when the daemon autostart menu option is selected, to toggle
 * the daemon's autostart state. This is done by sending a DCOP message
 * to the daemon. The menu item state is changed only when the daemon
 * notifies its new autostart status.
 */
void AlarmDockWindow::toggleDaemonAutostart()
{
  AlarmDaemonIface_stub s( DAEMON_APP_NAME, DAEMON_DCOP_OBJECT );
  s.enableAutoStart( !mAlarmGui->autostartDaemon() );
  if (!s.ok())
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
  menu->setItemEnabled(mAutostartDaemonId, mAlarmGui->isDaemonRunning(!mSettingDaemonStatus));
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
    proc << mAlarmGui->defaultClient();
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
  ClientList clients = mAlarmGui->clients();
  ClientList::ConstIterator client;
  for ( client = clients.begin();  client != clients.end();  ++client )
  {
    if (!apps.isEmpty())
      apps += '/';
    apps += (*client).title;
  }
  apps += i18n(" Alarm Monitor");

  if (!filename.isEmpty())
    apps += "\n" + filename;
  QToolTip::remove(this);
  QToolTip::add(this, apps);
}
