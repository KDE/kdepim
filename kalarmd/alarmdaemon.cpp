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

#include <unistd.h>
#include <stdlib.h>

#include <qtimer.h>
#include <qdatetime.h>

#include <kapp.h>
#include <kaboutdata.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kio/netaccess.h>
#include <dcopclient.h>

#include <libkcal/calendarlocal.h>

#include "alarmapp.h"
#include "dockwindow.h"
#include "alarmdialog.h"

#include "alarmdaemon.h"
#include "alarmdaemon.moc"


// The config file containing client and calendar information
const QString ClientDataFile("clients");

// Config file key strings
#define CLIENT_KEY  "Client_"
const QString CLIENTS_KEY("Clients");
const QString DEFAULT_CLIENT_KEY("Default Client");
const QString AUTOSTART_KEY("Autostart");
#define CALENDAR_KEY  "Calendar"

const int LOGIN_DELAY = 10;      // seconds to wait if ksplash can't be detected at login


AlarmDaemon::AlarmDaemon(QObject *parent, const char *name)
  : QObject(parent, name), DCOPObject(name),
    mSessionStartTimer(0L),
    mRevisingAlarmDialog(false),
    mDrawAlarmDialog(false)
{
  kdDebug() << "AlarmDaemon::AlarmDaemon()" << endl;

  mCalendars.setAutoDelete(true);

  bool splash = kapp->dcopClient()->isApplicationRegistered("ksplash");
  if (splash  ||  static_cast<AlarmApp*>(kapp)->startedAtLogin())
  {
    // Login session is starting up - need to wait for it to complete
    // in order to prevent the daemon starting clients before they are
    // restored by the session (where applicable).
    // If ksplash can be detected as running, start a 1-second timer;
    // otherwise, wait a few seconds.
    kdDebug() << "AlarmDaemon::AlarmDaemon(): session start\n";
    mSessionStartTimer = new QTimer(this);
    connect(mSessionStartTimer, SIGNAL(timeout()), SLOT(checkIfSessionStarted()));
    mSessionStartTimer->start(splash ? 1000 : LOGIN_DELAY * 1000);
  }

  QString defaultClient = readConfig();

  mDocker = new AlarmDockWindow(*this, defaultClient);
  setAutostart(true);    // switch autostart on whenever the program is run
  mDocker->show();

  mAlarmDialog = new AlarmDialog;
  connect(mAlarmDialog, SIGNAL(suspendSignal(int)), SLOT(suspend(int)));

  // set up the alarm timer
  mAlarmTimer   = new QTimer(this);
  mSuspendTimer = new QTimer(this);

  setToolTipStartTimer();
  checkAlarms();
}

AlarmDaemon::~AlarmDaemon()
{
  delete mDocker;
}

/*
 * DCOP call to quit the program.
 */
void AlarmDaemon::quit()
{
  kdDebug() << "AlarmDaemon::quit()" << endl;
  exit(0);
}

/*
 * DCOP call to add a new calendar file to the list of monitored calendars.
 * If the calendar file is already in the list, the request is ignored.
 */
void AlarmDaemon::addCal_(const QString& appname, const QString& urlString, bool msgCal)
{
  kdDebug() << "AlarmDaemon::addCal_(" << urlString << "): " << (msgCal ? "KALARM" : "KORGANISER") << endl;

  ADCalendar* cal = getCalendar(urlString);
  if (cal)
  {
    // Calendar is already being monitored
    if (!cal->unregistered)
      return;
    if (cal->appName() == appname)
    {
      cal->unregistered = false;
      reloadCal_(cal);
      return;
    }
    // The calendar used to belong to another application!
    mCalendars.remove(cal);
  }

  // Load the calendar
  cal = new ADCalendar(urlString, appname, (msgCal ? ADCalendar::KALARM : ADCalendar::KORGANISER));
  mCalendars.append(cal);

  writeConfigCalendar(appname, cal);

  if (cal->loaded())
    mDocker->updateMenuCalendars(true);
  kdDebug() << "AlarmDaemon::addCal_(): calendar added" << endl;

  setToolTipStartTimer();
  checkAlarms(cal, true);
}

/*
 * DCOP call to reload the specified calendar.
 * The calendar is first added to the list of monitored calendars if necessary.
 */
