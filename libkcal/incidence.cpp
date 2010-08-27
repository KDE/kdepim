/*
    This file is part of libkcal.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "calformat.h"

#include "incidence.h"
#include "calendar.h"

using namespace KCal;

Incidence::Incidence() :
  IncidenceBase(),
  mRelatedTo(0), mStatus(StatusNone), mSecrecy(SecrecyPublic),
  mPriority(0), mRecurrence(0),
  mHasRecurrenceID( false ), mChildRecurrenceEvents()
{
  recreate();

  mAlarms.setAutoDelete(true);
  mAttachments.setAutoDelete(true);
}

Incidence::Incidence( const Incidence &i ) : IncidenceBase( i ),Recurrence::Observer()
{
// TODO: reenable attributes currently commented out.
  mRevision = i.mRevision;
  mCreated = i.mCreated;
  mDescription = i.mDescription;
  mSummary = i.mSummary;
  mCategories = i.mCategories;
//  Incidence *mRelatedTo;          Incidence *mRelatedTo;
  mRelatedTo = 0;
  mRelatedToUid = i.mRelatedToUid;
//  Incidence::List mRelations;    Incidence::List mRelations;
  mResources = i.mResources;
  mStatusString = i.mStatusString;
  mStatus = i.mStatus;
  mSecrecy = i.mSecrecy;
  mPriority = i.mPriority;
  mLocation = i.mLocation;
  mRecurrenceID = i.mRecurrenceID;
  mHasRecurrenceID = i.mHasRecurrenceID;
  mChildRecurrenceEvents = i.mChildRecurrenceEvents;

  // Alarms and Attachments are stored in ListBase<...>, which is a TQValueList<...*>.
  // We need to really duplicate the objects stored therein, otherwise deleting
  // i will also delete all attachments from this object (setAutoDelete...)
  Alarm::List::ConstIterator it;
  for( it = i.mAlarms.begin(); it != i.mAlarms.end(); ++it ) {
    Alarm *b = new Alarm( **it );
    b->setParent( this );
    mAlarms.append( b );
  }
  mAlarms.setAutoDelete(true);

  Attachment::List::ConstIterator it1;
  for ( it1 = i.mAttachments.begin(); it1 != i.mAttachments.end(); ++it1 ) {
    Attachment *a = new Attachment( **it1 );
    mAttachments.append( a );
  }
  mAttachments.setAutoDelete( true );

  if (i.mRecurrence) {
    mRecurrence = new Recurrence( *(i.mRecurrence) );
    mRecurrence->addObserver( this );
  } else
    mRecurrence = 0;

  mSchedulingID = i.mSchedulingID;
}

Incidence::~Incidence()
{
    Incidence::List Relations = mRelations;
    List::ConstIterator it;
    for ( it = Relations.begin(); it != Relations.end(); ++it ) {
        if ( (*it)->relatedTo() == this ) (*it)->mRelatedTo = 0;
    }
    if ( relatedTo() ) relatedTo()->removeRelation( this );

    delete mRecurrence;
}

// A string comparison that considers that null and empty are the same
static bool stringCompare( const TQString& s1, const TQString& s2 )
{
  return ( s1.isEmpty() && s2.isEmpty() ) || (s1 == s2);
}

Incidence& Incidence::operator=( const Incidence &i )
{
  if ( &i == this )
    return *this;
  IncidenceBase::operator=( i );
  mRevision = i.mRevision;
  mCreated = i.mCreated;
  mDescription = i.mDescription;
  mSummary = i.mSummary;
  mCategories = i.mCategories;
  mRelatedTo = 0;
  mRelatedToUid = i.mRelatedToUid;
  mRelations.clear();
  mResources = i.mResources;
  mStatusString = i.mStatusString;
  mStatus = i.mStatus;
  mSecrecy = i.mSecrecy;
  mPriority = i.mPriority;
  mLocation = i.mLocation;
  mRecurrenceID = i.mRecurrenceID;
  mHasRecurrenceID = i.mHasRecurrenceID;
  mChildRecurrenceEvents = i.mChildRecurrenceEvents;

  mAlarms.clearAll();
  Alarm::List::ConstIterator it;
  for( it = i.mAlarms.begin(); it != i.mAlarms.end(); ++it ) {
    Alarm *b = new Alarm( **it );
    b->setParent( this );
    mAlarms.append( b );
  }

  mAttachments.clearAll();
  Attachment::List::ConstIterator it1;
  for ( it1 = i.mAttachments.begin(); it1 != i.mAttachments.end(); ++it1 ) {
    Attachment *a = new Attachment( **it1 );
    mAttachments.append( a );
  }

  delete mRecurrence;
  if (i.mRecurrence) {
    mRecurrence = new Recurrence( *(i.mRecurrence) );
    mRecurrence->addObserver( this );
  } else
    mRecurrence = 0;

  mSchedulingID = i.mSchedulingID;
  return *this;
}

bool Incidence::operator==( const Incidence& i2 ) const
{
    if( alarms().count() != i2.alarms().count() ) {
        return false; // no need to check further
    }

    Alarm::List::ConstIterator a1 = alarms().begin();
    Alarm::List::ConstIterator a2 = i2.alarms().begin();
    for( ; a1 != alarms().end() && a2 != i2.alarms().end(); ++a1, ++a2 )
        if( **a1 == **a2 )
            continue;
        else {
            return false;
        }

    if ( !IncidenceBase::operator==(i2) )
        return false;

    bool recurrenceEqual = ( mRecurrence == 0 && i2.mRecurrence == 0 );
    if ( !recurrenceEqual )
    {
        recurrenceEqual = mRecurrence != 0 &&
                          i2.mRecurrence != 0 &&
                          *mRecurrence == *i2.mRecurrence;
    }

    return
        recurrenceEqual &&
        created() == i2.created() &&
        stringCompare( description(), i2.description() ) &&
        stringCompare( summary(), i2.summary() ) &&
        categories() == i2.categories() &&
        // no need to compare mRelatedTo
        stringCompare( relatedToUid(), i2.relatedToUid() ) &&
        relations() == i2.relations() &&
        attachments() == i2.attachments() &&
        resources() == i2.resources() &&
        mStatus == i2.mStatus &&
        ( mStatus == StatusNone || stringCompare( mStatusString, i2.mStatusString ) ) &&
        secrecy() == i2.secrecy() &&
        priority() == i2.priority() &&
        stringCompare( location(), i2.location() ) &&
        stringCompare( schedulingID(), i2.schedulingID() );
}


void Incidence::recreate()
{
  setCreated(TQDateTime::currentDateTime());

  setUid(CalFormat::createUniqueId());
  setSchedulingID( TQString::null );

  setRevision(0);

  setLastModified(TQDateTime::currentDateTime());
  setPilotId( 0 );
  setSyncStatus( SYNCNONE );
}

void Incidence::setReadOnly( bool readOnly )
{
  IncidenceBase::setReadOnly( readOnly );
  if ( mRecurrence )
    mRecurrence->setRecurReadOnly( readOnly );
}

void Incidence::setFloats(bool f)
{
  if (mReadOnly) return;
  if ( recurrence() )
    recurrence()->setFloats( f );
  IncidenceBase::setFloats( f );
}

void Incidence::setCreated( const TQDateTime &created )
{
  if (mReadOnly) return;
  mCreated = created;

// FIXME: Shouldn't we call updated for the creation date, too?
//  updated();
}

TQDateTime Incidence::created() const
{
  return mCreated;
}

void Incidence::setRevision( int rev )
{
  if (mReadOnly) return;
  mRevision = rev;

  updated();
}

int Incidence::revision() const
{
  return mRevision;
}

void Incidence::setDtStart(const TQDateTime &dtStart)
{
  if ( mRecurrence ) {
    mRecurrence->setStartDateTime( dtStart );
    mRecurrence->setFloats( doesFloat() );
  }
  IncidenceBase::setDtStart( dtStart );
}

void Incidence::setDescription(const TQString &description)
{
  if (mReadOnly) return;
  mDescription = description;
  updated();
}

TQString Incidence::description() const
{
  return mDescription;
}


void Incidence::setSummary(const TQString &summary)
{
  if (mReadOnly) return;
  mSummary = summary;
  updated();
}

TQString Incidence::summary() const
{
  return mSummary;
}

void Incidence::setCategories(const TQStringList &categories)
{
  if (mReadOnly) return;
  mCategories = categories;
  updated();
}

// TODO: remove setCategories(TQString) function
void Incidence::setCategories(const TQString &catStr)
{
  if (mReadOnly) return;
  mCategories.clear();

  if (catStr.isEmpty()) return;

  mCategories = TQStringList::split(",",catStr);

  TQStringList::Iterator it;
  for(it = mCategories.begin();it != mCategories.end(); ++it) {
    *it = (*it).stripWhiteSpace();
  }

  updated();
}

TQStringList Incidence::categories() const
{
  return mCategories;
}

TQString Incidence::categoriesStr() const
{
  return mCategories.join(",");
}

void Incidence::setRelatedToUid(const TQString &relatedToUid)
{
  if ( mReadOnly || mRelatedToUid == relatedToUid ) return;
  mRelatedToUid = relatedToUid;
  updated();
}

TQString Incidence::relatedToUid() const
{
  return mRelatedToUid;
}

void Incidence::setRelatedTo(Incidence *relatedTo)
{
  if (mReadOnly || mRelatedTo == relatedTo) return;
  if(mRelatedTo)
    mRelatedTo->removeRelation(this);
  mRelatedTo = relatedTo;
  if (mRelatedTo) {
    mRelatedTo->addRelation(this);
    if ( mRelatedTo->uid() != mRelatedToUid )
      setRelatedToUid( mRelatedTo->uid() );
  } else {
    setRelatedToUid( TQString::null );
  }
}

Incidence *Incidence::relatedTo() const
{
  return mRelatedTo;
}

Incidence::List Incidence::relations() const
{
  return mRelations;
}

void Incidence::addRelation( Incidence *event )
{
  if ( mRelations.find( event ) == mRelations.end() ) {
    mRelations.append( event );
  }
}

void Incidence::removeRelation(Incidence *event)
// Remove the relation of our incident. E.g. if you have a task t and a
// subtask, the subtask will have its relation to the task t.
{
  mRelations.removeRef(event);
//  if (event->getRelatedTo() == this) event->setRelatedTo(0);
  mRelatedToUid=TQString();
}


// %%%%%%%%%%%%  Recurrence-related methods %%%%%%%%%%%%%%%%%%%%


Recurrence *Incidence::recurrence() const
{
  if (!mRecurrence)
  {
    const_cast<KCal::Incidence*>(this)->mRecurrence = new Recurrence();
    mRecurrence->setStartDateTime( IncidenceBase::dtStart() );
    mRecurrence->setFloats( doesFloat() );
    mRecurrence->setRecurReadOnly( mReadOnly );
    mRecurrence->addObserver( const_cast<KCal::Incidence*>(this) );
  }

  return mRecurrence;
}

void Incidence::clearRecurrence()
{
  delete mRecurrence;
  mRecurrence = 0;
}

uint Incidence::recurrenceType() const
{
  if ( mRecurrence ) return mRecurrence->recurrenceType();
  else return Recurrence::rNone;
}

bool Incidence::doesRecur() const
{
  if ( mRecurrence ) return mRecurrence->doesRecur();
  else return false;
}

bool Incidence::recursOn(const TQDate &qd) const
{
  bool doesRecur = false;
  doesRecur = mRecurrence && mRecurrence->recursOn(qd);

  return doesRecur;
}

bool Incidence::recursAt(const TQDateTime &qdt) const
{
  bool doesRecur = false;
  doesRecur = mRecurrence && mRecurrence->recursAt(qdt);

  return doesRecur;
}

bool Incidence::recursOn(const TQDate &qd, Calendar *cal) const
{
  bool doesRecur = false;
  doesRecur = mRecurrence && mRecurrence->recursOn(qd);

  // Make sure that this instance has not been moved through a RECURRENCE-ID statement
  if (hasRecurrenceID() == false) {
    IncidenceList il = childIncidences();
    IncidenceListIterator it;
    for ( it = il.begin(); it != il.end(); ++it ) {
      QDateTime modifiedDt = cal->incidence(*it)->recurrenceID();
      modifiedDt.setTime(QTime());
      if (QDateTime(qd) == modifiedDt) {
        doesRecur = false;
      }
    }
  }

  return doesRecur;
}

bool Incidence::recursAt(const TQDateTime &qdt, Calendar *cal) const
{
  bool doesRecur = false;
  doesRecur = mRecurrence && mRecurrence->recursAt(qdt);

  // Make sure that this instance has not been moved through a RECURRENCE-ID statement
  if (hasRecurrenceID() == false) {
    IncidenceList il = childIncidences();
    IncidenceListIterator it;
    for ( it = il.begin(); it != il.end(); ++it ) {
      if (qdt == cal->incidence(*it)->recurrenceID()) {
        doesRecur = false;
      }
    }
  }

  return doesRecur;
}

/**
  Calculates the start date/time for all recurrences that happen at some time
  on the given date (might start before that date, but end on or after the
  given date).
  @param date the date where the incidence should occur
  @return the start date/time of all occurences that overlap with the given
      date. Empty list if the incidence does not overlap with the date at all
*/
TQValueList<TQDateTime> Incidence::startDateTimesForDate( const TQDate &date ) const
{
//kdDebug(5800) << "Incidence::startDateTimesForDate " << date << ", incidence=" << summary() << endl;
  TQDateTime start = dtStart();
  TQDateTime end = endDateRecurrenceBase();

  TQValueList<TQDateTime> result;

  // TODO_Recurrence: Also work if only due date is given...
  if ( !start.isValid() && ! end.isValid() ) {
    return result;
  }

  // if the incidence doesn't recur,
  if ( !doesRecur() ) {
    if ( !(start.date() > date || end.date() < date ) ) {
      result << start;
    }
    return result;
  }

  int days = start.daysTo( end );
  // Account for possible recurrences going over midnight, while the original event doesn't
  TQDate tmpday( date.addDays( -days - 1 ) );
  TQDateTime tmp;
  while ( tmpday <= date ) {
    if ( recurrence()->recursOn( tmpday ) ) {
      TQValueList<TQTime> times = recurrence()->recurTimesOn( tmpday );
      for ( TQValueList<TQTime>::ConstIterator it = times.begin(); it != times.end(); ++it ) {
        tmp = TQDateTime( tmpday, *it );
        if ( endDateForStart( tmp ).date() >= date )
          result << tmp;
      }
    }
    tmpday = tmpday.addDays( 1 );
  }
  return result;
}

