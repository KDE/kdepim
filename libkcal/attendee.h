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

#ifndef KCAL_ATTENDEE_H
#define KCAL_ATTENDEE_H

#include <qstring.h>

#include "listbase.h"
#include "person.h"

namespace KCal {

/**
  This class represents information related to an attendee of an event.
*/
class LIBKCAL_EXPORT Attendee : public Person
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
    Attendee( const QString &name, const QString &email,
              bool rsvp = false, PartStat status = NeedsAction,
              Role role = ReqParticipant, const QString &u = QString::null );
    /**
      Destruct Attendee.
    */
    virtual ~Attendee();

    /**
      Set role of Attendee.
    */
    // FIXME: List of roles still has to be documented.
    void setRole( Role );
    
    /**
      Return role of Attendee.
    */
    Role role() const;
    
    /**
      Return role as clear text string.
    */
    QString roleStr() const;
    /**
      Return string represenation of role.
    */
    static QString roleName( Role );
    /**
      Return string representations of all available roles.
    */
    static QStringList roleList();

    /**
      Return unique id of the attendee.
    */
    QString uid() const;
    /**
      Set unique id of attendee.
    */
    void setUid ( const QString & );

    /**
      Set status. See enum for definitions of possible values.
    */
    void setStatus( PartStat s );

    /**
      Return status.
    */
    PartStat status() const;
    
    /**
      Return status as human-readable string.
    */
    QString statusStr() const;
    /**
      Return string representation of attendee status.
    */
    static QString statusName( PartStat );
    /**
      Return string representations of all available attendee status values.
    */
    static QStringList statusList();

    /**
      Set if Attendee is asked to reply.
    */
    void setRSVP( bool r ) { mRSVP = r; }
    /**
      Return, if Attendee is asked to reply.
    */
    bool RSVP() const { return mRSVP; }

  private:
    bool mRSVP;
    Role mRole;
    PartStat mStatus;
    QString mUid;

    class Private;
    Private *d;
};

bool operator==( const Attendee& a1, const Attendee& a2 );

}

#endif