void AlarmDaemon::reloadCal_(const QString& appname, const QString& urlString, bool msgCal)
{
  kdDebug() << "AlarmDaemon::reloadCal_(" << urlString << "): " << (msgCal ? "KALARM" : "KORGANISER") << endl;

  if (!urlString.isEmpty())
  {
    ADCalendar* cal = getCalendar(urlString);
    if (cal)
      reloadCal_(cal);
    else
    {
      // Calendar wasn't in the list, so add it
      if (!appname.isEmpty())
        addCal_(appname, urlString, msgCal);
    }
  }
}

/*
 * Reload the specified calendar.
 */
void AlarmDaemon::reloadCal_(ADCalendar* cal)
{
  kdDebug() << "AlarmDaemon::reloadCal_(): calendar" << endl;

  if (cal)
  {
    if (cal->actionType() == ADCalendar::KORGANISER)
      removeDialogEvents(cal);

    cal->close();
    if (cal->loadFile())
      kdDebug() << "AlarmDaemon::reloadCal_(): calendar reloaded" << endl;
    mDocker->updateMenuCalendars(false);
    setToolTipStartTimer();
    checkAlarms(cal, true);
  }
}

/*
 * DCOP call to reload the specified calendar and reset the data associated with it.
 * The calendar is first added to the list of monitored calendars if necessary.
 */
void AlarmDaemon::resetMsgCal_(const QString& appname, const QString& urlString)
{
  kdDebug() << "AlarmDaemon::resetMsgCal_(" << urlString << ")\n";

  reloadCal_(appname, urlString, true);
  ADCalendar::clearEventsHandled(urlString);
}

/* Remove a calendar file from the list of monitored calendars */
void AlarmDaemon::removeCal_(const QString& urlString)
{
  kdDebug() << "AlarmDaemon::removeCal_(" << urlString << ")\n";

  ADCalendar* cal = getCalendar(urlString);
  if (cal)
  {
    if (cal->loaded()  &&  cal->actionType() == ADCalendar::KORGANISER)
      removeDialogEvents(cal);
    deleteConfigCalendar(cal);
    mCalendars.remove(cal);
    kdDebug() << "AlarmDaemon::removeCal_(): calendar removed" << endl;
    mDocker->updateMenuCalendars(true);
    setToolTipStartTimer();
  }
}

/*
 * DCOP call to add an application to the list of client applications,
 * and add it to the config file.
 */
void AlarmDaemon::registerApp(const QString& appName, const QString& appTitle,
                              const QString& dcopObject, bool commandLineNotify,
                              bool displayCalendarName)
{
  kdDebug() << "AlarmDaemon::registerApp(" << appName << ", " << appTitle << ")\n";
  if (!appName.isEmpty())
  {
    if (KStandardDirs::findExe(appName) == QString::null)
      kdError() << "AlarmDaemon::registerApp(): app not found\n";
    else
    {
      const ClientInfo* c = getClientInfo(appName);
      if (c)
      {
        // The application is already in the clients list.
        // Mark all its calendar files as unregistered and remove it from the list.
        for (ADCalendar* cal = mCalendars.first();  cal;  cal = mCalendars.next())
        {
          if (cal->appName() == appName)
            cal->unregistered = true;
        }
        mClients.remove(appName);
      }
      ClientInfo cinfo(appTitle, dcopObject, commandLineNotify, displayCalendarName, false);
      mClients.insert(appName, cinfo);

      writeConfigClient(appName, cinfo);

      setAutostart(true);
      mDocker->updateMenuClients();
      setToolTipStartTimer();
      notifyPendingEvents(appName);
      checkAlarms(appName);
    }
  }
}

/*
 * Must be called at program initialisation only.
 * Read the default client application from the config file.
 * Read all client applications from the client data file and store them in the client list.
 * Open all calendar files listed in the client data file and start monitoring them.
 * Calendar files are monitored until their client application registers, upon which
 * monitoring ceases until the client application tell the daemon to monitor it.
 * Reply = name of the default client application.
 */