/**
  Calculates the start date/time for all recurrences that happen at the given
  time.
  @param datetime the date/time where the incidence should occur
  @return the start date/time of all occurences that overlap with the given
      date/time. Empty list if the incidence does not happen at the given
      time at all.
*/
TQValueList<TQDateTime> Incidence::startDateTimesForDateTime( const TQDateTime &datetime ) const
{
// kdDebug(5800) << "Incidence::startDateTimesForDateTime " << datetime << ", incidence=" << summary() << endl;
  TQDateTime start = dtStart();
  TQDateTime end = endDateRecurrenceBase();

  TQValueList<TQDateTime> result;

  // TODO_Recurrence: Also work if only due date is given...
  if ( !start.isValid() && ! end.isValid() ) {
    return result;
  }

  // if the incidence doesn't recur,
  if ( !doesRecur() ) {
    if ( !(start > datetime || end < datetime ) ) {
      result << start;
    }
    return result;
  }

  int days = start.daysTo( end );
  // Account for possible recurrences going over midnight, while the original event doesn't
  TQDate tmpday( datetime.date().addDays( -days - 1 ) );
  TQDateTime tmp;
  while ( tmpday <= datetime.date() ) {
    if ( recurrence()->recursOn( tmpday ) ) {
      TQValueList<TQTime> times = recurrence()->recurTimesOn( tmpday );
      for ( TQValueList<TQTime>::ConstIterator it = times.begin(); it != times.end(); ++it ) {
        tmp = TQDateTime( tmpday, *it );
        if ( !(tmp > datetime || endDateForStart( tmp ) < datetime ) )
          result << tmp;
      }
    }
    tmpday = tmpday.addDays( 1 );
  }
  return result;
}

