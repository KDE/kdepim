/*
    Calendar and client access for KDE Alarm Daemon.

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

//#include <unistd.h>
//#include <stdlib.h>

#include <qdatetime.h>

#include <kapp.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kio/netaccess.h>

#include <libkcal/calendarlocal.h>

#include "adconfigdatabase.h"

// The config file containing client and calendar information
#define CLIENT_DATA_FILE "clients"

// Config file key strings
const QString ADConfigDataBase::CLIENT_KEY("Client_");
const QString ADConfigDataBase::CLIENTS_KEY("Clients");
const QString ADConfigDataBase::GUI_KEY("Gui_");
const QString ADConfigDataBase::GUIS_KEY("Guis");
const QString ADConfigDataBase::CALENDAR_KEY("Calendar");


ADConfigDataBase::ADConfigDataBase(bool daemon)
  : mIsAlarmDaemon(daemon)
{
  mCalendars.setAutoDelete(true);
}

/*
 * Read all client applications from the client data file and store them in the client list.
 * Open all calendar files listed in the client data file and start monitoring them.
 * Calendar files are monitored until their client application registers, upon which
 * monitoring ceases until the client application tell the daemon to monitor it.
 * Reply = updated Clients option in main Alarm Daemon config file.
 */
QString ADConfigDataBase::readConfigData(bool sessionStarting, bool& deletedClients, bool& deletedCalendars,
                                         ADCalendarBaseFactory *calFactory)
{
  kdDebug() << "ADConfigDataBase::readConfigData()" << endl;
  deletedClients   = false;
  deletedCalendars = false;
  if (mClientDataFile.isEmpty())
  {
    if (mIsAlarmDaemon)
      mClientDataFile = locateLocal("appdata", QString(CLIENT_DATA_FILE));
    else
      mClientDataFile = locate("appdata", QString("kalarmd/" CLIENT_DATA_FILE));
  }
  KSimpleConfig clientConfig(mClientDataFile);
  clientConfig.setGroup("General");
  QStringList clients = QStringList::split(',', clientConfig.readEntry(CLIENTS_KEY), true);

  // Delete any clients which are no longer in the config file
  for (ClientMap::Iterator cl = mClients.begin();  cl != mClients.end();  )
  {
    bool found = false;
    for (unsigned int i = 0;  i < clients.count();  ++i)
      if (clients[i] == cl.key())
      {
        found = true;
        break;
      }
    if (!found)
    {
      // This client has disappeared. Remove it and its calendars
      for (ADCalendarBase* cal = mCalendars.first();  cal;  cal = mCalendars.next())
      {
        if (cal->appName() == cl.key())
        {
          mCalendars.remove(cal);
          deletedCalendars = true;
        }
      }
      ClientMap::Iterator c = cl;
      ++cl;                      // prevent iterator becoming invalid with remove()
      mClients.remove(c);
      deletedClients = true;
    }
    else
      ++cl;
  }

  // Update the clients and calendars lists with the new data
  bool writeNewClients = false;
  QString newClients;
  for (unsigned int i = 0;  i < clients.count();  ++i)
  {
    kdDebug() << "ADConfigDataBase::readConfigData(): client: " << clients[i] << endl;
    if (clients[i].isEmpty()
    ||  KStandardDirs::findExe(clients[i]) == QString::null)
    {
      // Null client name, or application doesn't exist
      if (mIsAlarmDaemon)
      {
        if (!clients[i].isEmpty())
          clientConfig.deleteGroup(CLIENT_KEY + clients[i], true);
        writeNewClients = true;
      }
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
      info->notificationType  = (ClientInfo::NotificationType)clientConfig.readNumEntry("Notification type", 0);
      info->displayCalName    = clientConfig.readBoolEntry("Display calendar names", true);
      if (!found)
      {
        info->waitForRegistration = sessionStarting;
        mClients.insert(clients[i], newinfo);
      }

      // Get the client's calendar files
#warning "check if any calendars have been deleted"
      int len = CALENDAR_KEY.length();
      QMap<QString, QString> entries = clientConfig.entryMap(groupKey);
      for (QMap<QString, QString>::ConstIterator it = entries.begin();  it != entries.end();  ++it)
      {
        kdDebug() << "ADConfigDataBase::readConfigData(): " << it.key() << "=" << it.data() << endl;
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
              ADCalendarBase* cal = getCalendar(calname);
              if (cal)
              {
                // The calendar is already in the client's list, so remove
                // this redundant client data file entry.
                if (mIsAlarmDaemon)
                  deleteConfigCalendar(cal);
              }
              else
              {
                // Add the calendar to the client's list
                cal = calFactory->create(calname, clients[i],
                             static_cast<ADCalendarBase::Type>(it.data().left(comma).toInt()));
                mCalendars.append(cal);
                kdDebug() << "ADConfigDataBase::readConfigData(): calendar " << cal->urlString() << endl;
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
  return writeNewClients ? newClients : QString::null;
}

void ADConfigDataBase::deleteConfigCalendar(const ADCalendarBase*)
{
}

/* Return the ClientInfo structure for the specified client application */
const ClientInfo* ADConfigDataBase::getClientInfo(const QString& appName) const
{
  if (!appName.isEmpty())
  {
    ClientMap::ConstIterator c = mClients.find(appName);
    if (c != mClients.end())
      return &c.data();
  }
  return 0L;
}

/* Return the ADCalendarBase structure for the specified full calendar URL */
ADCalendarBase *ADConfigDataBase::getCalendar(const QString& calendarURL)
{
  if (!calendarURL.isEmpty())
  {
    for (ADCalendarBase *cal = mCalendars.first();  cal;  cal = mCalendars.next())
    {
      if (cal->urlString() == calendarURL)
        return cal;
    }
  }
  return 0L;
}

/*
 * Expand a DCOP call parameter URL to a full URL.
 * (We must store full URLs in the calendar data since otherwise
 *  later calls to reload or remove calendars won't necessarily
 *  find a match.)
 */
QString ADConfigDataBase::expandURL(const QString& urlString)
{
  if (urlString.isEmpty())
    return QString();
  return KURL(urlString).url();
}