QString AlarmDaemon::readConfig()
{
  kdDebug() << "AlarmDaemon::readConfig()" << endl;
  mClientDataFile = locateLocal("appdata", ClientDataFile);
  KSimpleConfig clientConfig(mClientDataFile);
  clientConfig.setGroup("General");
  QStringList clients = QStringList::split(',', clientConfig.readEntry(CLIENTS_KEY), true);
  bool writeNewClients = false;
  QString newClients;
  for (unsigned int i = 0;  i < clients.count();  ++i)
  {
    kdDebug() << "AlarmDaemon::readConfig(): client: " << clients[i] << endl;
    if (clients[i].isEmpty()
    ||  KStandardDirs::findExe(clients[i]) == QString::null)
    {
      // Null client name, or application doesn't exist
      if (!clients[i].isEmpty())
        clientConfig.deleteGroup(QString(CLIENT_KEY) + clients[i], true);
      writeNewClients = true;
    }
    else
    {
      // Get this client's details from its own config section
      ClientMap::Iterator c = mClients.find(clients[i]);
      bool found = (c != mClients.end());
      ClientInfo newinfo;
      ClientInfo* info = found ? &c.data() : &newinfo;
      QString groupKey = QString(CLIENT_KEY) + clients[i];
      clientConfig.setGroup(groupKey);
      info->title             = clientConfig.readEntry("Title", clients[i]);   // read app title (default = app name)
      info->dcopObject        = clientConfig.readEntry("DCOP object");
      info->commandLineNotify = clientConfig.readBoolEntry("Command line notify", false);
      info->displayCalName    = clientConfig.readBoolEntry("Display calendar names", true);
      info->menuIndex         = 0;
      if (!found)
      {
        info->waitForRegistration = !!mSessionStartTimer;
        mClients.insert(clients[i], newinfo);
      }

      // Get the client's calendar files
      int len = strlen(CALENDAR_KEY);
      QMap<QString, QString> entries = clientConfig.entryMap(groupKey);
      for (QMap<QString, QString>::ConstIterator it = entries.begin();  it != entries.end();  ++it)
      {
        kdDebug() << "AlarmDaemon::readConfig(): " << it.key() << "=" << it.data() << endl;
        if (it.key().startsWith(CALENDAR_KEY))
        {
          bool ok;
          it.key().mid(len).toInt(&ok);
          if (ok)
          {
            // The config file key is CalendarN, so open the calendar file
            int comma = it.data().find(',');
            if (comma >= 0)
            {
              QString calname = it.data().mid(comma + 1);
              ADCalendar* cal = getCalendar(calname);
              if (cal)
              {
                // The calendar is already in the client's list, so remove
                // this redundant client data file entry.
                deleteConfigCalendar(cal);
              }
              else
              {
                // Add the calendar to the client's list
                cal = new ADCalendar(calname, clients[i],
                                     static_cast<ADCalendar::Type>(it.data().left(comma).toInt()),
                                     true);
                mCalendars.append(cal);
                kdDebug() << "AlarmDaemon::readConfig(): calendar " << cal->urlString() << endl;
              }
            }
          }
        }
      }

      if (!newClients.isEmpty())
        newClients += ',';
      newClients += clients[i];
    }
  }
  if (writeNewClients)
  {
    // One or more clients in the Clients config entry was invalid, so rewrite the entry
    clientConfig.setGroup("General");
    clientConfig.writeEntry(CLIENTS_KEY, newClients);
  }

  // Read the default client application
  return checkDefaultClient();
}

/*
 * Write a client application's details to the client data file.
 * Any existing entries relating to the application are deleted,
 * including calendar file information.
 */
void AlarmDaemon::writeConfigClient(const QString& appName, const ClientInfo& cinfo)
{
  KSimpleConfig clientConfig(mClientDataFile);
  clientConfig.setGroup("General");
  QStringList clients = QStringList::split(',', clientConfig.readEntry(CLIENTS_KEY), true);
  if (clients.find(appName) == clients.end())
  {
    // It's a new client, so add it to the Clients config file entry
    for (QStringList::Iterator i = clients.begin();  i != clients.end();  )
    {
      if ((*i).isEmpty())
        i = clients.remove(i);    // remove null entries
      else
        ++i;
    }
    clients.append(appName);
    clientConfig.writeEntry(CLIENTS_KEY, clients.join(","));
  }

  QString groupKey = QString(CLIENT_KEY) + appName;
  clientConfig.deleteGroup(groupKey, true);

  clientConfig.setGroup(groupKey);
  clientConfig.writeEntry("Title", cinfo.title);
  if (!cinfo.dcopObject.isEmpty())
    clientConfig.writeEntry("DCOP object", cinfo.dcopObject);
  clientConfig.writeEntry("Command line notify", cinfo.commandLineNotify);
  clientConfig.writeEntry("Display calendar names", cinfo.displayCalName);
  int i = 0;
  for (ADCalendar* cal = mCalendars.first();  cal;  cal = mCalendars.next())
  {
    if (cal->appName() == appName)
      clientConfig.writeEntry(QString(CALENDAR_KEY"%1").arg(++i), QString("%1,").arg(cal->actionType()) + cal->urlString());
  }

  // Set the default client if it's currently null
  checkDefaultClient();
}

