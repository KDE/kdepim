/*
    This file is part of libkcal.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef INCIDENCE_H
#define INCIDENCE_H

#include <qdatetime.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "recurrence.h"
#include "alarm.h"
#include "attachment.h"
#include "libkcal_export.h"

#include "incidencebase.h"
#include <kdepimmacros.h>

namespace KCal {


/**
  This class provides the base class common to all calendar components.
*/
class LIBKCAL_EXPORT Incidence : public IncidenceBase
{
  public:
    /**
      This class implements a visitor for adding an Incidence to a resource
      supporting addEvent(), addTodo() and addJournal() calls.
    */
    template<class T>
    class AddVisitor : public IncidenceBase::Visitor
    {
      public:
        AddVisitor( T *r ) : mResource( r ) {}

        bool visit( Event *e ) { return mResource->addEvent( e ); }
        bool visit( Todo *t ) { return mResource->addTodo( t ); }
        bool visit( Journal *j ) { return mResource->addJournal( j ); }

      private:
        T *mResource;
    };

    /**
      This class implements a visitor for deleting an Incidence from a resource
      supporting deleteEvent(), deleteTodo() and deleteJournal() calls.
    */
    template<class T>
    class DeleteVisitor : public IncidenceBase::Visitor
    {
      public:
        DeleteVisitor( T *r ) : mResource( r ) {}

        bool visit( Event *e ) { mResource->deleteEvent( e ); return true; }
        bool visit( Todo *t ) { mResource->deleteTodo( t ); return true; }
        bool visit( Journal *j ) { mResource->deleteJournal( j ); return true; }

      private:
        T *mResource;
    };

    /** Enumeration for describing an event's status.  */
    enum Status {
        StatusNone, StatusTentative, StatusConfirmed, StatusCompleted,
        StatusNeedsAction, StatusCanceled, StatusInProcess, StatusDraft,
        StatusFinal,
        StatusX   // indicates a non-standard status string
    };

    /** enumeration for describing an event's secrecy. */
    enum { SecrecyPublic = 0, SecrecyPrivate = 1, SecrecyConfidential = 2 };

    typedef ListBase<Incidence> List;

    Incidence();
    Incidence( const Incidence & );
    ~Incidence();

    bool operator==( const Incidence & ) const;

    /**
      Return copy of this object. The returned object is owned by the caller.
    */
    virtual Incidence *clone() = 0;

    /**
      Set readonly state of incidence.

      @param readonly If true, the incidence is set to readonly, if false the
                      incidence is set to readwrite.
    */
    void setReadOnly( bool readonly );

    /**
      Recreate event. The event is made a new unique event, but already stored
      event information is preserved. Sets uniquie id, creation date, last
      modification date and revision number.
    */
    void recreate();

    /**
      Set creation date.
    */
    void setCreated( const QDateTime & );
    /**
      Return time and date of creation.
    */
    QDateTime created() const;

    /**
      Set the number of revisions this event has seen.
    */
    void setRevision( int rev );
    /**
      Return the number of revisions this event has seen.
    */
    int revision() const;

    /**
      Set starting date/time.
    */
    virtual void setDtStart( const QDateTime &dtStart );
    /**
      Return the incidence's ending date/time as a QDateTime.
    */
    virtual QDateTime dtEnd() const  { return QDateTime(); }

    /**
      Set the long description.
    */
    void setDescription( const QString &description );
    /**
      Return long description.
    */
    QString description() const;

    /**
      Set short summary.
    */
    void setSummary( const QString &summary );
    /**
      Return short summary.
    */
    QString summary() const;

    /**
      Set categories.
    */
    void setCategories( const QStringList &categories );
    /**
      Set categories based on a comma delimited string.
    */
    void setCategories(const QString &catStr);
    /**
      Return categories as a list of strings.
    */
    QStringList categories() const;
    /**
      Return categories as a comma separated string.
    */
    QString categoriesStr() const;

    /**
      Point at some other event to which the event relates. This function should
      only be used when constructing a calendar before the related Incidence
      exists.
    */
    void setRelatedToUid(const QString &);
    /**
      What event does this one relate to? This function should
      only be used when constructing a calendar before the related Incidence
      exists.
    */
    QString relatedToUid() const;
    /**
      Point at some other event to which the event relates
    */
    void setRelatedTo(Incidence *relatedTo);
    /**
      What event does this one relate to?
    */
    Incidence *relatedTo() const;
    /**
      All events that are related to this event.
    */
    Incidence::List relations() const;
    /**
      Add an event which is related to this event.
    */
    void addRelation(Incidence *);
    /**
      Remove event that is related to this event.
    */
    void removeRelation(Incidence *);

    /**
      Returns the list of dates which are exceptions to the recurrence rule.
    */
    DateList exDates() const;
    /**
      Returns the list of date/times which are exceptions to the recurrence
      rule.
    */
    DateTimeList exDateTimes() const;
    /**
      Sets the list of dates which are exceptions to the recurrence rule.
      This does not affect the date-time exception list.
    */
    void setExDates( const DateList &exDates );
    /**
      Sets the list of date/times which are exceptions to the recurrence rule.
     This does not affect the date-only exception list.
    */
    void setExDateTimes( const DateTimeList &exDateTimes );
    /**
      Add a date to the list of exceptions of the recurrence rule.
    */
    void addExDate( const QDate &date );
    /**
      Add a date/time to the list of exceptions of the recurrence rule.
    */
    void addExDateTime( const QDateTime &dateTime );

