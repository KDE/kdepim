#ifndef _ATTENDEE_H
#define _ATTENDEE_H
// $Id$
//
// Attendee class

#include <qstring.h>

#include "person.h"

namespace KCal {

/**
  This class represents information related to an attendee of an event.
*/
class Attendee : public Person
{
  public:
    enum PartStat { NeedsAction, Accepted, Declined, Tentative,
                    Delegated, Completed, InProcess };
    enum Role { ReqParticipant, OptParticipant, NonParticipant, Chair };
  
    /**
      Create Attendee.
      
      @param name Name
      @param email Email address
      @param rsvp Request for reply
      @param status Status (see enum for list)
      @param role Role
    */
    Attendee(const QString& name, const QString &email,
             bool rsvp=false, PartStat status=NeedsAction,
             Role role=ReqParticipant);
//    Attendee(const Attendee &);
    /** Destruct Attendee */
    virtual ~Attendee();

    /** Set role of Attendee. List of roles still has to be documented. */
    void setRole( Role );
    /** Return role of Attendee. */
    Role role() const;
    /** Return role as clear text string */
    QString roleStr() const;
    static QString roleName( Role );
    static QStringList roleList();

    /** Set status. See enum for definitions of possible values */
    void setStatus(PartStat s);
    /** Return status. */
    PartStat status() const;
    /** Return status as human-readable string. */
    QString statusStr() const;
    static QString statusName( PartStat );
    static QStringList statusList();

    /** Set if Attendee is asked to reply. */
    void setRSVP(bool r) { mRSVP = r; }
    /** Return, if Attendee is asked to reply. */
    bool RSVP() const { return mRSVP; }

  private:
    bool mRSVP;
    Role mRole;
    PartStat mStatus;

    // used to tell whether we have need to mail this person or not.
    bool mFlag;
};

}

#endif
