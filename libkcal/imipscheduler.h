/*
    This file is part of libkcal.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KCAL_IMIPSCHEDULER_H
#define KCAL_IMIPSCHEDULER_H

#include <qptrlist.h>
#include "libkcal_export.h"

#include "scheduler.h"

namespace KCal {

/*
  \internal

  This class implements the iTIP interface using the email interface specified
  as iMIP.
*/
class KDE_EXPORT IMIPScheduler : public Scheduler
{
  public:
    IMIPScheduler( Calendar * );
    virtual ~IMIPScheduler();
    
    bool publish (IncidenceBase *incidence,const QString &recipients);
    bool performTransaction(IncidenceBase *incidence,Method method);
    QPtrList<ScheduleMessage> retrieveTransactions();

  private:
    class Private;
    Private *d;
};

}

#endif

