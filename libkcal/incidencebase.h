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
#ifndef KCAL_INCIDENCEBASE_H
#define KCAL_INCIDENCEBASE_H

#include <qdatetime.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qptrlist.h>

#include "customproperties.h"
#include "attendee.h"

namespace KCal {

typedef QValueList<QDate> DateList;
typedef QValueList<QDateTime> DateTimeList;
class Event;
class Todo;
class Journal;
class FreeBusy;

/**
  This class provides the base class common to all calendar components.
*/
class IncidenceBase : public CustomProperties
{
  public:
    /**
      This class provides the interface for a visitor of calendar components. It
      serves as base class for concrete visitors, which implement certain actions on
      calendar components. It allows to add functions, which operate on the concrete
      types of calendar components, without changing the calendar component classes.
    */
    class Visitor
    {
      public:
        /** Destruct Incidence::Visitor */
        virtual ~Visitor() {}

        /**
          Reimplement this function in your concrete subclass of IncidenceBase::Visitor to perform actions
          on an Event object.
        */
        virtual bool visit(Event *) { return false; }
        /**
          Reimplement this function in your concrete subclass of IncidenceBase::Visitor to perform actions
          on a Todo object.
        */
        virtual bool visit(Todo *) { return false; }
        /**
          Reimplement this function in your concrete subclass of IncidenceBase::Visitor to perform actions
          on an Journal object.
        */
        virtual bool visit(Journal *) { return false; }
        /**
          Reimplement this function in your concrete subclass of IncidenceBase::Visitor to perform actions
          on a FreeBusy object.
        */
        virtual bool visit(FreeBusy *) { return false; }

      protected:
        /** Constructor is protected to prevent direct creation of visitor base class. */
        Visitor() {}
    };

    class Observer {
      public:
        virtual void incidenceUpdated( IncidenceBase * ) = 0;
    };

    IncidenceBase();
    IncidenceBase( const IncidenceBase & );
    virtual ~IncidenceBase();
    bool operator==( const IncidenceBase & ) const;

    /**
      Accept IncidenceVisitor. A class taking part in the visitor mechanism has to
      provide this implementation:
      <pre>
        bool accept(Visitor &v) { return v.visit(this); }
      </pre>
    */
    virtual bool accept(Visitor &) { return false; }

    virtual QCString type() const = 0;

    /** Set the unique id for the event */
    void setUid( const QString & );
    /** Return the unique id for the event */
    QString uid() const;

    /** Sets the time the incidence was last modified. */
    void setLastModified( const QDateTime &lm );
    /** Return the time the incidence was last modified. */
    QDateTime lastModified() const;

    /** sets the organizer for the event */
    void setOrganizer( const QString &o );
    QString organizer() const;

    /** Set readonly status. */
    virtual void setReadOnly( bool );
    /** Return if the object is read-only. */
    bool isReadOnly() const { return mReadOnly; }

    /** for setting the event's starting date/time with a QDateTime. */
    virtual void setDtStart( const QDateTime &dtStart );
    /** returns an event's starting date/time as a QDateTime. */
    virtual QDateTime dtStart() const;
    /** returns an event's starting time as a string formatted according to the
     users locale settings */
    virtual QString dtStartTimeStr() const;
    /** returns an event's starting date as a string formatted according to the
     users locale settings */
    virtual QString dtStartDateStr( bool shortfmt = true ) const;
    /** returns an event's starting date and time as a string formatted according
     to the users locale settings */
    virtual QString dtStartStr() const;

    virtual void setDuration( int seconds );
    int duration() const;
    void setHasDuration( bool );
    bool hasDuration() const;

    /** Return true or false depending on whether the incidence "floats,"
     * i.e. has a date but no time attached to it. */
    bool doesFloat() const;
    /** Set whether the incidence floats, i.e. has a date but no time attached to it. */
    void setFloats( bool f );

    //
    // Comments
    //

    /**
     * Add a comment to this incidence.
     *
     * Does not add a linefeed character.  Just appends the text as passed in.
     *
     * @param comment  The comment to add.
     */
    void addComment(const QString& comment);

    /**
     * Remove a comment from the event.
     *
     * Removes first comment whose string is an exact match for the string
     * passed in.
     *
     * @return true if match found, false otherwise.
     */
    bool removeComment( const QString& comment );

    /** Delete all comments associated with this incidence. */
    void clearComments();

    /** Return all comments associated with this incidence.  */
    QStringList comments() const;

    /**
      Add Attendee to this incidence. IncidenceBase takes ownership of the
      Attendee object.

      @param attendee a pointer to the attendee to add
      @param doUpdate If true the Observers are notified, if false they are not.
    */
    void addAttendee( Attendee *attendee, bool doUpdate = true );
    /**
      Remove all Attendees.
    */
    void clearAttendees();
    /**
      Return list of attendees.
    */
    const Attendee::List &attendees() const { return mAttendees; };
    /**
      Return number of attendees.
    */
    int attendeeCount() const { return mAttendees.count(); };
    /**
      Return the Attendee with this email address.
    */
    Attendee *attendeeByMail( const QString & ) const;
    /**
      Return first Attendee with one of the given email addresses.
    */
    Attendee *attendeeByMails( const QStringList &,
                               const QString &email = QString::null ) const;
    /**
      Return attendee with given uid.
    */
    Attendee *attendeeByUid( const QString &uid ) const;

    /**
      Pilot synchronization states
    */
    enum { SYNCNONE = 0, SYNCMOD = 1, SYNCDEL = 3 };
    /**
      Set synchronisation satus.
    */
    void setSyncStatus( int status );
    /**
      Return synchronisation status.
    */
    int syncStatus() const;

    /**
      Set Pilot Id.
    */
    void setPilotId( unsigned long id );
    /**
      Return Pilot Id.
    */
    unsigned long pilotId() const;

    /**
      Register observer. The observer is notified when the observed object
      changes.
    */
    void registerObserver( Observer * );
    /**
      Unregister observer. It isn't notified anymore about changes.
    */
    void unRegisterObserver( Observer * );
    /**
      Call this to notify the observers after the IncidenceBas object has
      changed.
    */
    void updated();

  protected:
    bool mReadOnly;

  private:
    // base components
    QDateTime mDtStart;
    QString mOrganizer;
    QString mUid;
    QDateTime mLastModified;
    Attendee::List mAttendees;
    QStringList mComments;

    bool mFloats;

    int mDuration;
    bool mHasDuration;

    // PILOT SYNCHRONIZATION STUFF
    unsigned long mPilotId;                         // unique id for pilot sync
    int mSyncStatus;                      // status (for sync)

    QPtrList<Observer> mObservers;

    class Private;
    Private *d;
};

}

#endif
