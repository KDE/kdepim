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

#include <kcal/calendarresources.h>

/**
 * @file stdcalendar.h
 * Provides a class for Calendar Resources.
 */

/**
 * @brief
 * Class for Calendar resources.
 * @author Cornelius Schumacher
 * @author Allen Winter
 */
class StdCalendar : public KCal::CalendarResources
{
  public:
    /**
     * Default Constructor
     */
    StdCalendar();

    /**
     * Creates a calendar given a @p fileName and a resource.
     *
     * @param fileName is a filename containing a calendar.
     * @param resName is the the name of a resource.
     */
    StdCalendar( const QString &fileName, const QString &resName );

    /**
     * Destructor
     */
    ~StdCalendar();

    /**
     * Adds a resource to the calendar given a @p fileName and a resource.
     *
     * @param fileName is a filename containing a calendar.
     * @param resName is the the name of a resource.
     */
    void addFileResource( const QString &fileName, const QString &resName );

  private:
    //@cond PRIVATE
    KCal::CalendarResourceManager *mManager;
    //@endcond
};

#endif