/*
 * Check that the default client is in the list of client applications.
 * If not, set it to the first client application and update the client data file.
 */
QString AlarmDaemon::checkDefaultClient()
{
  // Read the default client application
  KConfig* config = kapp->config();
  config->setGroup("General");
  QString defaultClient = config->readEntry(DEFAULT_CLIENT_KEY);

  if (!getClientInfo(defaultClient))
  {
    // Default client isn't in the list of clients.
    // Replace it with the first client in the list.
    defaultClient = mClients.count() ? mClients.begin().key() : QString();
    config->writeEntry(DEFAULT_CLIENT_KEY, defaultClient);
    config->sync();
  }
  return defaultClient;
}

void AlarmDaemon::setDefaultClient(int menuIndex)
{
  for (ClientMap::ConstIterator client = mClients.begin();  client != mClients.end();  ++client)
  {
    if (client.data().menuIndex == menuIndex)
    {
      QString defaultClient = client.key();
      KConfig* config = kapp->config();
      config->setGroup("General");
      config->writeEntry(DEFAULT_CLIENT_KEY, defaultClient);
      config->sync();
    }
  }
}

/*
 * Set autostart at login on or off, and set the context menu accordingly.
 */
void AlarmDaemon::setAutostart(bool on)
{
  KConfig* config = kapp->config();
  config->setGroup("General");
  config->writeEntry(AUTOSTART_KEY, on);
  config->sync();
  mDocker->setAutostart(on);
}

/*
 * Called when the autostart menu option is selected, to
 * toggle the autostart state.
 */
void AlarmDaemon::toggleAutostart()
{
  setAutostart(!mDocker->autostartOn());
}

// Add a calendar file URL to the client data file for a specified application.
void AlarmDaemon::writeConfigCalendar(const QString& appName, const ADCalendar* cal)
{
  KSimpleConfig clientConfig(mClientDataFile);
  QString groupKey = QString(CLIENT_KEY) + appName;
  QMap<QString, QString> entries = clientConfig.entryMap(groupKey);
  // Find an unused CalendarN entry for this calendar
  for (int i = 1;  ;  ++i)
  {
    QString key = QString(CALENDAR_KEY"%1").arg(i);
    if (entries.find(key) == entries.end())
    {
      // This calendar index is unused, so use it for the new calendar
      clientConfig.setGroup(groupKey);
      clientConfig.writeEntry(key, QString("%1,").arg(cal->actionType()) + cal->urlString());
      return;
    }
  }
}

/*
 * Delete all entries in the client data file for the specified calendar
 */
void AlarmDaemon::deleteConfigCalendar(const ADCalendar* cal)
{
  KSimpleConfig clientConfig(mClientDataFile);
  QString groupKey = QString(CLIENT_KEY) + cal->appName();
  int len = strlen(CALENDAR_KEY);
  QMap<QString, QString> entries = clientConfig.entryMap(groupKey);
  for (int i = 1;  ;  ++i)
  {
    for (QMap<QString, QString>::ConstIterator it = entries.begin();  it != entries.end();  ++it)
    {
      if (it.key().startsWith(CALENDAR_KEY))
      {
        bool ok;
        it.key().mid(len).toInt(&ok);
        if (ok)
        {
          // The config file key is CalendarN
          int comma = it.data().find(',');
          if (comma >= 0  &&  it.data().mid(comma + 1) == cal->urlString())
          {
            clientConfig.setGroup(groupKey);
            clientConfig.deleteEntry(it.key(), true);
          }
        }
      }
    }
  }
}

/*
 * Check if any alarms are pending for any enabled calendar, and
 * display the pending alarms.
 * Called by the alarm timer.
 */
void AlarmDaemon::checkAlarmsSlot()
{
  if (mAlarmTimerSyncing)
  {
    // We've synced to the minute boundary. Now set timer to 1 minute intervals.
    mAlarmTimer->changeInterval(1000 * 60);
    mAlarmTimerSyncing = false;
  }
  checkAlarms();
}

