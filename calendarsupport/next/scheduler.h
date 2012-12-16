/*
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
#ifndef CALENDARSUPPORT_SCHEDULER_H
#define CALENDARSUPPORT_SCHEDULER_H

#include "calendarsupport_export.h"
#include "nepomukcalendar.h"

#include <KCalCore/ScheduleMessage>
#include <KCalCore/IncidenceBase>
#include <KCalCore/Calendar>

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QObject>

/**
   Most scheduler methods are async. They return immediately a unique identifier (a CallId) and
   to the work in background.
   Use the CallId to match the call with the result signal( CallId, ResultCode );
*/
typedef int CallId;

namespace KCalCore {
  class FreeBusyCache;
}

namespace CalendarSupport {
  class IncidenceChanger2;
/**
  This class provides an encapsulation of iTIP transactions (RFC 2446).
  It is an abstract base class for inheritance by implementations of the
  iTIP scheme like iMIP or iRIP.
*/
class CALENDARSUPPORT_EXPORT Scheduler : public QObject
{
  Q_OBJECT
  public:

    enum ResultCode {
      ResultCodeSuccess,
      ResultCodeDifferentIncidenceTypes,
      ResultCodeNewIncidenceTooOld,
      ResultCodeUnknownStatus,
      ResultCodeInvalidIncidence,
      ResultCodeNotUpdate,
      ResultCodeSaveFreeBusyError,
      ResultCodeIncidenceOrAttendeeNotFound,
      ResultCodeIncidenceNotFound,
      ResultCodeErrorDeletingIncidence,
      ResultCodeErrorCreatingIncidence,
      ResultCodeErrorUpdatingIncidence,
      ResultCodeErrorSendingEmail
    };

    /**
      Creates a scheduler for calendar specified as argument.
    */
    Scheduler( const CalendarSupport::NepomukCalendar::Ptr &calendar,
               CalendarSupport::IncidenceChanger2 *changer );

    virtual ~Scheduler();

    /**
      iTIP publish action
    */
    virtual CallId publish( const KCalCore::IncidenceBase::Ptr &incidence,
                            const QString &recipients ) = 0;
    /**
      Performs iTIP transaction on incidence. The method is specified as the
      method argument and can be any valid iTIP method.

      @param incidence the incidence for the transaction.
      @param method the iTIP transaction method to use.
    */
    virtual CallId performTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                       KCalCore::iTIPMethod method ) = 0;

    /**
      Performs iTIP transaction on incidence to specified recipient(s).
      The method is specified as the method argumanet and can be any valid iTIP method.

      @param incidence the incidence for the transaction.
      @param method the iTIP transaction method to use.
      @param recipients the receipients of the transaction.
    */
    virtual CallId performTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                       KCalCore::iTIPMethod method, const QString &recipients ) = 0;

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
    CallId acceptTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                              KCalCore::iTIPMethod method,
                              KCalCore::ScheduleMessage::Status status,
                              const QString &email = QString() );

    virtual bool deleteTransaction( const QString &uid );

    /**
      Returns the directory where the free-busy information is stored.
    */
    virtual QString freeBusyDir() const = 0;

    /**
      Sets the free/busy cache used to store free/busy information.
    */
    void setFreeBusyCache( KCalCore::FreeBusyCache * );

    /**
      Returns the free/busy cache.
    */
    KCalCore::FreeBusyCache *freeBusyCache() const;

  protected:

    // This signal sis delayed because it can't be emitted before "return callId",
    // otherwise the caller would not know the callId that was being sent in the signal
    void emitOperationFinished( CallId, ResultCode, const QString & );

    CallId acceptPublish( const KCalCore::IncidenceBase::Ptr &,
                          KCalCore::ScheduleMessage::Status status,
                          KCalCore::iTIPMethod method );

    CallId acceptRequest( const KCalCore::IncidenceBase::Ptr &,
                          KCalCore::ScheduleMessage::Status status,
                          const QString &email );

    CallId acceptAdd( const KCalCore::IncidenceBase::Ptr &,
                      KCalCore::ScheduleMessage::Status status );

    CallId acceptCancel( const KCalCore::IncidenceBase::Ptr &,
                         KCalCore::ScheduleMessage::Status status,
                         const QString &attendee );

    CallId acceptDeclineCounter( const KCalCore::IncidenceBase::Ptr &,
                                 KCalCore::ScheduleMessage::Status status );

    CallId acceptReply( const KCalCore::IncidenceBase::Ptr &,
                        KCalCore::ScheduleMessage::Status status,
                        KCalCore::iTIPMethod method );

    CallId acceptRefresh( const KCalCore::IncidenceBase::Ptr &,
                          KCalCore::ScheduleMessage::Status status );

    CallId acceptCounter( const KCalCore::IncidenceBase::Ptr &,
                          KCalCore::ScheduleMessage::Status status );

    CallId acceptFreeBusy( const KCalCore::IncidenceBase::Ptr &, KCalCore::iTIPMethod method );

    NepomukCalendar::Ptr calendar() const;

    IncidenceChanger2 * changer() const;

    CallId nextCallId();

  Q_SIGNALS:
    void acceptTransactionFinished( CallId callId, ResultCode code, const QString &errorMessage );
    void performTransactionFinished( CallId callId, ResultCode code, const QString &errorMessage );
    void publishFinished( CallId callId, ResultCode code, const QString &errorMessage );

  private:
    Q_DISABLE_COPY( Scheduler )
    class Private;
    Private *const d;
};

}

#endif
