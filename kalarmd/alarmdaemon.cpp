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
#include <kstddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kio/netaccess.h>
#include <dcopclient.h>

#include <libkcal/calendarlocal.h>

#include "alarmdaemon.h"
#include "alarmdaemon.moc"


// Config file key strings
const QString AUTOSTART_KEY("Autostart");


AlarmDaemon::AlarmDaemon(QObject *parent, const char *name)
  : QObject(parent, name), DCOPObject(name),
    mSessionStartTimer(0L)
{
  kdDebug() << "AlarmDaemon::AlarmDaemon()" << endl;

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

  readDaemonData(!!mSessionStartTimer);

  enableAutostart(true);    // switch autostart on whenever the program is run

  // set up the alarm timer
  mAlarmTimer = new QTimer(this);
  setTimerStatus();
  checkAlarms();
}

AlarmDaemon::~AlarmDaemon()
{
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
 * DCOP call to enable or disable monitoring of a calendar.
 */
void AlarmDaemon::enableCal_(const QString& urlString, bool enable)
{
  kdDebug() << "AlarmDaemon::addCal_(" << urlString << "): " << (msgCal ? "KALARM)" : "KORGANISER)") << endl;

  ADCalendar* cal = getCalendar(urlString);
  if (cal)
  {
    cal->enabled_ = enable;
    notifyGuiCalStatus(cal);    // notify any other GUI applications
  }
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
    notifyGui((msgCal ? ADD_MSG_CALENDAR : ADD_CALENDAR), cal->urlString(), appname);
  kdDebug() << "AlarmDaemon::addCal_(): calendar added" << endl;

  setTimerStatus();
  checkAlarms(cal);
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
    cal->close();
    if (cal->loadFile())
      kdDebug() << "AlarmDaemon::reloadCal_(): calendar reloaded" << endl;
    notifyGuiCalStatus(cal);
    setTimerStatus();
    checkAlarms(cal);
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
    deleteConfigCalendar(cal);
    mCalendars.remove(cal);
    kdDebug() << "AlarmDaemon::removeCal_(): calendar removed" << endl;
    notifyGui(DELETE_CALENDAR, urlString);
    setTimerStatus();
  }
}

/*
 * DCOP call to add an application to the list of client applications,
 * and add it to the config file.
 */
void AlarmDaemon::registerApp(const QString& appName, const QString& appTitle,
                              const QString& dcopObject, int notificationType,
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
        for (ADCalendarBase* cal = mCalendars.first();  cal;  cal = mCalendars.next())
        {
          if (cal->appName() == appName)
            static_cast<ADCalendar*>(cal)->unregistered = true;
        }
        mClients.remove(appName);
      }
      ClientInfo cinfo(appTitle, dcopObject, notificationType, displayCalendarName);
      mClients.insert(appName, cinfo);

      writeConfigClient(appName, cinfo);

      enableAutostart(true);
      notifyGui(CHANGE_CLIENT);
      setTimerStatus();
      checkAlarms(appName);
    }
  }
}

/*
 * DCOP call to set autostart at login on or off.
 */
void AlarmDaemon::enableAutoStart(bool on)
{
  KConfig* config = kapp->config();
  config->setGroup("General");
  config->writeEntry(AUTOSTART_KEY, on);
  config->sync();
  notifyGui(CHANGE_STATUS);
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
  if (mEnabled)
  {
    for (ADCalendarBase* cal = mCalendars.first();  cal;  cal = mCalendars.next())
      checkAlarms(static_cast<ADCalendar*>(cal));
  }
}

/*
 * Check if any alarms are pending for any enabled calendar
 * belonging to a specified client, and display the pending alarms.
 */
void AlarmDaemon::checkAlarms(const QString& appName)
{
  if (mEnabled)
  {
    for (ADCalendarBase* cal = mCalendars.first();  cal;  cal = mCalendars.next())
    {
      if (cal->appName() == appName)
        checkAlarms(static_cast<ADCalendar*>(cal));
    }
  }
}

/*
 * Check if any alarms are pending for a specified calendar, and
 * display the pending alarms.
 * Reply = true if there were any KOrganiser type alarms.
 */
void AlarmDaemon::checkAlarms(ADCalendar* cal)
{
  if (mEnabled  &&  cal->loaded()  &&  cal->enabled())
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
            notifyEvent(cal, eventID);
          }
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
}

/*
 * Check which of the alarms for the given event are due.
 * The times in 'alarmtimes' corresponding to due alarms are set.
 */