/*
 * Check if any alarms are pending for any enabled calendar, and
 * display the pending alarms.
 */
void AlarmDaemon::checkAlarms()
{
  // Leave immediately if alarms are off
  if (!mDocker->alarmsOn()) return;

  bool korgAlarms = false;
  for (ADCalendar* cal = mCalendars.first();  cal;  cal = mCalendars.next())
  {
    if (checkAlarms(cal, false))
      korgAlarms = true;
  }
  if (korgAlarms)
    showAlarmDialog();
}

/*
 * Check if any alarms are pending for any enabled calendar
 * belonging to a specified client, and display the pending alarms.
 */
void AlarmDaemon::checkAlarms(const QString& appName)
{
  // Leave immediately if alarms are off
  if (!mDocker->alarmsOn()) return;

  bool korgAlarms = false;
  for (ADCalendar* cal = mCalendars.first();  cal;  cal = mCalendars.next())
  {
    if (cal->appName() == appName
    &&  checkAlarms(cal, false))
      korgAlarms = true;
  }
  if (korgAlarms)
    showAlarmDialog();
}

/*
 * Check if any alarms are pending for a specified calendar, and
 * display the pending alarms.
 * Reply = true if there were any KOrganiser type alarms.
 */
bool AlarmDaemon::checkAlarms(ADCalendar* cal, bool showDialog)
{
  bool korgAlarms = false;
  if (mDocker->alarmsOn()  &&  cal->loaded()  &&  cal->enabled())
  {
    QPtrList<Event> alarmEvents;
    switch (cal->actionType())
    {
      case ADCalendar::KORGANISER:
        if (cal->checkNonRecurringAlarms(alarmEvents)
        ||  cal->checkRecurringAlarms(alarmEvents, true))
        {
          for (Event* event = alarmEvents.first();  event;  event = alarmEvents.next())
          {
            kdDebug() << "AlarmDaemon::checkAlarms(): KORGANISER event " << event->VUID() << endl;
            mAlarmDialog->appendEvent(cal, event);
          }
          korgAlarms = true;
        }
        break;

      case ADCalendar::KALARM:
        if (cal->checkAlarmsPast(alarmEvents))
        {
kdDebug()<<"Kalarm alarms="<<alarmEvents.count()<<endl;
          for (Event* event = alarmEvents.first();  event;  event = alarmEvents.next())
          {
            const QString& eventID = event->VUID();
            kdDebug() << "AlarmDaemon::checkAlarms(): KALARM event " << eventID  << endl;
            QValueList<QDateTime> alarmtimes;
            checkEventAlarms(*event, alarmtimes);
            if (!cal->eventHandled(event, alarmtimes))
            {
              if (notifyEvent(cal, eventID))
                cal->setEventHandled(event, alarmtimes);
              else
                cal->setEventPending(eventID);
            }
          }
        }
        break;
    }
  }

  if (showDialog  &&  korgAlarms)
    showAlarmDialog();
  return korgAlarms;
}

/*
 * Check which of the alarms for the given event are due.
 * The times in 'alarmtimes' corresponding to due alarms are set.
 */
void AlarmDaemon::checkEventAlarms(const Event& event, QValueList<QDateTime>& alarmtimes)
{
  alarmtimes.clear();
  const Alarm* alarm;
  QDateTime now = QDateTime::currentDateTime();
  for (QPtrListIterator<Alarm> it(event.alarms());  (alarm = it.current()) != 0;  ++it) {
    alarmtimes.append((alarm->enabled()  &&  alarm->time() <= now) ? alarm->time() : QDateTime());
  }
}

/*
 * Send a DCOP message to a client application telling it that an alarm
 * should now be handled.
 * Reply = false if the event should be held pending until the client
 *         application can be started.
 */
