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

// $Id$

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

  recreate();

  mAlarms.setAutoDelete(true);
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

  QPtrListIterator<Alarm> it( i.mAlarms );
  const Alarm *a;
  while( (a = it.current()) ) {
    Alarm *b = new Alarm( *a );
    b->setParent( this );
    mAlarms.append( b );

    ++it;
  }

//  Recurrence *mRecurrence;      Recurrence *mRecurrence;
  mRecurrence = new Recurrence(this);
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

void Incidence::setAttachments(const QStringList &attachments)
{
  if (mReadOnly) return;
  mAttachments = attachments;
  updated();
}

QStringList Incidence::attachments() const
{
  return mAttachments;
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
