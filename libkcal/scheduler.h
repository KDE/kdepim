// $Id$
#ifndef SCHEDULER_H
#define SCHEDULER_H

// iTIP transactions base class

#include <qstring.h>
#include <qptrlist.h>

namespace KCal {

class Incidence;
class Event;
class Calendar;
class ICalFormat;

/**
  This class provides an encapsulation of a scheduling message. It associates an
  incidence with a method and status information. This class is used by the
  Scheduler class.

  @short A Scheduling message
*/
class ScheduleMessage {
  public:
    /** Message status. */
    enum Status { PublishNew, Obsolete, RequestNew, RequestUpdate, Unknown };
  
    /**
      Create a scheduling message with method as defined in Scheduler::Method
      and a status.
    */
    ScheduleMessage(Incidence *,int method,Status status);
    ~ScheduleMessage() {};
    
    /** Return event associated with this message. */
    Incidence *event() { return mEvent; }
    /** Return iTIP method associated with this message. */
    int method() { return mMethod; }
    /** Return status of this message. */
    Status status() { return mStatus; }
    /** Return error message if there is any. */
    QString error() { return mError; }

    /** Return a human-readable name for an ical message status. */
    static QString statusName(Status status);

  private:
    Incidence *mEvent;
    int mMethod;
    Status mStatus;
    QString mError;
};

/**
  This class provides an encapsulation of iTIP transactions. It is an abstract
  base class for inheritance by implementations of the iTIP scheme like iMIP or
  iRIP.
*/
class Scheduler {
  public:
    /** iTIP methods. */
    enum Method { Publish,Request,Refresh,Cancel,Add,Reply,Counter,
                  Declinecounter,NoMethod };
  
    /** Create scheduler for calendar specified as argument. */
    Scheduler(Calendar *calendar);
    virtual ~Scheduler();
    
    /** iTIP publish action */
    virtual bool publish (Event *incidence,const QString &recipients) = 0;
    /** Perform iTIP transaction on incidence. The method is specified as the
    method argumanet and can be any valid iTIP method. */
    virtual bool performTransaction(Event *incidence,Method method) = 0;
    /** Retrieve incoming iTIP transactions */
    virtual QPtrList<ScheduleMessage> retrieveTransactions() = 0;

    /**
      Accept transaction. The incidence argument specifies the iCal compoennt
      on which the transaction acts. The status is the result of processing a
      iTIP message with the current calendar and specifies the action to be
      taken for this incidence.
    */
    bool acceptTransaction(Incidence *,ScheduleMessage::Status status);

    /** Return a human-readable name for a iTIP method. */
    static QString methodName(Method);

  protected:
    Calendar *mCalendar;
    ICalFormat *mFormat;
};

}

#endif  // SCHEDULER_H
