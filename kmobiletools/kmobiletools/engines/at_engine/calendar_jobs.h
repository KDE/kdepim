/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef CALENDAR_JOBS_H
#define CALENDAR_JOBS_H

#include "at_jobs.h"

#include <kcal/event.h>

class FetchCalendar : public kmobiletoolsATJob
{
    Q_OBJECT
    public:
      FetchCalendar(KMobileTools::Job *pjob, KMobileTools::SerialManager *device, kmobiletoolsAT_engine* parent = 0 );
      JobType type()            { return KMobileTools::Job::fetchKCal; }
        KCal::Event::List *calendar() { return p_calendar; }
    protected:
        void run ();
        KCal::Event::List *p_calendar;
        void fetchMotorolaCalendar();
    signals:
};

#endif
