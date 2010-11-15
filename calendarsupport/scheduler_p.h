/*
  This file is part of the CalendarSupport library.

  Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2010 Sérgio Martins <iamsergio@gmail.com>

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
#ifndef CALENDARSUPPORT_SCHEDULER_P_H
#define CALENDARSUPPORT_SCHEDULER_P_H

#include "nepomukcalendar.h"

#include <Akonadi/Item>
#include <QObject>

//@cond PRIVATE

namespace KCalCore {
  class ICalFormat;
  class FreeBusyCache;
}

namespace CalendarSupport {

class Scheduler;
class IncidenceChanger2;

class Scheduler::Private : public QObject
{
  Q_OBJECT
  public:
    Private( Scheduler *qq,
             const CalendarSupport::NepomukCalendar::Ptr &calendar,
             IncidenceChanger2 *changer );

    ~Private();

    // This signals are delayed because they can't be emitted before "return callId",
    // otherwise the caller would not know the callId that was being sent in the signal
    void emitOperationFinished( CallId, ResultCode, const QString & );

  public Q_SLOTS:
    // to catch incidenceChanger signals
    void createFinished( int changeId,
                         const Akonadi::Item &item,
                         CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                         const QString &errorMessage );

    void deleteFinished( int changeId,
                         const QVector<Akonadi::Item::Id> &itemId,
                         CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                         const QString &errorMessage );

    void modifyFinished( int changeId,
                         const Akonadi::Item &item,
                         CalendarSupport::IncidenceChanger2::ResultCode resultCode,
                         const QString &errorMessage );
  public:
    QHash<CallId,int> mCallIdByChangeId;
    KCalCore::FreeBusyCache *mFreeBusyCache;
    IncidenceChanger2 *mChanger;
    CalendarSupport::NepomukCalendar::Ptr mCalendar;
    KCalCore::ICalFormat *mFormat;
    CallId mLatestCallId;
    Scheduler *q;
};

}
//@endcond

#endif
