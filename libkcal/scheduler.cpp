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

// $Id$

#include <klocale.h>

#include "event.h"
#include "icalformat.h"
#include "calendar.h"

#include "scheduler.h"

using namespace KCal;

ScheduleMessage::ScheduleMessage(Incidence *event,int method,ScheduleMessage::Status status)
{
  mEvent = event;
  mMethod = method;
  mStatus = status;
}

QString ScheduleMessage::statusName(ScheduleMessage::Status status)
{
  switch (status) {
    case PublishNew:
      return i18n("Publish");
    case Obsolete:
      return i18n("Obsolete");
    case RequestNew:
      return i18n("New Request");
    case RequestUpdate:
      return i18n("Updated Request");
    default:
      return i18n("Unknown Status: %1").arg(QString::number(status));
  }
}

Scheduler::Scheduler(Calendar *calendar)
{
  mCalendar = calendar;
  mFormat = mCalendar->iCalFormat();
}

Scheduler::~Scheduler()
{
}

bool Scheduler::acceptTransaction(Incidence *incidence,ScheduleMessage::Status status)
{
  switch (status) {
    case ScheduleMessage::PublishNew:
      if (!mCalendar->getEvent(incidence->VUID())) {
        mCalendar->addIncidence(incidence);
      }
      return true;
    case ScheduleMessage::Obsolete:
      return true;
    case ScheduleMessage::RequestNew:
      mCalendar->addIncidence(incidence);
      return true;
    default:
      return false;
  }
}

QString Scheduler::methodName(Method method)
{
  switch (method) {
    case Publish:
      return i18n("Publish");
    case Request:
      return i18n("Request");
    case Refresh:
      return i18n("Refresh");
    case Cancel:
      return i18n("Cancel");
    case Add:
      return i18n("Add");
    case Reply:
      return i18n("Reply");
    case Counter:
      return i18n("Counter");
    case Declinecounter:
      return i18n("Decline Counter");
    default:
      return i18n("Unknown");
  }
}