/** Return the end time of the occurrence if it starts at the given date/time */
TQDateTime Incidence::endDateForStart( const TQDateTime &startDt ) const
{
  TQDateTime start = dtStart();
  TQDateTime end = endDateRecurrenceBase();
  if ( !end.isValid() ) return start;
  if ( !start.isValid() ) return end;

  return startDt.addSecs( start.secsTo( end ) );
}

// %%%%%%%%%%%%%%%%% begin:RecurrenceRule %%%%%%%%%%%%%%%%%

// Exception Dates
/*void Incidence::setExDates(const DateList &exDates)
{
  if ( mReadOnly ) return;
  recurrence()->setExDates( exDates );
  updated();
}

void Incidence::addExDate( const TQDate &date )
{
  if ( mReadOnly ) return;
  recurrence()->addExDate( date );
  updated();
}

DateList Incidence::exDates() const
{
  if ( mRecurrence ) return mRecurrence->exDates();
  else return DateList();
}


// Exception DateTimes
void Incidence::setExDateTimes( const DateTimeList &exDates )
{
  if ( mReadOnly ) return;
  recurrence()->setExDateTimes( exDates );
  updated();
}

void Incidence::addExDateTime( const TQDateTime &date )
{
  if ( mReadOnly ) return;
  recurrence()->addExDateTime( date );
  updated();
}

DateTimeList Incidence::exDateTimes() const
{
  if ( mRecurrence ) return mRecurrence->exDateTimes();
  else return DateTimeList();
}


// Recurrence Dates
void Incidence::setRDates(const DateList &exDates)
{
  if ( mReadOnly ) return;
  recurrence()->setRDates( exDates );
  updated();
}

void Incidence::addRDate( const TQDate &date )
{
  if ( mReadOnly ) return;
  recurrence()->addRDate( date );
  updated();
}

DateList Incidence::rDates() const
{
  if ( mRecurrence ) return mRecurrence->rDates();
  else return DateList();
}


// Recurrence DateTimes
void Incidence::setRDateTimes( const DateTimeList &exDates )
{
  if ( mReadOnly ) return;
  recurrence()->setRDateTimes( exDates );
  updated();
}

void Incidence::addRDateTime( const TQDateTime &date )
{
  if ( mReadOnly ) return;
  recurrence()->addRDateTime( date );
  updated();
}

DateTimeList Incidence::rDateTimes() const
{
  if ( mRecurrence ) return mRecurrence->rDateTimes();
  else return DateTimeList();
}*/