    /**
      Returns true if there is an exception for this date in the recurrence
      rule set, or false otherwise. Does not check the date/time exception list.
    */
    bool isException( const QDate &qd ) const;
    /**
      Returns true if there is an exception for this date/time in the recurrence
      rule set, or false otherwise. Does not check the date-only exception list.
    */
    bool isException( const QDateTime &qdt ) const;

    /**
      Add attachment.
    */
    void addAttachment( Attachment *attachment );
    /**
      Remove and delete a specific attachment.
    */
    void deleteAttachment( Attachment *attachment );
    /**
      Remove and delete all attachments with this mime type.
    */
    void deleteAttachments( const QString &mime );
    /**
      Return list of all associated attachments.
    */
    Attachment::List attachments() const;
    /**
      Find a list of attachments with this mime type.
    */
    Attachment::List attachments( const QString &mime ) const;
    /**
      Remove and delete all attachments.
    */
    void clearAttachments();

    /**
      Sets secrecy status. This can be Public, Private or Confidential. See
      separate enum.
    */
    void setSecrecy( int );
    /**
      Return the event's secrecy.
    */
    int secrecy() const;
    /**
      Return secrecy as translated string.
    */
    QString secrecyStr() const;
    /**
      Return list of all available secrecy states as list of translated strings.
    */
    static QStringList secrecyList();
    /**
      Return human-readable translated name of secrecy class.
    */
    static QString secrecyName( int );

    /**
      Sets the incidence status to a standard status value. See
      separate enum. Note that StatusX cannot be specified.
    */
    void setStatus( Status status );
    /**
      Sets the incidence status to a non-standard status value.
      @param status non-standard status string. If empty,
      the incidence status will be set to StatusNone.
    */
    void setCustomStatus( const QString &status );
    /**
      Return the event's status.
    */
    Status status() const;
    /**
      Return the event's status string.
    */
    QString statusStr() const;
    /**
      Return human-readable translated name of status value.
    */
    static QString statusName( Status );

    /**
      Returns true if the date specified is one on which the incidence will
      recur.
    */
    virtual bool recursOn( const QDate &qd ) const;
    /**
      Returns true if the date/time specified is one on which the incidence will
      recur.
    */
    bool recursAt( const QDateTime &qdt ) const;

    // VEVENT and VTODO, but not VJOURNAL (move to EventBase class?):

    /**
      Set resources used, such as Office, Car, etc.
    */
    void setResources( const QStringList &resources );
    /**
      Return list of current resources.
    */
    QStringList resources() const;

    /**
      Set the incidences priority, 0 is undefined, 1 highest (decreasing order).
    */
    void setPriority( int priority );
    /**
      Return priority. The priority is a number between 1 and 5. 1 is highest
      priority.
    */
    int priority() const;

    /**
      All alarms that are associated with this incidence.
    */
    const Alarm::List &alarms() const;
    /**
      Create a new alarm which is associated with this incidence.
    */
    Alarm *newAlarm();
    /**
      Add an alarm which is associated with this incidence.
    */
    void addAlarm( Alarm * );
    /**
      Remove an alarm that is associated with this incidence.
    */
    void removeAlarm( Alarm * );
    /**
      Remove all alarms that are associated with this incidence.
    */
    void clearAlarms();
    /**
      Return whether any alarm associated with this incidence is enabled.
    */
    bool isAlarmEnabled() const;

    /**
      Return the recurrence rule associated with this incidence. If there is
      none, returns an appropriate (non-0) object.
    */
    Recurrence *recurrence() const;

    /**
      Forward to Recurrence::doesRecur().
    */
    ushort doesRecur() const;

    /**
      Set the event's/todo's location. Do _not_ use it with journal.
    */
    void setLocation(const QString &location);
    /**
      Return the event's/todo's location. Do _not_ use it with journal.
    */
    QString location() const;

    /**
      Set the event's/todo's scheduling ID. Does not make sense for journals.
      This is used for accepted invitations as the place to store the UID
      of the invitation. It is later used again if updates to the
      invitation comes in.
      If we did not set a new UID on incidences from invitations, we can
      end up with more than one resource having events with the same UID,
      if you have access to other peoples resources.
    */
    void setSchedulingID( const QString& sid );
    /**
      Return the event's/todo's scheduling ID. Does not make sense for journals
      If this is not set, it will return uid().
    */
    QString schedulingID() const;

  private:
    int mRevision;

    // base components of jounal, event and todo
    QDateTime mCreated;
    QString mDescription;
    QString mSummary;
    QStringList mCategories;
    Incidence *mRelatedTo;
    QString mRelatedToUid;
    Incidence::List mRelations;
    DateList mExDates;
    DateTimeList mExDateTimes;
    Attachment::List mAttachments;
    QStringList mResources;

    QString mStatusString;
    Status  mStatus;
    int mSecrecy;
    int mPriority;                        // 1 = highest, 2 = less, etc.

    Alarm::List mAlarms;
    Recurrence *mRecurrence;

    QString mLocation;

    // Scheduling ID - used only to identify between scheduling mails
    QString mSchedulingID;

    class Private;
    Private *d;
};

}

#endif