bool AlarmDaemon::notifyEvent(const ADCalendar* calendar, const QString& eventID)
{
  kdDebug() << "AlarmDaemon::notifyEvent(" << eventID << ")\n";
  if (calendar)
  {
    const ClientInfo* client = getClientInfo(calendar->appName());
    if (!client)
      kdDebug() << "AlarmDaemon::notifyEvent(): unknown client" << endl;
    else
    {
      if (client->waitForRegistration)
      {
        // Don't start the client application if the session manager is still
        // starting the session, since if we start the client before the
        // session manager does, a KUniqueApplication client will not then be
        // able to restore its session.
        // And don't contact a client which was started by the login session
        // until it's ready to handle DCOP calls.
        kdDebug() << "AlarmDaemon::notifyEvent(): wait for session startup" << endl;
        return false;
      }
      if (!kapp->dcopClient()->isApplicationRegistered(static_cast<const char*>(calendar->appName())))
      {
        // The client application is not running, so start it
        QString execStr = locate("exe", calendar->appName());
        if (execStr.isEmpty())
        {
          kdDebug() << "AlarmDaemon::notifyEvent(): '" << calendar->appName() << "' not found" << endl;
          return true;
        }
        if (client->commandLineNotify)
        {
          // Use the command line to tell the client about the alarm
          execStr += " --handleEvent ";
          execStr += eventID;
          execStr += " --calendarURL ";
          execStr += calendar->urlString();
          system(execStr.latin1());
          kdDebug() << "AlarmDaemon::notifyEvent(): used command line" << endl;
          return true;
        }
        system(execStr.latin1());
        kdDebug() << "AlarmDaemon::notifyEvent(): started " << execStr.latin1() << endl;
      }

      // Use DCOP to tell the client about the alarm
      QByteArray data;
      QDataStream arg(data, IO_WriteOnly);
      arg << calendar->urlString() << eventID;
      if (!kapp->dcopClient()->send(static_cast<const char*>(calendar->appName()),
                                    static_cast<const char*>(client->dcopObject),
                                    "handleEvent(const QString&,const QString&)",
                                    data))
        kdDebug() << "AlarmDaemon::notifyEvent(): dcop send failed" << endl;
    }
  }
  return true;
}

/* Notify the specified client of any pending alarms */
void AlarmDaemon::notifyPendingEvents(const QString& appname)
{
  kdDebug() << "AlarmDaemon::notifyPendingEvents(" << appname << ")\n";
  for (ADCalendar* cal = mCalendars.first();  cal;  cal = mCalendars.next())
  {
    if (cal->appName() == appname
    &&  cal->actionType() == ADCalendar::KALARM)
    {
      QString eventID;
      while (cal->getEventPending(eventID))
      {
        notifyEvent(cal, eventID);
        const Event* event = cal->getEvent(eventID);
        QValueList<QDateTime> alarmtimes;
        checkEventAlarms(*event, alarmtimes);
        cal->setEventHandled(event, alarmtimes);
      }
    }
  }
}

/*
 * Called by the timer to check whether session startup is complete.
 * If so, it checks which clients are already running and notifies
 * any which have registered of any pending alarms.
 * (Ideally checking for session startup would be done using a signal
 * from ksmserver, but until such a signal is available, we can check
 * whether ksplash is still running.)
 */
void AlarmDaemon::checkIfSessionStarted()
{
  if (!kapp->dcopClient()->isApplicationRegistered("ksplash"))
  {
    // Session startup has now presumably completed. Cancel the timer.
    kdDebug() << "AlarmDaemon::checkIfSessionStarted(): startup complete\n";
    delete mSessionStartTimer;

    // Notify clients which are not yet running of pending alarms
    for (ClientMap::Iterator client = mClients.begin();  client != mClients.end();  ++client)
    {
      if (!kapp->dcopClient()->isApplicationRegistered(static_cast<const char*>(client.key())))
      {
        client.data().waitForRegistration = false;
        notifyPendingEvents(client.key());
      }
    }

    mSessionStartTimer = 0L;    // indicate that session startup is complete
  }
}

/* Schedule the alarm dialog for redisplay after a specified number of minutes */
void AlarmDaemon::suspend(int minutes)
{
//  kdDebug() << "AlarmDaemon::suspend() " << minutes << " minutes" << endl;
  connect(mSuspendTimer, SIGNAL(timeout()), SLOT(showAlarmDialog()));
  mSuspendTimer->start(1000*60*minutes, true);
}

/* Display the alarm dialog (showing KOrganiser-type events) */
void AlarmDaemon::showAlarmDialog()
{
  if (mRevisingAlarmDialog)
    mDrawAlarmDialog = true;
  else
  {
    KNotifyClient::beep();
    mAlarmDialog->show();
    mAlarmDialog->eventNotification();
    mDrawAlarmDialog = false;
  }
}

