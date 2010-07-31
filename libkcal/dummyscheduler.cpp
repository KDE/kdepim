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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

//
// DummyScheduler - iMIP implementation of iTIP methods
//

#include <tqfile.h>
#include <tqtextstream.h>

#include <kdebug.h>
#include <kstandarddirs.h>

#include "event.h"
#include "icalformat.h"

#include "dummyscheduler.h"

using namespace KCal;

DummyScheduler::DummyScheduler(Calendar *calendar)
  : Scheduler(calendar)
{
}

DummyScheduler::~DummyScheduler()
{
}

bool DummyScheduler::publish (IncidenceBase *incidence,const TQString &/*recipients*/)
{
  TQString messageText = mFormat->createScheduleMessage(incidence,
                                                       Scheduler::Publish);

  return saveMessage(messageText);
}

bool DummyScheduler::performTransaction(IncidenceBase *incidence,Method method,const TQString &/*recipients*/)
{
  TQString messageText = mFormat->createScheduleMessage(incidence,method);

  return saveMessage(messageText);
}

bool DummyScheduler::performTransaction(IncidenceBase *incidence,Method method)
{
  TQString messageText = mFormat->createScheduleMessage(incidence,method);

  return saveMessage(messageText);
}

bool DummyScheduler::saveMessage(const TQString &message)
{
  TQFile f("dummyscheduler.store");
  if (f.open(IO_WriteOnly | IO_Append)) {
    TQTextStream t(&f);
    t << message << endl;
    f.close();
    return true;
  } else {
    return false;
  }
}

TQPtrList<ScheduleMessage> DummyScheduler::retrieveTransactions()
{
  TQPtrList<ScheduleMessage> messageList;

  TQFile f("dummyscheduler.store");
  if (!f.open(IO_ReadOnly)) {
    kdDebug(5800) << "DummyScheduler::retrieveTransactions(): Can't open file"
              << endl;
  } else {
    TQTextStream t(&f);
    TQString messageString;
    TQString messageLine = t.readLine();
    while (!messageLine.isNull()) {
//      kdDebug(5800) << "++++++++" << messageLine << endl;
      messageString += messageLine + "\n";
      if (messageLine.find("END:VCALENDAR") >= 0) {
        kdDebug(5800) << "---------------" << messageString << endl;
        ScheduleMessage *message = mFormat->parseScheduleMessage(mCalendar,
                                                                 messageString);
        kdDebug(5800) << "--Parsed" << endl;
        if (message) {
          messageList.append(message);
        } else {
          TQString errorMessage;
          if (mFormat->exception()) {
            errorMessage = mFormat->exception()->message();
          }
          kdDebug(5800) << "DummyScheduler::retrieveTransactions() Error parsing "
                       "message: " << errorMessage << endl;
        }
        messageString="";
      }
      messageLine = t.readLine();
    }
    f.close();
  }

  return messageList;
}

TQString DummyScheduler::freeBusyDir()
{
  // the dummy scheduler should never handle freebusy stuff - so it's hardcoded
  return TQString("");
}
