/*
    KDE Alarm Daemon GUI.

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

#include <kstandarddirs.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kurl.h>
#include <dcopclient.h>
#include <knotifyclient.h>
#include <kdebug.h>

#include "dockwindow.h"
#include "alarmdialog.h"
#include "alarmdaemoniface_stub.h"

#include "alarmgui.h"
#include "alarmgui.moc"

const int DAEMON_TIMER_INTERVAL = 5;    // seconds between checks on daemon status

void ADConfigData::readDaemonData(bool& deletedClients, bool& deletedCalendars)
{
  ADCalendarGuiFactory calFactory;
  readConfigData(false, deletedClients, deletedCalendars, &calFactory);
}

AlarmGui::AlarmGui(QObject *parent, const char *name)
  : QObject(parent, name),
    DCOPObject(name),
    mSuspendTimer(this),
    mDaemonStatusTimer(this),
    mDaemonStatusTimerCount(0),
    mRevisingAlarmDialog(false),
    mDrawAlarmDialog(false)
{
  kdDebug(5900) << "AlarmGui::AlarmGui()" << endl;

  readDaemonConfig();
  bool deletedClients;
  bool deletedCalendars;
  readDaemonData(deletedClients, deletedCalendars);
  checkDefaultClient();

  mDocker = new AlarmDockWindow(this);
  mDocker->show();

  connect(&mDaemonStatusTimer, SIGNAL(timeout()), SLOT(checkDaemonRunning()));
  mDaemonStatusTimer.start(DAEMON_TIMER_INTERVAL*1000);     // check regularly if daemon is running

  mAlarmDialog = new AlarmDialog;
  connect(mAlarmDialog, SIGNAL(suspendSignal(int)), SLOT(suspend(int)));

  setToolTip();

  registerWithDaemon();
}

AlarmGui::~AlarmGui()
{
  delete mDocker;
}

/*
 * DCOP call from the alarm daemon to notify a change.
 */
void AlarmGui::alarmDaemonUpdate(int alarmGuiChangeType,
                                 const QString& calendarURL,
                                 const QCString& appName)
{
  AlarmGuiChangeType changeType = AlarmGuiChangeType(alarmGuiChangeType);
  switch (changeType)
  {
    case CHANGE_STATUS:
      kdDebug(5900) << "AlarmGui::alarmDaemonUpdate(CHANGE_STATUS)\n";
      readDaemonConfig();
      mDocker->setDaemonAutostart(mAutostartDaemon);
      break;
    case CHANGE_CLIENT:
    {
      kdDebug(5900) << "AlarmGui::alarmDaemonUpdate(CHANGE_CLIENT)\n";
      bool deletedClients, deletedCalendars;
      readDaemonData(deletedClients, deletedCalendars);
      checkDefaultClient();
      mDocker->updateMenuClients();
      mDocker->updateMenuCalendars(true);
      setToolTip();
      break;
    }
    default:
    {
      // It must be a calendar-related change
      bool recreateMenu = false;
      ADCalendarBase* cal = getCalendar(expandURL(calendarURL));
      switch (changeType)
      {
        case ADD_CALENDAR:
          // Add a KOrganizer-type calendar
          kdDebug(5900) << "AlarmGui::alarmDaemonUpdate(ADD_CALENDAR)\n";
          if (cal)
          {
            if (cal->actionType() == ADCalendarBase::KORGANIZER)
            {
              removeDialogEvents(cal);
              cal->close();
              cal->loadFile();
            }
          }
          else
          {
            cal = new ADCalendarGui(calendarURL, appName, ADCalendarBase::KORGANIZER);
            mCalendars.append(cal);
            kdDebug(5900) << "AlarmGui::alarmDaemonUpdate(): KORGANIZER calendar added" << endl;
            recreateMenu = true;
          }
          break;
        case ADD_MSG_CALENDAR:
          // Add a KAlarm-type calendar
          kdDebug(5900) << "AlarmGui::alarmDaemonUpdate(ADD_MSG_CALENDAR)\n";
          if (cal)
          {
            if (cal->actionType() == ADCalendarBase::KORGANIZER)
              removeDialogEvents(cal);
            mCalendars.remove(cal);
          }
          cal = new ADCalendarGui(calendarURL, appName, ADCalendarBase::KALARM);
          mCalendars.append(cal);
          recreateMenu = true;
          break;
        default:
          if (!cal)
          {
            kdDebug(5900) << "AlarmGui::alarmDaemonUpdate(): unknown calendar: " << calendarURL << endl;
            return;
          }
          switch (changeType)
          {
            case DELETE_CALENDAR:
              kdDebug(5900) << "AlarmGui::alarmDaemonUpdate(DELETE_CALENDAR)\n";
              removeDialogEvents(cal);
              mCalendars.remove(cal);
              recreateMenu = true;
              break;
            case CALENDAR_UNAVAILABLE:
              // Calendar is not available for monitoring
              kdDebug(5900) << "AlarmGui::alarmDaemonUpdate(CALENDAR_UNAVAILABLE)\n";
              cal->setAvailable( false );
              cal->setEnabled( false );
              break;
            case DISABLE_CALENDAR:
              // Calendar is available for monitoring but is not currently being monitored
              kdDebug(5900) << "AlarmGui::alarmDaemonUpdate(DISABLE_CALENDAR)\n";
              cal->setAvailable( true );
              cal->setEnabled( false );
              break;
            case ENABLE_CALENDAR:
              // Calendar is currently being monitored
              kdDebug(5900) << "AlarmGui::alarmDaemonUpdate(ENABLE_CALENDAR)\n";
              cal->setAvailable( true );
              cal->setEnabled( true );
              break;
            default:
              kdDebug(5900) << "AlarmGui::alarmDaemonUpdate(): unknown change type: " << changeType << endl;
              return;
          }
          break;
      }
      mDocker->updateMenuCalendars(recreateMenu);
      setToolTip();
      break;
    }
  }
}

