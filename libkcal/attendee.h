#ifndef _ATTENDEE_H
#define _ATTENDEE_H
// $Id$
//
// Attendee class

#include <qstring.h>

namespace KCal {

/**
  This class represents information related to an attendee of an event.
*/
// TODO: remove const char * functions,
class Attendee
{
  public:
    enum { NEEDS_ACTION = 0, ACCEPTED = 1, SENT = 2, TENTATIVE = 3,
	   CONFIRMED = 4, DECLINED = 5, COMPLETED = 6, DELEGATED = 7 };

    /**
      Create Attendee.
      
      @param n Name
      @param e Email address
      @param rsvp Request for reply
      @param s Status (see enum for list)
      @param r Role
    */
    Attendee(const QString& n, const QString & e = QString::null,
             bool rsvp=FALSE, int s = NEEDS_ACTION, int r = 0);
//    Attendee(const Attendee &);
    /** Destruct Attendee */
    virtual ~Attendee();

    /** Set Attendee name. That is the full real name. */
    void setName(const QString &n) { mName = n; }
    /** Return real name of Attendee. */
    QString name() const { return mName; }

    /** Set email address of Attendee. */
    void setEmail(const QString e) { mEmail = e; }
    /** Return email address of Attendee. */
    QString email() const { return mEmail; }

    /** Set role of Attendee. List of roles still has to be documented. */
    void setRole(int r) { mRole = r; }
    /** Return role of Attendee. */
    int role() const { return mRole; }
    /** Return role as clear text string */
    QString roleStr() const;

    /** Set status. See enum for definitions of possible values */
    void setStatus(int s) { mStatus = s; }
    /** \deprecated Set status from human-readable string. */
    void setStatus(const char *s);
    /** Return status. */
    int status() const { return mStatus; }
    /** Return status as human-readable string. */
    QString statusStr() const;

    /** Set if Attendee is asked to reply. */
    void setRSVP(bool r) { mRSVP = r; }
    /** Return, if Attendee is asked to reply. */
    bool RSVP() const { return mRSVP; }

  private:
    bool mRSVP;
    int mRole;
    int mStatus;
    QString mName;
    QString mEmail;

    // used to tell whether we have need to mail this person or not.
    bool mFlag;
};

}

#endif
