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

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "calformat.h"

#include "incidence.h"

using namespace KCal;

Incidence::Incidence() :
  IncidenceBase(),
  mRelatedTo(0), mSecrecy(SecrecyPublic), mPriority(3), mRecurrence(0)
{
  recreate();

  mAlarms.setAutoDelete(true);
  mAttachments.setAutoDelete(true);
}

Incidence::Incidence( const Incidence &i ) : IncidenceBase( i )
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
  mExDates = i.mExDates;
  mAttachments = i.mAttachments;
  mResources = i.mResources;
  mSecrecy = i.mSecrecy;
  mPriority = i.mPriority;
  mLocation = i.mLocation;

  Alarm::List::ConstIterator it;
  for( it = i.mAlarms.begin(); it != i.mAlarms.end(); ++it ) {
    Alarm *b = new Alarm( **it );
    b->setParent( this );
    mAlarms.append( b );
  }
  mAlarms.setAutoDelete(true);

  if (i.mRecurrence)
    mRecurrence = new Recurrence( *(i.mRecurrence), this );
  else
    mRecurrence = 0;
}

Incidence::~Incidence()
{
  List::ConstIterator it;
  for ( it = mRelations.begin(); it != mRelations.end(); ++it ) {
    if ( (*it)->relatedTo() == this ) (*it)->setRelatedTo( 0 );
  }
  if ( relatedTo() ) relatedTo()->removeRelation( this );

  delete mRecurrence;
}

// A string comparison that considers that null and empty are the same
static bool stringCompare( const QString& s1, const QString& s2 )
{
    if ( s1.isEmpty() && s2.isEmpty() )
        return true;
    return s1 == s2;
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

    if ( !( static_cast<const IncidenceBase&>(*this) == static_cast<const IncidenceBase&>(i2) ) )
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
        exDates() == i2.exDates() &&
        attachments() == i2.attachments() &&
        resources() == i2.resources() &&
        secrecy() == i2.secrecy() &&
        priority() == i2.priority() &&
        stringCompare( location(), i2.location() );
}


void Incidence::recreate()
{
  setCreated(QDateTime::currentDateTime());

  setUid(CalFormat::createUniqueId());

  setRevision(0);

  setLastModified(QDateTime::currentDateTime());
}

void Incidence::setReadOnly( bool readOnly )
{
  IncidenceBase::setReadOnly( readOnly );
  if (mRecurrence)
    mRecurrence->setRecurReadOnly(readOnly);
}

void Incidence::setCreated(QDateTime created)
{
  if (mReadOnly) return;
  mCreated = created;
}

QDateTime Incidence::created() const
{
  return mCreated;
}

void Incidence::setRevision(int rev)
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
  if (mRecurrence)
    mRecurrence->setRecurStart( dtStart);
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

QString Incidence::categoriesStr()
{
  return mCategories.join(",");
}

void Incidence::setRelatedToUid(const QString &relatedToUid)
{
  if (mReadOnly) return;
  mRelatedToUid = relatedToUid;
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
  if (mRelatedTo) mRelatedTo->addRelation(this);
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
    updated();
  }
}

void Incidence::removeRelation(Incidence *event)
{
  mRelations.removeRef(event);
//  if (event->getRelatedTo() == this) event->setRelatedTo(0);
}

bool Incidence::recursOn(const QDate &qd) const
{
  return (mRecurrence && mRecurrence->recursOnPure(qd) && !isException(qd));
}

void Incidence::setExDates(const DateList &exDates)
{
  if (mReadOnly) return;
  mExDates = exDates;
  updated();
}

void Incidence::addExDate(const QDate &date)
{
  if (mReadOnly) return;
  mExDates.append(date);
  updated();
}

DateList Incidence::exDates() const
{
  return mExDates;
}

bool Incidence::isException(const QDate &date) const
{
  DateList::ConstIterator it;
  for( it = mExDates.begin(); it != mExDates.end(); ++it ) {
    if ( (*it) == date ) {
      return true;
    }
  }

  return false;
}

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

void Incidence::deleteAttachments(const QString& mime)
{
  Attachment *at = mAttachments.first();
  while (at) {
    if (at->mimeType() == mime)
      mAttachments.remove();
    else
      at = mAttachments.next();
  }
}

Attachment::List Incidence::attachments() const
{
  return mAttachments;
}

Attachment::List Incidence::attachments(const QString& mime) const
{
  Attachment::List attachments;
  QPtrListIterator<Attachment> it( mAttachments );
  Attachment *at;
  while ( (at = it.current()) ) {
    if (at->mimeType() == mime)
      attachments.append(at);
    ++it;
  }

  return attachments;
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
      break;
    case SecrecyPrivate:
      return i18n("Private");
      break;
    case SecrecyConfidential:
      return i18n("Confidential");
      break;
    default:
      return i18n("Undefined");
      break;
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
  mAlarms.clear();
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

Recurrence *Incidence::recurrence() const
{
  if (!mRecurrence)
  {
    const_cast<KCal::Incidence*>(this)->mRecurrence = new Recurrence(const_cast<KCal::Incidence*>(this));
    mRecurrence->setRecurReadOnly(mReadOnly);
    mRecurrence->setRecurStart(dtStart());
  }

  return mRecurrence;
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

ushort Incidence::doesRecur() const
{
  if ( mRecurrence ) return mRecurrence->doesRecur();
  else return Recurrence::rNone;
}