// %%%%%%%%%%%%%%%%% end:RecurrenceRule %%%%%%%%%%%%%%%%%

void Incidence::addAttachment(Attachment *attachment)
{
  if (mReadOnly || !attachment) return;
  mAttachments.append(attachment);
  updated();
}

void Incidence::deleteAttachment(Attachment *attachment)
{
  mAttachments.removeRef(attachment);
}

void Incidence::deleteAttachments( const TQString &mime )
{
  Attachment::List::Iterator it = mAttachments.begin();
  while( it != mAttachments.end() ) {
    if ( (*it)->mimeType() == mime ) mAttachments.remove( it );
    else ++it;
  }
}

Attachment::List Incidence::attachments() const
{
  return mAttachments;
}

Attachment::List Incidence::attachments(const TQString& mime) const
{
  Attachment::List attachments;
  Attachment::List::ConstIterator it;
  for( it = mAttachments.begin(); it != mAttachments.end(); ++it ) {
    if ( (*it)->mimeType() == mime ) attachments.append( *it );
  }

  return attachments;
}

void Incidence::clearAttachments()
{
  mAttachments.clearAll();
}

void Incidence::setResources(const TQStringList &resources)
{
  if (mReadOnly) return;
  mResources = resources;
  updated();
}

TQStringList Incidence::resources() const
{
  return mResources;
}


