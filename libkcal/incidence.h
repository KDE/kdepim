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
#ifndef INCIDENCE_H
#define INCIDENCE_H
//
// Incidence - base class of calendaring components
//

#include <qdatetime.h>
#include <qstringlist.h>
#include <qvaluelist.h>

#include "recurrence.h"
#include "alarm.h"
#include "attachment.h"

#include "incidencebase.h"

namespace KCal {

class Event;
class Todo;
class Journal;

/**
  This class provides the base class common to all calendar components.
*/
class Incidence : public IncidenceBase
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
          Reimplement this function in your concrete subclass of IncidenceVisitor to perform actions
          on an Event object.
        */
        virtual bool visit(Event *) { return false; }
        /**
          Reimplement this function in your concrete subclass of IncidenceVisitor to perform actions
          on an Todo object.
        */
        virtual bool visit(Todo *) { return false; }
        /**
          Reimplement this function in your concrete subclass of IncidenceVisitor to perform actions
          on an Journal object.
        */
        virtual bool visit(Journal *) { return false; }

      protected:
        /** Constructor is protected to prevent direct creation of visitor base class. */
        Visitor() {}
    };

    /** enumeration for describing an event's secrecy. */
    enum { SecrecyPublic = 0, SecrecyPrivate = 1, SecrecyConfidential = 2 };

    Incidence();
    Incidence(const Incidence &);
    ~Incidence();

    /**
      Accept IncidenceVisitor. A class taking part in the visitor mechanism has to
      provide this implementation:
      <pre>
        bool accept(Visitor &v) { return v.visit(this); }
      </pre>
    */
    virtual bool accept(Visitor &) { return false; }

    virtual Incidence *clone() = 0;

    void setReadOnly( bool );

    /**
      Recreate event. The event is made a new unique event, but already stored
      event information is preserved. Sets uniquie id, creation date, last
      modification date and revision number.
    */
    void recreate();

    /** set creation date */
    void setCreated(QDateTime);
    /** return time and date of cration. */
    QDateTime created() const;

    /** set the number of revisions this event has seen */
    void setRevision(int rev);
    /** return the number of revisions this event has seen */
    int revision() const;

    /** Set starting date/time. */
    void setDtStart(const QDateTime &dtStart);

    /** sets the event's lengthy description. */
    void setDescription(const QString &description);
    /** returns a reference to the event's description. */
    QString description() const;

    /** sets the event's short summary. */
    void setSummary(const QString &summary);
    /** returns a reference to the event's summary. */
    QString summary() const;

    /** set event's applicable categories */
    void setCategories(const QStringList &categories);
    /** set event's categories based on a comma delimited string */
    void setCategories(const QString &catStr);
    /** return categories in a list */
    QStringList categories() const;
    /** return categories as a comma separated string */
    QString categoriesStr();

    /** point at some other event to which the event relates. This function should
     *  only be used when constructing a calendar before the related Event
     *  exists. */
    void setRelatedToUid(const QString &);
    /** what event does this one relate to? This function should
     *  only be used when constructing a calendar before the related Event
     *  exists. */
    QString relatedToUid() const;
    /** point at some other event to which the event relates */
    void setRelatedTo(Incidence *relatedTo);
    /** what event does this one relate to? */
    Incidence *relatedTo() const;
    /** All events that are related to this event */
    QPtrList<Incidence> relations() const;
    /** Add an event which is related to this event */
    void addRelation(Incidence *);
    /** Remove event that is related to this event */
    void removeRelation(Incidence *);

    /** returns the list of dates which are exceptions to the recurrence rule */
    DateList exDates() const;
    /** sets the list of dates which are exceptions to the recurrence rule */
    void setExDates(const DateList &_exDates);
    void setExDates(const char *dates);
    /** Add a date to the list of exceptions of the recurrence rule. */
    void addExDate(const QDate &date);

    /** returns true if there is an exception for this date in the recurrence
     rule set, or false otherwise. */
    bool isException(const QDate &qd) const;

    /** add attachment to this event */
    void addAttachment(Attachment *attachment);
    /** remove and delete a specific attachment */
    void deleteAttachment(Attachment *attachment);
    /** remove and delete all attachments with this mime type */
    void deleteAttachments(const QString& mime);
    /** return list of all associated attachments */
    QPtrList<Attachment> attachments() const;
    /** find a list of attachments with this mime type */
    QPtrList<Attachment> attachments(const QString& mime) const;

    /** sets the event's status the value specified.  See the enumeration
     * above for possible values. */
    void setSecrecy(int);
    /** return the event's secrecy. */
    int secrecy() const;
    /** return the event's secrecy in string format. */
    QString secrecyStr() const;
    /** return list of all availbale secrecy classes */
    static QStringList secrecyList();
    /** return human-readable name of secrecy class */
    static QString secrecyName(int);

    /** returns TRUE if the date specified is one on which the event will
     * recur. */
    bool recursOn(const QDate &qd) const;

    // VEVENT and VTODO, but not VJOURNAL (move to EventBase class?):

    /** set resources used, such as Office, Car, etc. */
    void setResources(const QStringList &resources);
    /** return list of current resources */
    QStringList resources() const;

    /** set the event's priority, 0 is undefined, 1 highest (decreasing order) */
    void setPriority(int priority);
    /** get the event's priority */
    int priority() const;

    /** All alarms that are associated with this incidence */
    QPtrList<Alarm> alarms() const;
    /** Create a new alarm which is associated with this incidence */
    Alarm* newAlarm();
    /** Add an alarm which is associated with this incidence */
    void addAlarm(Alarm*);
    /** Remove an alarm that is associated with this incidence */
    void removeAlarm(Alarm*);
    /** Remove all alarms that are associated with this incidence */
    void clearAlarms();
    /** return whether any alarm associated with this incidence is enabled */
    bool isAlarmEnabled() const;

    /**
      Return the recurrence rule associated with this incidence. If there is
      none, returns an appropriate (non-0) object.
    */
    Recurrence *recurrence() const;
    
    /** set the event's/todo's location. Do _not_ use it with journal */
    void setLocation(const QString &location);
    /** return the event's/todo's location. Do _not_ use it with journal */
    QString location() const;
    
  private:
    int mRevision;

    // base components of jounal, event and todo
    QDateTime mCreated;
    QString mDescription;
    QString mSummary;
    QStringList mCategories;
    Incidence *mRelatedTo;
    QString mRelatedToUid;
    QPtrList<Incidence> mRelations;
    DateList mExDates;
    QPtrList<Attachment> mAttachments;
    QStringList mResources;

    int mSecrecy;
    int mPriority;                        // 1 = highest, 2 = less, etc.

    QPtrList<Alarm> mAlarms;
    Recurrence *mRecurrence;
    
    QString mLocation;
};

bool operator==( const Incidence&, const Incidence& );

}

#endif