/* Remove all events belonging to the specified calendar from the alarm dialog */
void AlarmDaemon::removeDialogEvents(const Calendar* calendar)
{
  mRevisingAlarmDialog = true;   // prevent dialog being displayed while it's being changed
  if (mAlarmDialog->clearEvents(calendar) > 0)
  {
    // There are still some events left in the dialog, so display it
    // if the suspend time has expired
    mRevisingAlarmDialog = false;
    if (mDrawAlarmDialog)
      showAlarmDialog();
  }
  else
  {
    // The dialog is now empty, so tidy up
    mSuspendTimer->stop();
    mRevisingAlarmDialog = false;
    mDrawAlarmDialog = false;
  }
}

/*
 * Expand a DCOP call parameter URL to a full URL.
 * (We must store full URLs in the calendar data since otherwise
 *  later calls to reload or remove calendars won't necessarily
 *  find a match.)
 */
QString AlarmDaemon::expandURL(const QString& urlString)
{
  if (urlString.isEmpty())
    return QString();
  return KURL(urlString).url();
}

/* Return the ClientInfo structure for the specified client application */
const ClientInfo* AlarmDaemon::getClientInfo(const QString& appName) const
{
  if (!appName.isEmpty())
  {
    ClientMap::ConstIterator c = mClients.find(appName);
    if (c != mClients.end())
      return &c.data();
  }
  return 0L;
}

/* Return the ADCalendar structure for the specified full calendar URL */
ADCalendar* AlarmDaemon::getCalendar(const QString& calendarURL)
{
  if (!calendarURL.isEmpty())
  {
    for (ADCalendar* cal = mCalendars.first();  cal;  cal = mCalendars.next())
    {
      if (cal->urlString() == calendarURL)
        return cal;
    }
  }
  return 0L;
}

/*
 * Adds the appropriate calendar file name to the panel tool tip.
 * Starts or stops the alarm timer as necessary.
 */
void AlarmDaemon::setToolTipStartTimer()
{
  // Count the number of currently loaded calendars whose names should be displayed
  int nLoaded = 0;
  int nForDisplay = 0;
  ADCalendar* firstForDisplay = 0L;
  for (ADCalendar* cal = mCalendars.first();  cal;  cal = mCalendars.next())
  {
    if (cal->loaded())
    {
      const ClientInfo* c = getClientInfo(cal->appName());
      if (c  &&  c->displayCalName  &&  !nForDisplay++)
        firstForDisplay = cal;
      ++nLoaded;
    }
  }

  // Display the appropriate tooltip
  QString filename;
  if (nForDisplay == 1)
  {
    // Display the name of the one and only calendar whose name is to be displayed
    KURL url(firstForDisplay->urlString());
    if (url.isLocalFile())
      filename = KURL::decode_string(url.path());
    else
      filename = url.prettyURL();
  }
  else if (!nLoaded)
    filename = i18n("No calendar loaded.");
  mDocker->addToolTip(filename);

  // Start or stop the alarm timer if necessary
  if (!mAlarmTimer->isActive() && nLoaded)
  {
    // Timeout every minute.
    // But first synchronise to one second after the minute boundary.
    int firstInterval = 61 - QTime::currentTime().second();
    mAlarmTimer->start(1000 * firstInterval);
    mAlarmTimerSyncing = (firstInterval != 60);
    connect(mAlarmTimer, SIGNAL(timeout()), SLOT(checkAlarmsSlot()));
    kdDebug() << "Started alarm timer" << endl;
  }
  else if (mAlarmTimer->isActive() && !nLoaded)
  {
    mAlarmTimer->disconnect();
    mAlarmTimer->stop();
    kdDebug() << "Stopped alarm timer" << endl;
  }
}


///////////////////////////////////////////////////////////////////////////////
// class ClientInfo
///////////////////////////////////////////////////////////////////////////////

ClientInfo::ClientInfo(const QString& titl, const QString& dcopObj, bool cmdLine, bool disp, bool wait)
  : title(titl),
    dcopObject(dcopObj),
    menuIndex(0),
    commandLineNotify(cmdLine),
    displayCalName(disp),
    waitForRegistration(wait)
{
}

///////////////////////////////////////////////////////////////////////////////
// class ADCalendar
///////////////////////////////////////////////////////////////////////////////
ADCalendar::EventsMap  ADCalendar::eventsHandled_;


ADCalendar::ADCalendar(const QString& url, const QString& appname, Type type, bool quiet)
  : enabled_(true),
    unregistered(false),
    urlString_(url),
    appName_(appname),
    actionType_(type),
    loaded_(false)
{
  showDialogs(FALSE);
  loadFile(quiet);
}

