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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _ATTENDEE_H
#define _ATTENDEE_H

#include <qstring.h>

#include "listbase.h"
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

    typedef ListBase<Attendee> List;
  
    /**
      Create Attendee.
      
      @param name Name
      @param email Email address
      @param rsvp Request for reply
      @param status Status (see enum for list)
      @param role Role
      @param u the uid for the attendee
    */
    Attendee(const QString& name, const QString &email,
             bool rsvp=false, PartStat status=NeedsAction,
             Role role=ReqParticipant,const QString& u=QString::null);
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

    /** Holds the uid of the attendee, if applicable **/
    QString uid() const;
    void setUid (QString);

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
    QString mUid;

    // used to tell whether we have need to mail this person or not.
    bool mFlag;
};

    bool operator==( const Attendee& a1, const Attendee& a2 );

}

#endif
