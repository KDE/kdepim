// $Id$
//
// IMIPScheduler - iMIP implementation of iTIP methods
//

#include "event.h"
#include "icalformat.h"

#include "imipscheduler.h"

using namespace KCal;

IMIPScheduler::IMIPScheduler(Calendar *calendar)
  : Scheduler(calendar)
{
}

IMIPScheduler::~IMIPScheduler()
{
}

bool IMIPScheduler::publish (Event *incidence,const QString &recipients)
{
  return false;
}

bool IMIPScheduler::performTransaction(Event *incidence,Method method)
{
  mFormat->createScheduleMessage(incidence,method);

  return false;
}

QPtrList<ScheduleMessage> IMIPScheduler::retrieveTransactions()
{
  QPtrList<ScheduleMessage> messageList;

  return messageList;
}
