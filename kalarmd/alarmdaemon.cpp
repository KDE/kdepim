// KDE Alarm Daemon.
// (c) 2001 David Jarvie
// Based on the original, (c) 1998, 1999 Preston Brown

#include <unistd.h>
#include <stdlib.h>

#include <qtimer.h>
#include <qdatetime.h>

#include <kapp.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kio/netaccess.h>

#include <calendarlocal.h>

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


AlarmDaemon::AlarmDaemon(QObject *parent, const char *name)
  : QObject(parent, name), DCOPObject(name),
    mSessionStartTimer(0L),
    mRevisingAlarmDialog(false),
    mDrawAlarmDialog(false),
    mSessionStarted(false)
{
  kdDebug() << "AlarmDaemon::AlarmDaemon()" << endl;

  mCalendars.setAutoDelete(true);

  QString defaultClient = readConfig();

  mDocker = new AlarmDockWindow(*this, defaultClient);
  setAutostart(true);    // switch autostart on whenever the program is run
  mDocker->show();

  mAlarmDialog = new AlarmDialog;
  connect(mAlarmDialog, SIGNAL(suspendSignal(int)), SLOT(suspend(int)));

  // set up the alarm timer
  mAlarmTimer   = new QTimer(this);
  mSuspendTimer = new QTimer(this);

  mCalendars.setAutoDelete(true);

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
  kdDebug() << "AlarmDaemon::addCal_(): '" << urlString << "' (" << (msgCal ? "KALARM)" : "KORGANISER)") << endl;

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
  kdDebug() << "AlarmDaemon::reloadCal_(): '" << urlString << "' (" << (msgCal ? "KALARM)" : "KORGANISER)") << endl;

  if (!urlString.isEmpty())
  {
    ADCalendar* cal = getCalendar(urlString);
    if (cal)
    {
      reloadCal_(cal);
    }
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
  kdDebug() << "AlarmDaemon::resetMsgCal_(): '" << urlString << "'" << endl;

  reloadCal_(appname, urlString, true);
  ADCalendar::clearEventsHandled(urlString);
}

/* Remove a calendar file from the list of monitored calendars */
void AlarmDaemon::removeCal_(const QString& urlString)
{
  kdDebug() << "AlarmDaemon::removeCal_(): '" << urlString << "'" << endl;

  ADCalendar* cal = getCalendar(urlString);
  if (cal)
  {
    if (cal->loaded()  &&  cal->actionType() == ADCalendar::KORGANISER)
      removeDialogEvents(cal);
    deleteConfigCalendar(cal);
    mCalendars.remove();
    kdDebug() << "AlarmDaemon::removeCal_(): calendar removed" << endl;
    mDocker->updateMenuCalendars(true);
    setToolTipStartTimer();
  }
}

/*
 * Add an application to the list of client applications,
 * and add it to the config file.
 */
void AlarmDaemon::registerApp(const QString& appName, const QString& appTitle,
                              const QString& dcopObject, bool commandLineNotify,
                              bool displayCalendarName)
{
  kdDebug() << "AlarmDaemon::registerApp(): " << appName << ":" << appTitle << endl;
  if (!appName.isEmpty())
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
    ClientInfo cinfo(appTitle, dcopObject, commandLineNotify, displayCalendarName);
    mClients.insert(appName, cinfo);

    writeConfigClient(appName, cinfo);

    setAutostart(true);
    mDocker->updateMenuClients();
    setToolTipStartTimer();
    checkAlarms(appName);
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
  for (int i = 0;  i < clients.count();  ++i)
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
      ClientInfo info;
      QString groupKey = QString(CLIENT_KEY) + clients[i];
      clientConfig.setGroup(groupKey);
      info.title             = clientConfig.readEntry("Title", clients[i]);   // read app title (default = app name)
      info.dcopObject        = clientConfig.readEntry("DCOP object");
      info.commandLineNotify = clientConfig.readBoolEntry("Command line notify", false);
      info.displayCalName    = clientConfig.readBoolEntry("Display calendar names", true);
      info.menuIndex = 0;
      mClients.insert(clients[i], info);

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
  kdDebug() << "AlarmDaemon::checkAlarms(ADCalendar*)"<<endl;
  bool korgAlarms = false;
  if (mDocker->alarmsOn()  &&  cal->loaded()  &&  cal->enabled())
  {
    QList<Event> alarmEvents;
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
            if (!cal->eventHandled(eventID))
            {
              if (notifyEvent(cal, eventID))
                cal->setEventHandled(eventID);
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
 * Send a DCOP message to a client application telling it that an alarm
 * should now be handled.
 * Reply = false if the event should be held pending until the client
 *         application can be started.
 */
bool AlarmDaemon::notifyEvent(const ADCalendar* calendar, const QString& eventID)
{
  kdDebug() << "AlarmDaemon::notifyEvent(): " << eventID << endl;
  if (calendar)
  {
    const ClientInfo* client = getClientInfo(calendar->appName());
    if (!client)
      kdDebug() << "AlarmDaemon::notifyEvent(): unknown client" << endl;
    else
    {
      if (!kapp->dcopClient()->isApplicationRegistered(static_cast<const char*>(calendar->appName())))
      {
        // The client application is not running, so start it
        if (!isSessionStarted())
        {
          // Don't start the client application if the session manager is still
          // starting the session, since if we start the client before the
          // session manager does, a KUniqueApplication client will not then be
          // able to restore its session.
          return false;
        }
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
          return true;
        }
        system(execStr.latin1());
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

/*
 * Check whether session startup is complete.
 * Ideally this would be done using a signal from ksmserver, but until such a
 * signal is available, we can check whether ksplash is still running.
 */
bool AlarmDaemon::isSessionStarted()
{
  if (!mSessionStarted)
    checkIfSessionStarted();
  return mSessionStarted;
}

/*
 * Called by the timer to check whether session startup is complete.
 * If so, it notifies clients of any pending alarms.
 */
void AlarmDaemon::checkIfSessionStarted()
{
  if (!kapp->dcopClient()->isApplicationRegistered("ksplash"))
  {
    // Session startup has now completed. Cancel the timer.
    mSessionStarted = true;
    delete mSessionStartTimer;
    mSessionStartTimer = 0L;

    // Notify clients of pending alarms.
    for (ADCalendar* cal = mCalendars.first();  cal;  cal = mCalendars.next())
    {
      if (cal->actionType() == ADCalendar::KALARM)
      {
        QString eventID;
        while (cal->getEventPending(eventID))
        {
          notifyEvent(cal, eventID);
          cal->setEventHandled(eventID);
        }
      }
    }
  }
  else if (!mSessionStartTimer)
  {
    // Need to wait for session startup to complete.
    // Start a 1-second timer.
    mSessionStartTimer = new QTimer(this);
    connect(mSessionStartTimer, SIGNAL(timeout()), SLOT(checkIfSessionStarted()));
    mSessionStartTimer->start(1000);
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
const ClientInfo* AlarmDaemon::getClientInfo(const QString& appName)
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

ClientInfo::ClientInfo(const QString& titl, const QString& dcopObj, bool cmdLine, bool disp, int menuindex)
  : title(titl),
    dcopObject(dcopObj),
    menuIndex(menuindex),
    commandLineNotify(cmdLine),
    displayCalName(disp)
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
      kdDebug() << "Error loading calendar file '" << tmpFile << "'" << endl;
    else
    {
      // Remove all now non-existent events from the handled list
      for (EventsMap::Iterator it = eventsHandled_.begin();  it != eventsHandled_.end();  )
      {
        if (it.data() == urlString_  &&  !getEvent(it.key()))
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
    KMessageBox::error(0L, i18n("Cannot download calendar from %1.").arg(url.prettyURL()));
  return loaded_;
}

/*
 * Check whether the event with the given ID has already been handled.
 */
bool ADCalendar::eventHandled(const QString& ID)
{
  return eventsHandled_.contains(ID);
}

/*
 * Remember that the event with the given ID has been handled.
 */
void ADCalendar::setEventHandled(const QString& ID)
{
  if (!eventsHandled_.contains(ID))
    eventsHandled_.insert(ID, urlString_);
}

/*
 * Clear all memory of events handled for the specified calendar.
 */
void ADCalendar::clearEventsHandled(const QString& calendarURL)
{
  for (EventsMap::Iterator it = eventsHandled_.begin();  it != eventsHandled_.end();  )
  {
    if (it.data() == calendarURL)
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
    eventsPending_.append(&ID);
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
