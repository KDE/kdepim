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

#include "adcalendarbase.h"

ADCalendarBase::EventsMap ADCalendarBase::eventsHandled_;

ADCalendarBase::ADCalendarBase(const QString& url, const QString& appname, Type type)
  : urlString_(url),
    appName_(appname),
    actionType_(type),
    loaded_(false)
{
}

/*
 * Load the calendar file.
 */
bool ADCalendarBase::loadFile_(const QString& appName)
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
      kdDebug() << "ADCalendarBase::loadFile_(): Error loading calendar file '" << tmpFile << "'\n";
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