void Incidence::setPriority(int priority)
{
  if (mReadOnly) return;
  mPriority = priority;
  updated();
}

int Incidence::priority() const
{
  return mPriority;
}

void Incidence::setStatus(Incidence::Status status)
{
  if (mReadOnly || status == StatusX) return;
  mStatus = status;
  mStatusString = TQString::null;
  updated();
}

void Incidence::setCustomStatus(const TQString &status)
{
  if (mReadOnly) return;
  mStatus = status.isEmpty() ? StatusNone : StatusX;
  mStatusString = status;
  updated();
}

Incidence::Status Incidence::status() const
{
  return mStatus;
}

TQString Incidence::statusStr() const
{
  if (mStatus == StatusX)
    return mStatusString;
  return statusName(mStatus);
}

TQString Incidence::statusName(Incidence::Status status)
{
  switch (status) {
    case StatusTentative:    return i18n("incidence status", "Tentative");
    case StatusConfirmed:    return i18n("Confirmed");
    case StatusCompleted:    return i18n("Completed");
    case StatusNeedsAction:  return i18n("Needs-Action");
    case StatusCanceled:     return i18n("Canceled");
    case StatusInProcess:    return i18n("In-Process");
    case StatusDraft:        return i18n("Draft");
    case StatusFinal:        return i18n("Final");
    case StatusX:
    case StatusNone:
    default:                 return TQString::null;
  }
}