void AlarmDaemon::checkEventAlarms(const Event& event, QValueList<QDateTime>& alarmtimes)
{
  alarmtimes.clear();
  const KOAlarm* alarm;
  QDateTime now = QDateTime::currentDateTime();
  for (QPtrListIterator<KOAlarm> it(event.alarms());  (alarm = it.current()) != 0;  ++it) {
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
        // The client application is not running
        if (client->notificationType == NO_START_NOTIFY)
          return true;

        // Start the client application
        QString execStr = locate("exe", calendar->appName());
        if (execStr.isEmpty())
        {
          kdDebug() << "AlarmDaemon::notifyEvent(): '" << calendar->appName() << "' not found" << endl;
          return true;
        }
        if (client->notificationType == COMMAND_LINE_NOTIFY)
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
  for (ADCalendar* cal = static_cast<ADCalendar*>(mCalendars.first());
       cal;  static_cast<ADCalendar*>(cal = mCalendars.next()))
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

/*
 * Starts or stops the alarm timer as necessary after a calendar is enabled/disabled.
 */
void AlarmDaemon::setTimerStatus()
{
  // Count the number of currently loaded calendars whose names should be displayed
  int nLoaded = 0;
  for (ADCalendarBase* cal = mCalendars.first();  cal;  cal = mCalendars.next()) {
    if (cal->loaded())
      ++nLoaded;
  }

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

/*
 * DCOP call to add an application to the list of GUI applications,
 * and add it to the config file.
 */
void AlarmDaemon::registerGui(const QString& appName, const QString& dcopObject)
{
  kdDebug() << "AlarmDaemon::registerApp(): " << appName << ":" << appTitle << endl;
  if (!appName.isEmpty())
  {
    const GuiInfo* g = getGuiInfo(appName);
    if (g)
    {
      // The application is already in the GUI list
      mGui.remove(appName);
    }
    mGui.insert(appName, dcopObject);

    writeConfigClientsGui(appName, dcopObject);

    ?// send list of alarms to new GUI app??
  }
}

/*
 * Send a DCOP message to all GUI interface applications, notifying them of a change
 * in calendar status.
 */
void AlarmDaemon::notifyGuiCalStatus(const ADCalendar* cal)
{
   notifyGui((cal->available() ? (cal->enabled() ? ENABLE_CALENDAR : DISABLE_CALENDAR) : CALENDAR_UNAVAILABLE),
             cal->urlString());
}

/*
 * Send a DCOP message to all GUI interface applications, notifying them of a change.
 */
void AlarmDaemon::notifyGui(GuiChangeType change, const QString& calendarURL, const QString& appName)
{
  kdDebug() << "AlarmDaemon::guiNotify(): " << eventID << endl;

  QString changeType;
  switch (change)
  {
    case CHANGE_STATUS:         changeType = "STATUS";               break;
    case CHANGE_CLIENT:         changeType = "CLIENT";               break;
    case CHANGE_GUI:            changeType = "GUI";                  break;
    case ADD_CALENDAR:          changeType = "ADD_CALENDAR";         break;
    case ADD_MSG_CALENDAR:      changeType = "ADD_MSG_CALENDAR";     break;
    case DELETE_CALENDAR:       changeType = "DELETE_CALENDAR";      break;
    case ENABLE_CALENDAR:       changeType = "ENABLE_CALENDAR";      break;
    case DISABLE_CALENDAR:      changeType = "DISABLE_CALENDAR";     break;
    case CALENDAR_UNAVAILABLE:  changeType = "CALENDAR_UNAVAILABLE"; break;
    default:
      kdError() << "AlarmDaemon::guiNotify(): " << change << endl;
      return;
  }

  for (GuiMap::ConstIterator it = mGuis.begin();  it != mGuis.end();  ++it)
  {
    const QString& dcopObject = g.data();
    if (kapp->dcopClient()->isApplicationRegistered(static_cast<const char*>(g.key()))
    {
      QByteArray data;
      QDataStream arg(data, IO_WriteOnly);
      arg << changeType << calendarURL << appName;
      if (!kapp->dcopClient()->send(static_cast<const char*>(g.key()),
                                    static_cast<const char*>(g.data()),
                                    "alarmDaemonUpdate(const QString&,const QString&,const QString&)",
                                    data))
        kdDebug() << "AlarmDaemon::guiNotify(): dcop send failed:" << g.key() << endl;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////
// class ADCalendar
///////////////////////////////////////////////////////////////////////////////
ADCalendar::EventsMap  ADCalendar::eventsHandled_;


ADCalendar::ADCalendar(const QString& url, const QString& appname, Type type)
  : ADCalendarBase(url, appname, type),
    enabled_(true),
    unregistered(false)
{
  loadFile();
}

// A virtual "constructor"
ADCalendarBase* ADCalendar::create(const QString& url, const QString& appname, Type type)
{
  return new ADCalendar(url, appname, type);
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


///////////////////////////////////////////////////////////////////////////////
// class ADConfigDataRW
///////////////////////////////////////////////////////////////////////////////

void ADConfigDataRW::readDaemonData(bool sessionStarting)
{
  kdDebug() << "ADConfigDataRW::readDaemonData()" << endl;
  bool cls, cals;
  QString newClients = readConfigData(sessionStarting, cls, cals);
  if (!newClients.isEmpty())
  {
    // One or more clients in the Clients config entry was invalid, so rewrite the entry
    clientConfig.setGroup("General");
    clientConfig.writeEntry(CLIENTS_KEY, newClients);
  }

  // Read the GUI clients
  QStringList guis = QStringList::split(',', clientConfig.readEntry(GUIS_KEY), true);
  bool writeNewGuis = false;
  QString newGuis;
  for (unsigned int i = 0;  i < guis.count();  ++i)
  {
    kdDebug() << "ADConfigDataRW::readDaemonData(): gui: " << guis[i] << endl;
    if (guis[i].isEmpty()
    ||  KStandardDirs::findExe(guis[i]) == QString::null)
    {
      // Null client name, or application doesn't exist
      if (!guis[i].isEmpty())
        clientConfig.deleteGroup(GUI_KEY + guis[i], true);
      writeNewGuis = true;
    }
    else
    {
      // Get this client's details from its own config section
      QString groupKey = GUI_KEY + guis[i];
      clientConfig.setGroup(groupKey);
      QString dcopObject = clientConfig.readEntry("DCOP object");
      mGuis.insert(guis[i], dcopObject);
      if (!newGuis.isEmpty())
        newGuis += ',';
      newGuis += guis[i];
    }
  }
  if (writeNewGuis)
  {
    // One or more clients in the Guis config entry was invalid, so rewrite the entry
    clientConfig.setGroup("General");
    clientConfig.writeEntry(GUIS_KEY, newGuis);
  }
}

/*
 * Write a client application's details to the client data file.
 * Any existing entries relating to the application are deleted,
 * including calendar file information.
 */
void ADConfigDataRW::writeConfigClient(const QString& appName, const ClientInfo& cinfo)
{
  KSimpleConfig clientConfig(clientDataFile());
  addConfigClient(clientConfig, appName, CLIENTS_KEY);

  QString groupKey = CLIENT_KEY + appName;
  clientConfig.deleteGroup(groupKey, true);

  clientConfig.setGroup(groupKey);
  clientConfig.writeEntry("Title", cinfo.title);
  if (!cinfo.dcopObject.isEmpty())
    clientConfig.writeEntry("DCOP object", cinfo.dcopObject);
  clientConfig.writeEntry("Notification", cinfo.notificationType);
  clientConfig.writeEntry("Display calendar names", cinfo.displayCalName);
  int i = 0;
  for (ADCalendarBase* cal = mCalendars.first();  cal;  cal = mCalendars.next())
  {
    if (cal->appName() == appName)
      clientConfig.writeEntry(CALENDAR_KEY + QString::number(++i), QString("%1,").arg(cal->actionType()) + cal->urlString());
  }
}

/*
 * Write a GUI client application's details to the client data file.
 */
void ADConfigDataRW::writeConfigClientGui(const QString& appName, const QString& dcopObject)
{
  KSimpleConfig clientConfig(clientDataFile());
  addConfigClient(clientConfig, appName, GUIS_KEY);

  QString groupKey = GUI_KEY + appName;

  clientConfig.setGroup(groupKey);
  clientConfig.writeEntry("DCOP object", dcopObject);
}

/*
 * Add a client application's name to the client data file list.
 */
void ADConfigDataRW::addConfigClient(KSimpleConfig& clientConfig, const QString& appName, const QString& key)
{
  clientConfig.setGroup("General");
  QStringList clients = QStringList::split(',', clientConfig.readEntry(key), true);
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
    clientConfig.writeEntry(key, clients.join(","));
  }
}

// Add a calendar file URL to the client data file for a specified application.
void ADConfigDataRW::writeConfigCalendar(const QString& appName, const ADCalendar* cal)
{
  KSimpleConfig clientConfig(clientDataFile());
  QString groupKey = CLIENT_KEY + appName;
  QMap<QString, QString> entries = clientConfig.entryMap(groupKey);
  // Find an unused CalendarN entry for this calendar
  for (int i = 1;  ;  ++i)
  {
    QString key = CALENDAR_KEY + QString::number(i);
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
void ADConfigDataRW::deleteConfigCalendar(const ADCalendar* cal)
{
  KSimpleConfig clientConfig(clientDataFile());
  QString groupKey = CLIENT_KEY + cal->appName();
  int len = CALENDAR_KEY.length();
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
