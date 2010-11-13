/*
  This file is part of the calendarsupport library.

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#ifndef CALENDARSUPPORT_SCHEDULER_H
#define CALENDARSUPPORT_SCHEDULER_H

#include "calendarsupport_export.h"

#include <KCalCore/ScheduleMessage>
#include <KCalCore/IncidenceBase>
#include <KCalCore/Calendar>

#include <QtCore/QString>
#include <QtCore/QList>

namespace KCalCore {
  class ICalFormat;
  class FreeBusyCache;
};

namespace CalendarSupport {
  class IncidenceChanger2;
/**
  This class provides an encapsulation of iTIP transactions (RFC 2446).
  It is an abstract base class for inheritance by implementations of the
  iTIP scheme like iMIP or iRIP.
*/
class CALENDARSUPPORT_EXPORT Scheduler
{
  public:
    /**
      Creates a scheduler for calendar specified as argument.
    */
    Scheduler( const  KCalCore::Calendar::Ptr &calendar, CalendarSupport::IncidenceChanger2 *changer );
    virtual ~Scheduler();

    /**
      iTIP publish action
    */
    virtual bool publish( const KCalCore::IncidenceBase::Ptr &incidence,
                          const QString &recipients ) = 0;
    /**
      Performs iTIP transaction on incidence. The method is specified as the
      method argument and can be any valid iTIP method.

      @param incidence the incidence for the transaction.
      @param method the iTIP transaction method to use.
    */
    virtual bool performTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                     KCalCore::iTIPMethod method ) = 0;

    /**
      Performs iTIP transaction on incidence to specified recipient(s).
      The method is specified as the method argumanet and can be any valid iTIP method.

      @param incidence the incidence for the transaction.
      @param method the iTIP transaction method to use.
      @param recipients the receipients of the transaction.
    */
    virtual bool performTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                     KCalCore::iTIPMethod method, const QString &recipients ) = 0;

    /**
      Retrieves incoming iTIP transactions.
    */
     //KDAB_TODO PTR
    virtual QList<KCalCore::ScheduleMessage*> retrieveTransactions() = 0;

    /**
      Accepts the transaction. The incidence argument specifies the iCal
      component on which the transaction acts. The status is the result of
      processing a iTIP message with the current calendar and specifies the
      action to be taken for this incidence.

      @param incidence the incidence for the transaction.
      @param method iTIP transaction method to check.
      @param status scheduling status.
      @param email the email address of the person for whom this
      transaction is to be performed.
    */
    bool acceptTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                            KCalCore::iTIPMethod method,
                            KCalCore::ScheduleMessage::Status status,
                            const QString &email = QString() );

    virtual bool deleteTransaction( const KCalCore::IncidenceBase::Ptr &incidence );

    /**
      Returns the directory where the free-busy information is stored.
    */
    virtual QString freeBusyDir() = 0;

    /**
      Sets the free/busy cache used to store free/busy information.
    */
    void setFreeBusyCache( KCalCore::FreeBusyCache * );

    /**
      Returns the free/busy cache.
    */
    KCalCore::FreeBusyCache *freeBusyCache() const;

  protected:
    bool acceptPublish( const KCalCore::IncidenceBase::Ptr &,
                        KCalCore::ScheduleMessage::Status status,
                        KCalCore::iTIPMethod method );

    bool acceptRequest( const KCalCore::IncidenceBase::Ptr &,
                        KCalCore::ScheduleMessage::Status status,
                        const QString &email );

    bool acceptAdd( const KCalCore::IncidenceBase::Ptr &,
                    KCalCore::ScheduleMessage::Status status );

    bool acceptCancel( const KCalCore::IncidenceBase::Ptr &,
                       KCalCore::ScheduleMessage::Status status,
                       const QString &attendee );

    bool acceptDeclineCounter( const KCalCore::IncidenceBase::Ptr &,
                               KCalCore::ScheduleMessage::Status status );

    bool acceptReply( const KCalCore::IncidenceBase::Ptr &,
                      KCalCore::ScheduleMessage::Status status,
                      KCalCore::iTIPMethod method );

    bool acceptRefresh( const KCalCore::IncidenceBase::Ptr &,
                        KCalCore::ScheduleMessage::Status status );

    bool acceptCounter( const KCalCore::IncidenceBase::Ptr &,
                        KCalCore::ScheduleMessage::Status status );

    bool acceptFreeBusy( const KCalCore::IncidenceBase::Ptr &, KCalCore::iTIPMethod method );

  private:
    Q_DISABLE_COPY( Scheduler )
    struct Private;
    Private *const d;
};

}

#endif
