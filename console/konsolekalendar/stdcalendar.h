/*
    This file was originally from libkcal, then moved into korganizer.
    This version has been hacked for use by konsolekalendar.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Allen Winter <winter@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KONSOLEKALENDAR_STDCALENDAR_H
#define KONSOLEKALENDAR_STDCALENDAR_H

#include <libkcal/calendarresources.h>

namespace KCal {

class KDE_EXPORT StdCalendar : public KCal::CalendarResources
{
  public:
    StdCalendar();
    StdCalendar( const QString &fileName, const QString &resName );
    ~StdCalendar();

    void addFileResource( const QString &fileName, const QString &resName );

  private:
    KCal::CalendarResourceManager *mManager;
};

}

#endif
