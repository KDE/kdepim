/*
    Calendar access for KDE Alarm Daemon.

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kstddirs.h>
#include <ksimpleconfig.h>
#include <kio/netaccess.h>
#include <kdebug.h>

#include "adcalendarbase.h"

ADCalendarBase::EventsMap ADCalendarBase::eventsHandled_;

ADCalendarBase::ADCalendarBase(const QString& url, const QCString& appname, Type type)
  : urlString_(url),
    appName_(appname),
    actionType_(type),
    loaded_(false),
    mUnregistered(false)
{
  KConfig cfg( locate( "config", "korganizerrc" ) );
  cfg.setGroup( "Time & Date" );
  QString tz = cfg.readEntry( "TimeZoneId" );
  kdDebug(5900) << "ADCalendarBase(): tz: " << tz << endl;
  setTimeZoneId( cfg.readEntry( "TimeZoneId" ) );
}

/*
 * Load the calendar file.
 */
bool ADCalendarBase::loadFile_(const QCString& appName)
{
  loaded_ = false;
  KURL url(urlString_);
  QString tmpFile;
  if (KIO::NetAccess::download(url, tmpFile))
  {
    kdDebug(5900) << "--- Downloaded to " << tmpFile << endl;
    loaded_ = load(tmpFile);
    KIO::NetAccess::removeTempFile(tmpFile);
    if (!loaded_)
      kdDebug(5900) << "ADCalendarBase::loadFile_(): Error loading calendar file '" << tmpFile << "'\n";
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
  else if (!appName.isEmpty()) {
#if TODO_handle_download_error
    KMessageBox::error(0L, i18n("Cannot download calendar from\n%1.")
                           .arg(url.prettyURL()), appName);
#endif
  }
  return loaded_;
}

void ADCalendarBase::dump() const
{
  kdDebug(5900) << "  <calendar>" << endl;
  kdDebug(5900) << "    <url>" << urlString() << "</url>" << endl;
  kdDebug(5900) << "    <appname>" << appName() << "</appname>" << endl;
  if ( loaded() ) kdDebug(5900) << "    <loaded/>" << endl;
  kdDebug(5900) << "    <actiontype>" << int(actionType()) << "</actiontype>" << endl;
  if (enabled() ) kdDebug(5900) << "    <enabled/>" << endl;
  else kdDebug(5900) << "    <disabled/>" << endl;
  if (available()) kdDebug(5900) << "    <available/>" << endl;
  kdDebug(5900) << "  </calendar>" << endl;  
}
