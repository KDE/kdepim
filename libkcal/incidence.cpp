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
  mRelatedTo(0), mSecrecy(SecrecyPublic), mPriority(3)
{
  mRecurrence = new Recurrence(this);
  mLocation = ""; // avoid failures in comparison

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
//  QPtrList<Incidence> mRelations;    QPtrList<Incidence> mRelations;
  mExDates = i.mExDates;
  mAttachments = i.mAttachments;
  mResources = i.mResources;
  mSecrecy = i.mSecrecy;
  mPriority = i.mPriority;
  mLocation = i.mLocation;

  QPtrListIterator<Alarm> it( i.mAlarms );
  const Alarm *a;
  while( (a = it.current()) ) {
    Alarm *b = new Alarm( *a );
    b->setParent( this );
    mAlarms.append( b );

    ++it;
  }
  mAlarms.setAutoDelete(true);

  mRecurrence = new Recurrence( *(i.mRecurrence), this );
}

Incidence::~Incidence()
{
  Incidence *ev;
  for (ev=mRelations.first();ev;ev=mRelations.next()) {
    if (ev->relatedTo() == this) ev->setRelatedTo(0);
  }
  if (relatedTo()) relatedTo()->removeRelation(this);

  delete mRecurrence;
}


bool KCal::operator==( const Incidence& i1, const Incidence& i2 )
{
    if( i1.alarms().count() != i2.alarms().count() ) {
        return false; // no need to check further
    }
    
    for( Alarm* a1 = i1.alarms().first(), *a2 = i2.alarms().first();
         a1; a1 = i1.alarms().next(), a2 = i2.alarms().next() )
        if( *a1 == *a2 ) {
            continue;
        } else {
            return false;
        }

    return operator==( (const IncidenceBase&)i1, (const IncidenceBase&)i2 ) &&
        i1.created() == i2.created() &&
        i1.description() == i2.description() &&
        i1.summary() == i2.summary() &&
        i1.categories() == i2.categories() &&
        // no need to compare mRelatedTo
        i1.relatedToUid() == i2.relatedToUid() &&
        i1.relations() == i2.relations() &&
        i1.exDates() == i2.exDates() &&
        i1.attachments() == i2.attachments() &&
        i1.resources() == i2.resources() &&
        i1.secrecy() == i2.secrecy() &&
        i1.priority() == i2.priority() &&
        *i1.recurrence() == *i2.recurrence() &&
        i1.location() == i2.location();
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
  recurrence()->setRecurReadOnly( readOnly);
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
  recurrence()->setRecurStart( dtStart);
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
  if (mReadOnly) return;
  Incidence *oldRelatedTo = mRelatedTo;
  if(oldRelatedTo) {
    oldRelatedTo->removeRelation(this);
  }
  mRelatedTo = relatedTo;
  if (mRelatedTo) mRelatedTo->addRelation(this);
}

Incidence *Incidence::relatedTo() const
{
  return mRelatedTo;
}

QPtrList<Incidence> Incidence::relations() const
{
  return mRelations;
}

void Incidence::addRelation(Incidence *event)
{
  mRelations.append(event);
  updated();
}

void Incidence::removeRelation(Incidence *event)
{
  mRelations.removeRef(event);
//  if (event->getRelatedTo() == this) event->setRelatedTo(0);
}

bool Incidence::recursOn(const QDate &qd) const
{
  if (recurrence()->recursOnPure(qd) && !isException(qd)) return true;
  else return false;
}

void Incidence::setExDates(const DateList &exDates)
{
  if (mReadOnly) return;
  mExDates = exDates;

  recurrence()->setRecurExDatesCount(mExDates.count());

  updated();
}

void Incidence::addExDate(const QDate &date)
{
  if (mReadOnly) return;
  mExDates.append(date);

  recurrence()->setRecurExDatesCount(mExDates.count());

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

QPtrList<Attachment> Incidence::attachments() const
{
  return mAttachments;
}

QPtrList<Attachment> Incidence::attachments(const QString& mime) const
{
  QPtrList<Attachment> attachments;
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


QPtrList<Alarm> Incidence::alarms() const
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
  Alarm* alarm;
  for (QPtrListIterator<Alarm> it(mAlarms); (alarm = it.current()) != 0; ++it) {
    if (alarm->enabled())
      return true;
  }
  return false;
}

Recurrence *Incidence::recurrence() const
{
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
