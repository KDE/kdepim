#ifndef EVENT_H
#define EVENT_H
// $Id$
//
// Event component, representing a VEVENT object
//

#include "incidence.h"

namespace KCal {

/**
  This class provides an Event in the sense of RFC2445.
*/
class Event : public Incidence
{
  public:
    Event();
    Event(const Event &);
    ~Event();

    Event *clone();
  
    /** for setting an event's ending date/time with a QDateTime. */
    void setDtEnd(const QDateTime &dtEnd);
    /** returns an event's ending date/time as a QDateTime. */
    const QDateTime &dtEnd() const;
    /** returns an event's end time as a string formatted according to the
     users locale settings */
    QString dtEndTimeStr() const;
    /** returns an event's end date as a string formatted according to the
     users locale settings */
    QString dtEndDateStr(bool shortfmt=true) const;
    /** returns an event's end date and time as a string formatted according
     to the users locale settings */
    QString dtEndStr() const;

    /** Return true if the event spans multiple days, otherwise return false. */
    bool isMultiDay() const;

    /** set the event's time transparency level. */
    void setTransparency(int transparency);
    /** get the event's time transparency level. */
    int transparency() const;

  private:
    bool accept(IncidenceVisitor &v) { return v.visit(this); }

    QDateTime mDtEnd;
    int mTransparency;
};

}

#endif
