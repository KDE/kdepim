/*
    This file is part of libkcal.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef IMIPSCHEDULER_H
#define IMIPSCHEDULER_H
// $Id$
//
// iMIP implementation of iTIP methods
//

#include <qptrlist.h>

#include "scheduler.h"

namespace KCal {

/*
  This class implements the iTIP interface using the email interface specified
  as iMIP.
*/
class IMIPScheduler : public Scheduler {
  public:
    IMIPScheduler(Calendar *);
    virtual ~IMIPScheduler();
    
    bool publish (Event *incidence,const QString &recipients);
    bool performTransaction(Event *incidence,Method method);
    QPtrList<ScheduleMessage> retrieveTransactions();
};

}

#endif  // IMIPSCHEDULER_H

