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
#ifndef KCAL_SCHEDULER_H
#define KCAL_SCHEDULER_H

#include <qstring.h>
#include <qptrlist.h>

#include "libkcal_export.h"

namespace KCal {

class IncidenceBase;
class Event;
class Calendar;
class ICalFormat;
class FreeBusyCache;

/**
  This class provides an encapsulation of a scheduling message. It associates an
  incidence with a method and status information. This class is used by the
  Scheduler class.

  @short A Scheduling message
*/
class ScheduleMessage
{
  public:
    /**
      Message status.
    */
    enum Status { PublishNew, PublishUpdate, Obsolete, RequestNew,
                  RequestUpdate, Unknown };
  
    /**
      Create a scheduling message with method as defined in Scheduler::Method
      and a status.
    */
    ScheduleMessage( IncidenceBase *, int method, Status status );
    ~ScheduleMessage() {};
    
    /**
      Return event associated with this message.
    */
    IncidenceBase *event() { return mIncidence; }
    /**
      Return iTIP method associated with this message.
    */
    int method() { return mMethod; }
    /**
      Return status of this message.
    */
    Status status() { return mStatus; }
    /**
      Return error message if there is any.
    */
    QString error() { return mError; }

    /**
      Return a human-readable name for an iTIP message status.
    */
    static QString statusName( Status status );

  private:
    IncidenceBase *mIncidence;
    int mMethod;
    Status mStatus;
    QString mError;

    class Private;
    Private *d;
};

/**
  This class provides an encapsulation of iTIP transactions. It is an abstract
  base class for inheritance by implementations of the iTIP scheme like iMIP or
  iRIP.
*/
class LIBKCAL_EXPORT Scheduler
{
  public:
    /**
      iTIP methods.
    */
    enum Method { Publish,Request,Refresh,Cancel,Add,Reply,Counter,
                  Declinecounter,NoMethod };
  
    /**
      Create scheduler for calendar specified as argument.
    */
    Scheduler( Calendar *calendar );
    virtual ~Scheduler();
    
    /**
      iTIP publish action
    */
    virtual bool publish( IncidenceBase *incidence,
                          const QString &recipients ) = 0;
    /**
      Perform iTIP transaction on incidence. The method is specified as the
      method argument and can be any valid iTIP method.
    */
    virtual bool performTransaction( IncidenceBase *incidence,
                                     Method method ) = 0;
    /**
      Perform iTIP transaction on incidence to specified recipient(s). The 
      method is specified as the method argumanet and can be any valid iTIP 
      method.
    */
    virtual bool performTransaction( IncidenceBase *incidence, Method method,
                                     const QString &recipients ) = 0;
    /**
      Retrieve incoming iTIP transactions.
    */
    virtual QPtrList<ScheduleMessage> retrieveTransactions() = 0;

    /**
      Accept transaction. The incidence argument specifies the iCal compoennt
      on which the transaction acts. The status is the result of processing a
      iTIP message with the current calendar and specifies the action to be
      taken for this incidence.
    */
    bool acceptTransaction( IncidenceBase *, Method method,
                            ScheduleMessage::Status status );

    /**
      Return a machine-readable name for a iTIP method.
    */
    static QString methodName( Method );
    /**
      Return a translated human-readable name for a iTIP method.
    */
    static QString translatedMethodName( Method );

    virtual bool deleteTransaction( IncidenceBase *incidence );
    
    /**
      Returns the directory where the free-busy information is stored.
    */
    virtual QString freeBusyDir() = 0;

    /**
      Set free/busy cache used to store free/busy information.
    */
    void setFreeBusyCache( FreeBusyCache * );
    /**
      Return free/busy cache.
    */
    FreeBusyCache *freeBusyCache() const;

  protected:
    bool acceptPublish( IncidenceBase *, ScheduleMessage::Status status,
                        Method method );
    bool acceptRequest( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptAdd( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptCancel( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptDeclineCounter( IncidenceBase *,
                               ScheduleMessage::Status status );
    bool acceptReply( IncidenceBase *, ScheduleMessage::Status status,
                      Method method );
    bool acceptRefresh( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptCounter( IncidenceBase *, ScheduleMessage::Status status );
    bool acceptFreeBusy( IncidenceBase *, Method method );

    Calendar *mCalendar;
    ICalFormat *mFormat;

  private:
    class Private;
    Private *d;
};

}

#endif
