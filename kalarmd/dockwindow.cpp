// KDE Panel docking window for KDE Alarm Daemon.
// (c) 2001 David Jarvie
// Based on the original, (c) 1998, 1999 Preston Brown

#include <qtooltip.h>

#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <kconfig.h>
#include <kmessagebox.h>

#include "dockwindow.h"


AlarmDockWindow::AlarmDockWindow(AlarmDaemon& ad, const QString& defltClient,
                                 QWidget *parent, const char *name)
  : KSystemTray(parent, name),
    alarmDaemon(ad),
    defaultClient(defltClient),
    nClientIds(0),
    nCalendarIds(0)
{
  KGlobal::iconLoader()->addAppDir("kalarmd");
  dPixmap1 = BarIcon("kalarmd");
  dPixmap2 = BarIcon("kalarmd_disabled");

  if (dPixmap1.isNull() || dPixmap2.isNull()) {
    KMessageBox::sorry(this, i18n("Can't load docking tray icon!"),
                             i18n("Alarm Monitor Error"));
  }
  setPixmap(dPixmap1);

  alarmsEnabledId = contextMenu()->insertItem(i18n("Alarms Enabled"),
               this, SLOT(toggleAlarmsEnabled()));
  autostartId = contextMenu()->insertItem(i18n("Auto start at login"),
               &alarmDaemon, SLOT(toggleAutostart()));
  contextMenu()->setItemChecked(alarmsEnabledId, true);
  contextMenu()->setItemChecked(autostartId, true);
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
                                this, SLOT(select(int)), 0, -1, index);
      menu->setItemParameter(id, index);    // set parameter for select()
      menu->setItemChecked(id, (client.appName() == defaultClient));
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
        menu->setItemEnabled(id, cal.available());
        menu->setItemChecked(id, (cal.loaded() && cal.enabled()));
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
      menu->setItemEnabled(id, cal.loaded());
      menu->setItemChecked(id, (cal.loaded() && cal.enabled()));
    }
  }
}

/*
 * Called when a client application is selected in the context menu.
 * The config file is updated with the new default client.
 */
void AlarmDockWindow::select(int menuIndex)
{
  KPopupMenu* menu = contextMenu();
  kdDebug() << "AlarmDockWindow::select(): "<<menuIndex<<endl;
  if (!menu->isItemChecked(menu->idAt(menuIndex)))
  {
    for (int i = clientIndex;  i < clientIndex + nClientIds;  ++i)
      menu->setItemChecked(menu->idAt(i), (i == menuIndex));

    alarmDaemon.setDefaultClient(menuIndex);
  }
}

/*
 * Called when a calendar is selected in the context menu.
 * The calendar's enable state is toggled.
 */
void AlarmDockWindow::selectCal(int menuIndex)
{
  KPopupMenu* menu = contextMenu();
  int id = menu->idAt(menuIndex);
  bool newstate = !menu->isItemChecked(id);
  kdDebug() << "AlarmDockWindow::selectCal(): "<<menuIndex<<": id="<<id<<" newstate="<<(int)newstate<<endl;
  menu->setItemChecked(id, newstate);
  int index = clientIndex + nClientIds;
  for (CalendarIteration cal = alarmDaemon.getCalendarIteration();  cal.ok();  cal.next())
  {
    if (++index == menuIndex)
      cal.enabled(newstate);
  }
}

void AlarmDockWindow::mousePressEvent(QMouseEvent *e)
{
  if (e->button() == LeftButton) {
    // Start up the default client application
    KProcess proc;
    proc << defaultClient;
    proc.start(KProcess::DontCare);
  } else
    KSystemTray::mousePressEvent(e);
}

void AlarmDockWindow::closeEvent(QCloseEvent *)
{
  kapp->quit();
}

void AlarmDockWindow::addToolTip(const QString &filename)
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