/*
 * Load the calendar file.
 */
bool ADCalendar::loadFile(bool quiet)
{
  loaded_ = false;
  KURL url(urlString_);
  QString tmpFile;
  if (KIO::NetAccess::download(url, tmpFile))
  {
    kdDebug() << "--- Downloaded to " << tmpFile << endl;
    loaded_ = load(tmpFile);
    KIO::NetAccess::removeTempFile(tmpFile);
    if (!loaded_)
      kdDebug() << "ADCalendar::loadFile(): Error loading calendar file '" << tmpFile << "'\n";
    else
    {
      // Remove all now non-existent events from the handled list
      for (EventsMap::Iterator it = eventsHandled_.begin();  it != eventsHandled_.end();  )
      {
        if (it.data().calendarURL == urlString_  &&  !getEvent(it.key()))
        {
          // Event belonged to this calendar, but can't find it any more
          EventsMap::Iterator i = it;
          ++it;                      // prevent iterator becoming invalid with remove()
          eventsHandled_.remove(i);
        }
        else
          ++it;
      }
    }
  }
  else if (!quiet)
    KMessageBox::error(0L, i18n("Cannot download calendar from\n%1.").arg(url.prettyURL()), kapp->aboutData()->programName());
  return loaded_;
}

/*
 * Check whether all the alarms for the event with the given ID have already
 * been handled.
 */
bool ADCalendar::eventHandled(const Event* event, const QValueList<QDateTime>& alarmtimes)
{
  EventsMap::ConstIterator it = eventsHandled_.find(event->VUID());
  if (it == eventsHandled_.end())
    return false;

  int oldCount = it.data().alarmTimes.count();
  int count = alarmtimes.count();
  for (int i = 0;  i < count;  ++i) {
    if (alarmtimes[i].isValid()) {
      if (i >= oldCount                              // is it an additional alarm?
      ||  !it.data().alarmTimes[i].isValid()         // or has it just become due?
      ||  it.data().alarmTimes[i].isValid()          // or has it changed?
       && alarmtimes[i] != it.data().alarmTimes[i])
        return false;     // this alarm has changed
    }
  }
  return true;
}

/*
 * Remember that the specified alarms for the event with the given ID have been
 * handled.
 */
void ADCalendar::setEventHandled(const Event* event, const QValueList<QDateTime>& alarmtimes)
{
  if (event)
  {
    kdDebug() << "ADCalendar::setEventHandled(" << event->VUID() << ")\n";
    EventsMap::Iterator it = eventsHandled_.find(event->VUID());
    if (it != eventsHandled_.end())
    {
      // Update the existing entry for the event
      it.data().alarmTimes = alarmtimes;
      it.data().eventSequence = event->revision();
    }
    else
      eventsHandled_.insert(event->VUID(), EventItem(urlString_, event->revision(), alarmtimes));
  }
}

/*
 * Clear all memory of events handled for the specified calendar.
 */
void ADCalendar::clearEventsHandled(const QString& calendarURL)
{
  for (EventsMap::Iterator it = eventsHandled_.begin();  it != eventsHandled_.end();  )
  {
    if (it.data().calendarURL == calendarURL)
    {
      EventsMap::Iterator i = it;
      ++it;                      // prevent iterator becoming invalid with remove()
      eventsHandled_.remove(i);
    }
    else
      ++it;
  }
}

/*
 * Note that the event with the given ID is pending
 * (i.e. waiting until the client can be notified).
 */
void ADCalendar::setEventPending(const QString& ID)
{
  if (actionType_ == KALARM  &&  !eventsPending_.containsRef(&ID))
  {
    eventsPending_.append(&ID);
    kdDebug() << "ADCalendar::setEventPending(): " << ID << endl;
  }
}

/*
 * Get an event from the pending list, and remove it from the list.
 */
bool ADCalendar::getEventPending(QString& ID)
{
  if (actionType_ == KALARM)
  {
    QString* eventID = eventsPending_.getFirst();
    if (eventID)
    {
      eventsPending_.removeFirst();
      ID = *eventID;
      return true;
    }
  }
  return false;
}

ADCalendar::EventItem::EventItem(const QString& url, int seqno, const QValueList<QDateTime>& alarmtimes)
  : calendarURL(url),
    eventSequence(seqno),
    alarmTimes(alarmtimes)
{
}