void Incidence::setSecrecy(int sec)
{
  if (mReadOnly) return;
  mSecrecy = sec;
  updated();
}

int Incidence::secrecy() const
{
  return mSecrecy;
}

TQString Incidence::secrecyStr() const
{
  return secrecyName(mSecrecy);
}

TQString Incidence::secrecyName(int secrecy)
{
  switch (secrecy) {
    case SecrecyPublic:
      return i18n("Public");
    case SecrecyPrivate:
      return i18n("Private");
    case SecrecyConfidential:
      return i18n("Confidential");
    default:
      return i18n("Undefined");
  }
}

TQStringList Incidence::secrecyList()
{
  TQStringList list;
  list << secrecyName(SecrecyPublic);
  list << secrecyName(SecrecyPrivate);
  list << secrecyName(SecrecyConfidential);

  return list;
}


const Alarm::List &Incidence::alarms() const
{
  return mAlarms;
}

Alarm* Incidence::newAlarm()
{
  Alarm* alarm = new Alarm(this);
  mAlarms.append(alarm);
//  updated();
  return alarm;
}

void Incidence::addAlarm(Alarm *alarm)
{
  mAlarms.append(alarm);
  updated();
}

void Incidence::removeAlarm(Alarm *alarm)
{
  mAlarms.removeRef(alarm);
  updated();
}

void Incidence::clearAlarms()
{
  mAlarms.clearAll();
  updated();
}

bool Incidence::isAlarmEnabled() const
{
  Alarm::List::ConstIterator it;
  for( it = mAlarms.begin(); it != mAlarms.end(); ++it ) {
    if ( (*it)->enabled() ) return true;
  }
  return false;
}

void Incidence::setLocation(const TQString &location)
{
  if (mReadOnly) return;
  mLocation = location;
  updated();
}

TQString Incidence::location() const
{
  return mLocation;
}

void Incidence::setSchedulingID( const TQString& sid )
{
  mSchedulingID = sid;
}

TQString Incidence::schedulingID() const
{
  if ( mSchedulingID.isNull() )
    // Nothing set, so use the normal uid
    return uid();
  return mSchedulingID;
}

bool Incidence::hasRecurrenceID() const
{
  return mHasRecurrenceID;
}

void Incidence::setHasRecurrenceID( bool hasRecurrenceID )
{
  if ( mReadOnly ) {
    return;
  }

  mHasRecurrenceID = hasRecurrenceID;
  updated();
}

TQDateTime Incidence::recurrenceID() const
{
  return mRecurrenceID;
}

void Incidence::setRecurrenceID( const TQDateTime &recurrenceID )
{
  if ( mReadOnly ) {
    return;
  }

//   update();
  mRecurrenceID = recurrenceID;
  updated();
}

void Incidence::addChildIncidence( TQString childIncidence )
{
  mChildRecurrenceEvents.append(childIncidence);
}

void Incidence::deleteChildIncidence( TQString childIncidence )
{
  mChildRecurrenceEvents.remove(childIncidence);
}

IncidenceList Incidence::childIncidences() const
{
  return mChildRecurrenceEvents;
}

/** Observer interface for the recurrence class. If the recurrence is changed,
    this method will be called for the incidence the recurrence object
    belongs to. */
void Incidence::recurrenceUpdated( Recurrence *recurrence )
{
  if ( recurrence == mRecurrence )
    updated();
}