/*
 * DCOP call from the alarm daemon to notify an event becoming due.
 */
void AlarmGui::handleEvent(const QString& calendarURL, const QString& eventID)
{
  ADCalendarBase* cal = getCalendar(expandURL(calendarURL));
  Event* event = cal->getEvent(eventID);
  mAlarmDialog->appendEvent(cal, event);
}

void AlarmGui::registerWithDaemon()
{
  kdDebug(5900) << "AlarmGui::registerWithDaemon()" << endl;
  AlarmDaemonIface_stub s( DAEMON_APP_NAME, DAEMON_DCOP_OBJECT );
  s.registerGui(kapp->aboutData()->appName(), DCOP_OBJECT_NAME);
}

// Read the Alarm Daemon's config file
void AlarmGui::readDaemonConfig()
{
  if (mDaemonDataFile.isEmpty())
    mDaemonDataFile = locate("config", QString("kalarmdrc"));
  KSimpleConfig kalarmdConfig(mDaemonDataFile, true);
  kalarmdConfig.setGroup("General");
  mAutostartDaemon = kalarmdConfig.readBoolEntry("Autostart", true);
  kdDebug(5900) << "AlarmGui::readDaemonConfig(): " << mDaemonDataFile
                << " auto=" << (int)mAutostartDaemon << endl;
}

/*
 * Check that the default client is in the list of client applications.
 * If not, set it to the first client application and update the client data file.
 */
void AlarmGui::checkDefaultClient()
{
  // Read the default client application
  KConfig* config = kapp->config();
  config->setGroup("General");
  mDefaultClient = config->readEntry("Default Client");

  if (!getClientInfo(mDefaultClient).isValid())
  {
    // Default client isn't in the list of clients.
    // Replace it with the first client in the list.
    if ( mClients.count() > 0 ) {
      mDefaultClient = mClients[ 0 ].appName;
    } else {
      mDefaultClient = "";
    }
    config->writeEntry("Default Client", QString::fromLocal8Bit(mDefaultClient));
    config->sync();
  }
}

void AlarmGui::setDefaultClient(int menuIndex)
{
  ClientList::ConstIterator client;
  for (client = mClients.begin();  client != mClients.end();  ++client)
  {
    if ((*client).menuIndex == menuIndex)
    {
      mDefaultClient = (*client).appName;
      KConfig* config = kapp->config();
      config->setGroup("General");
      config->writeEntry("Default Client",
                         QString::fromLocal8Bit(mDefaultClient));
      config->sync();
    }
  }
}

/* Check whether the alarm daemon is currently running */
bool AlarmGui::isDaemonRunning(bool updateDockWindow)
{
  bool newstatus = kapp->dcopClient()->isApplicationRegistered(static_cast<const char*>("kalarmd"));
  if (!updateDockWindow)
    return newstatus;
  if (newstatus != mDaemonRunning)
  {
//kdDebug(5900) << "AlarmGui::isDaemonRunning(): "<<(int)mDaemonRunning<<"->"<<(int)newstatus<<endl;
    mDaemonRunning = newstatus;
    mDocker->setDaemonStatus(newstatus);
    mDaemonStatusTimer.changeInterval(DAEMON_TIMER_INTERVAL*1000);
    mDaemonStatusTimerCount = 0;
    if (newstatus)
      registerWithDaemon();   // the alarm daemon has started up, so register with it
  }
  return mDaemonRunning;
}

/*
 * Called by a timer to check whether the daemon is running.
 */
void AlarmGui::checkDaemonRunning()
{
  isDaemonRunning();
  if (mDaemonStatusTimerCount > 0  &&  --mDaemonStatusTimerCount <= 0)   // limit how long we check at fast rate
    mDaemonStatusTimer.changeInterval(DAEMON_TIMER_INTERVAL*1000);
}

/* Starts checking at a faster rate whether the daemon is running */
void AlarmGui::setFastDaemonCheck()
{
  mDaemonStatusTimer.start(500);     // check new status every half second
  mDaemonStatusTimerCount = 20;      // don't check at this rate for more than 10 seconds
}

/* Schedule the alarm dialog for redisplay after a specified number of minutes */
void AlarmGui::suspend(int minutes)
{
//  kdDebug(5900) << "AlarmGui::suspend() " << minutes << " minutes" << endl;
  connect(&mSuspendTimer, SIGNAL(timeout()), SLOT(showAlarmDialog()));
  mSuspendTimer.start(1000*60*minutes, true);
}

/* Display the alarm dialog (showing KOrganiser-type events) */
void AlarmGui::showAlarmDialog()
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
void AlarmGui::removeDialogEvents(const Calendar* calendar)
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
    mSuspendTimer.stop();
    mRevisingAlarmDialog = false;
    mDrawAlarmDialog = false;
  }
}

/*
 * Adds the appropriate calendar file name to the panel tool tip.
 */
void AlarmGui::setToolTip()
{
  // Count the number of currently loaded calendars whose names should be displayed
  int nAvailable = 0;
  int nForDisplay = 0;
  ADCalendarBase* firstForDisplay = 0L;
  for (ADCalendarBase* cal = mCalendars.first();  cal;  cal = mCalendars.next())
  {
    if (cal->available())
    {
      ClientInfo c = getClientInfo(cal->appName());
      if (c.isValid()  &&  c.displayCalName  &&  !nForDisplay++) {
        firstForDisplay = cal;
      }
      ++nAvailable;
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
  else if (!nAvailable)
    filename = i18n("No calendar loaded.");
  mDocker->addToolTip(filename);
}
