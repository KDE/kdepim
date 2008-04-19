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

using namespace KCal;

Incidence::Incidence() :
  IncidenceBase(),
  mRelatedTo(0), mStatus(StatusNone), mSecrecy(SecrecyPublic),
  mPriority(0), mRecurrence(0)
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

  // Alarms and Attachments are stored in ListBase<...>, which is a QValueList<...*>.
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
static bool stringCompare( const QString& s1, const QString& s2 )
{
  return ( s1.isEmpty() && s2.isEmpty() ) || (s1 == s2);
}

Incidence& Incidence::operator=( const Incidence &i )
{
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
  setCreated(QDateTime::currentDateTime());

  setUid(CalFormat::createUniqueId());
  setSchedulingID( QString::null );

  setRevision(0);

  setLastModified(QDateTime::currentDateTime());
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

void Incidence::setCreated( const QDateTime &created )
{
  if (mReadOnly) return;
  mCreated = created;

// FIXME: Shouldn't we call updated for the creation date, too?
//  updated();
}

QDateTime Incidence::created() const
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

void Incidence::setDtStart(const QDateTime &dtStart)
{
  if ( mRecurrence ) {
    mRecurrence->setStartDateTime( dtStart );
    mRecurrence->setFloats( doesFloat() );
  }
  IncidenceBase::setDtStart( dtStart );
}

void Incidence::setDescription(const QString &description)
{
  if (mReadOnly) return;
  mDescription = description;
  updated();
}

QString Incidence::description() const
{
  return mDescription;
}


void Incidence::setSummary(const QString &summary)
{
  if (mReadOnly) return;
  mSummary = summary;
  updated();
}

QString Incidence::summary() const
{
  return mSummary;
}

void Incidence::setCategories(const QStringList &categories)
{
  if (mReadOnly) return;
  mCategories = categories;
  updated();
}

// TODO: remove setCategories(QString) function
void Incidence::setCategories(const QString &catStr)
{
  if (mReadOnly) return;
  mCategories.clear();

  if (catStr.isEmpty()) return;

  mCategories = QStringList::split(",",catStr);

  QStringList::Iterator it;
  for(it = mCategories.begin();it != mCategories.end(); ++it) {
    *it = (*it).stripWhiteSpace();
  }

  updated();
}

QStringList Incidence::categories() const
{
  return mCategories;
}

QString Incidence::categoriesStr() const
{
  return mCategories.join(",");
}

void Incidence::setRelatedToUid(const QString &relatedToUid)
{
  if ( mReadOnly || mRelatedToUid == relatedToUid ) return;
  mRelatedToUid = relatedToUid;
  updated();
}

QString Incidence::relatedToUid() const
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
    setRelatedToUid( QString::null );
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
  mRelatedToUid=QString();
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

bool Incidence::recursOn(const QDate &qd) const
{
  return ( mRecurrence && mRecurrence->recursOn(qd) );
}

bool Incidence::recursAt(const QDateTime &qdt) const
{
  return ( mRecurrence && mRecurrence->recursAt(qdt) );
}

/**
  Calculates the start date/time for all recurrences that happen at some time
  on the given date (might start before that date, but end on or after the
  given date).
  @param date the date where the incidence should occur
  @return the start date/time of all occurences that overlap with the given
      date. Empty list if the incidence does not overlap with the date at all
*/
QValueList<QDateTime> Incidence::startDateTimesForDate( const QDate &date ) const
{
//kdDebug(5800) << "Incidence::startDateTimesForDate " << date << ", incidence=" << summary() << endl;
  QDateTime start = dtStart();
  QDateTime end = endDateRecurrenceBase();

  QValueList<QDateTime> result;

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
  QDate tmpday( date.addDays( -days - 1 ) );
  QDateTime tmp;
  while ( tmpday <= date ) {
    if ( recurrence()->recursOn( tmpday ) ) {
      QValueList<QTime> times = recurrence()->recurTimesOn( tmpday );
      for ( QValueList<QTime>::ConstIterator it = times.begin(); it != times.end(); ++it ) {
        tmp = QDateTime( tmpday, *it );
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
QValueList<QDateTime> Incidence::startDateTimesForDateTime( const QDateTime &datetime ) const
{
// kdDebug(5800) << "Incidence::startDateTimesForDateTime " << datetime << ", incidence=" << summary() << endl;
  QDateTime start = dtStart();
  QDateTime end = endDateRecurrenceBase();

  QValueList<QDateTime> result;

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
  QDate tmpday( datetime.date().addDays( -days - 1 ) );
  QDateTime tmp;
  while ( tmpday <= datetime.date() ) {
    if ( recurrence()->recursOn( tmpday ) ) {
      QValueList<QTime> times = recurrence()->recurTimesOn( tmpday );
      for ( QValueList<QTime>::ConstIterator it = times.begin(); it != times.end(); ++it ) {
        tmp = QDateTime( tmpday, *it );
        if ( !(tmp > datetime || endDateForStart( tmp ) < datetime ) )
          result << tmp;
      }
    }
    tmpday = tmpday.addDays( 1 );
  }
  return result;
}

/** Return the end time of the occurrence if it starts at the given date/time */
QDateTime Incidence::endDateForStart( const QDateTime &startDt ) const
{
  QDateTime start = dtStart();
  QDateTime end = endDateRecurrenceBase();
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

void Incidence::addExDate( const QDate &date )
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

void Incidence::addExDateTime( const QDateTime &date )
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

void Incidence::addRDate( const QDate &date )
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

void Incidence::addRDateTime( const QDateTime &date )
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

void Incidence::deleteAttachments( const QString &mime )
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

Attachment::List Incidence::attachments(const QString& mime) const
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

void Incidence::setResources(const QStringList &resources)
{
  if (mReadOnly) return;
  mResources = resources;
  updated();
}

QStringList Incidence::resources() const
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
  mStatusString = QString::null;
  updated();
}

void Incidence::setCustomStatus(const QString &status)
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

QString Incidence::statusStr() const
{
  if (mStatus == StatusX)
    return mStatusString;
  return statusName(mStatus);
}

QString Incidence::statusName(Incidence::Status status)
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
    default:                 return QString::null;
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

QString Incidence::secrecyStr() const
{
  return secrecyName(mSecrecy);
}

QString Incidence::secrecyName(int secrecy)
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

QStringList Incidence::secrecyList()
{
  QStringList list;
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

void Incidence::setLocation(const QString &location)
{
  if (mReadOnly) return;
  mLocation = location;
  updated();
}

QString Incidence::location() const
{
  return mLocation;
}

void Incidence::setSchedulingID( const QString& sid )
{
  mSchedulingID = sid;
}

QString Incidence::schedulingID() const
{
  if ( mSchedulingID.isNull() )
    // Nothing set, so use the normal uid
    return uid();
  return mSchedulingID;
}

/** Observer interface for the recurrence class. If the recurrence is changed,
    this method will be called for the incidence the recurrence object
    belongs to. */
void Incidence::recurrenceUpdated( Recurrence *recurrence )
{
  if ( recurrence == mRecurrence )
    updated();
}

