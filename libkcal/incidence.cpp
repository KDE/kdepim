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
  mReadOnly(false), mRelatedTo(0), mSecrecy(SecrecyPublic), mPriority(3), mPilotId(0),
  mSyncStatus(SYNCMOD), mFloats(true), mDuration(0), mHasDuration(false)
{
  mRecurrence = new Recurrence(this);

  recreate();

//  setOrganizer(KOPrefs::instance()->mEmail);
  setOrganizer("Fix me");
  if (organizer().isEmpty())
    setOrganizer("x-none");

  mSummary = "";
  mDescription = "";

  mAlarms.setAutoDelete(true);
}

Incidence::Incidence(const Incidence &i) : QObject()
{
// TODO: reenable attributes currently commented out.
  mReadOnly = i.mReadOnly;
  kdDebug() << endl;
  mDtStart = i.mDtStart;
  mDuration = i.mDuration;
  mHasDuration = i.mHasDuration;
  mOrganizer = i.mOrganizer;
  mVUID = i.mVUID;
  mRevision = i.mRevision;
//  QPtrList<Attendee> mAttendees;     QPtrList<Attendee> mAttendees;
  mLastModified = i.mLastModified;
  mCreated = i.mCreated;
  mDescription = i.mDescription;
  mSummary = i.mSummary;
  mCategories = i.mCategories;
//  Incidence *mRelatedTo;          Incidence *mRelatedTo;
  mRelatedTo = 0;
  mRelatedToVUID = i.mRelatedToVUID;
//  QPtrList<Incidence> mRelations;    QPtrList<Incidence> mRelations;
  mExDates = i.mExDates;
  mAttachments = i.mAttachments;
  mResources = i.mResources;
  mSecrecy = i.mSecrecy;
  mPriority = i.mPriority;
  mPilotId = i.mPilotId;
  mSyncStatus = i.mSyncStatus;
  mFloats = i.mFloats;
//  QPtrList<Alarm> mAlarms;    QPtrList<Alarm> mAlarms;
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

  setVUID(CalFormat::createUniqueId());

  setRevision(0);

  setLastModified(QDateTime::currentDateTime());
}

void Incidence::setReadOnly(bool readonly)
{
  mReadOnly = readonly;
  recurrence()->setRecurReadOnly(mReadOnly);
  for (Alarm* alarm = mAlarms.first(); alarm; alarm = mAlarms.next())
    alarm->setAlarmReadOnly(mReadOnly);
}

void Incidence::setLastModified(const QDateTime &lm)
{
  // DON'T! emit eventUpdated because we call this from
  // Calendar::updateEvent().
  mLastModified = lm;
}

QDateTime Incidence::lastModified() const
{
  return mLastModified;
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

void Incidence::setVUID(const QString &VUID)
{
  mVUID = VUID;
  emit eventUpdated(this);
}

QString Incidence::VUID() const
{
  return mVUID;
}

void Incidence::setRevision(int rev)
{
  if (mReadOnly) return;
  mRevision = rev;
  emit eventUpdated(this);
}

int Incidence::revision() const
{
  return mRevision;
}

void Incidence::setOrganizer(const QString &o)
{
  // we don't check for readonly here, because it is
  // possible that by setting the organizer we are changing
  // the event's readonly status...
  mOrganizer = o;
  if (mOrganizer.left(7).upper() == "MAILTO:")
    mOrganizer = mOrganizer.remove(0,7);

  emit eventUpdated(this);
}

QString Incidence::organizer() const
{
  return mOrganizer;
}


void Incidence::setDtStart(const QDateTime &dtStart)
{
//  if (mReadOnly) return;
  mDtStart = dtStart;
  recurrence()->setRecurStart(mDtStart);
  emit eventUpdated(this);
}

QDateTime Incidence::dtStart() const
{
  return mDtStart;
}

QString Incidence::dtStartTimeStr() const
{
  return KGlobal::locale()->formatTime(dtStart().time());
}

QString Incidence::dtStartDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(dtStart().date(),shortfmt);
}

QString Incidence::dtStartStr() const
{
  return KGlobal::locale()->formatDateTime(dtStart());
}


bool Incidence::doesFloat() const
{
  return mFloats;
}

void Incidence::setFloats(bool f)
{
  if (mReadOnly) return;
  mFloats = f;
  emit eventUpdated(this);
}


void Incidence::addAttendee(Attendee *a)
{
//  kdDebug() << "Incidence::addAttendee()" << endl;
  if (mReadOnly) return;
//  kdDebug() << "Incidence::addAttendee() weiter" << endl;
  if (a->name().left(7).upper() == "MAILTO:")
    a->setName(a->name().remove(0,7));

  mAttendees.append(a);
  emit eventUpdated(this);
}

#if 0
void Incidence::removeAttendee(Attendee *a)
{
  if (mReadOnly) return;
  mAttendees.removeRef(a);
  emit eventUpdated(this);
}

void Incidence::removeAttendee(const char *n)
{
  Attendee *a;

  if (mReadOnly) return;
  for (a = mAttendees.first(); a; a = mAttendees.next())
    if (a->getName() == n) {
      mAttendees.remove();
      break;
    }
}
#endif

void Incidence::clearAttendees()
{
  if (mReadOnly) return;
  mAttendees.clear();
}

#if 0
Attendee *Incidence::getAttendee(const char *n) const
{
  QPtrListIterator<Attendee> qli(mAttendees);

  qli.toFirst();
  while (qli) {
    if (qli.current()->getName() == n)
      return qli.current();
    ++qli;
  }
  return 0L;
}
#endif

void Incidence::setDescription(const QString &description)
{
  if (mReadOnly) return;
  mDescription = description;
  emit eventUpdated(this);
}

QString Incidence::description() const
{
  return mDescription;
}


void Incidence::setSummary(const QString &summary)
{
  if (mReadOnly) return;
  mSummary = summary;
  emit eventUpdated(this);
}

QString Incidence::summary() const
{
  return mSummary;
}

void Incidence::setCategories(const QStringList &categories)
{
  if (mReadOnly) return;
  mCategories = categories;
  emit eventUpdated(this);
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

  emit eventUpdated(this);
}

QStringList Incidence::categories() const
{
  return mCategories;
}

QString Incidence::categoriesStr()
{
  return mCategories.join(",");
}

void Incidence::setRelatedToVUID(const QString &relatedToVUID)
{
  if (mReadOnly) return;
  mRelatedToVUID = relatedToVUID;
}

QString Incidence::relatedToVUID() const
{
  return mRelatedToVUID;
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
  emit eventUpdated(this);
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

  emit eventUpdated(this);
}

void Incidence::addExDate(const QDate &date)
{
  if (mReadOnly) return;
  mExDates.append(date);

  recurrence()->setRecurExDatesCount(mExDates.count());

  emit eventUpdated(this);
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
  emit eventUpdated(this);
}

QStringList Incidence::attachments() const
{
  return mAttachments;
}

void Incidence::setResources(const QStringList &resources)
{
  if (mReadOnly) return;
  mResources = resources;
  emit eventUpdated(this);
}

QStringList Incidence::resources() const
{
  return mResources;
}


void Incidence::setPriority(int priority)
{
  if (mReadOnly) return;
  mPriority = priority;
  emit eventUpdated(this);
}

int Incidence::priority() const
{
  return mPriority;
}

void Incidence::setSecrecy(int sec)
{
  if (mReadOnly) return;
  mSecrecy = sec;
  emit eventUpdated(this);
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


void Incidence::setPilotId(int id)
{
  if (mReadOnly) return;
  mPilotId = id;
  //emit eventUpdated(this);
}

int Incidence::pilotId() const
{
  return mPilotId;
}

void Incidence::setSyncStatus(int stat)
{
  if (mReadOnly) return;
  mSyncStatus = stat;
  //  emit eventUpdated(this);
}

int Incidence::syncStatus() const
{
  return mSyncStatus;
}


const QPtrList<Alarm> &Incidence::alarms() const
{
  return mAlarms;
}

Alarm* Incidence::newAlarm()
{
  Alarm* alarm = new Alarm(this);
  mAlarms.append(alarm);
//  emit eventUpdated(this);
  return alarm;
}

void Incidence::addAlarm(Alarm *alarm)
{
  mAlarms.append(alarm);
  emit eventUpdated(this);
}

void Incidence::removeAlarm(Alarm *alarm)
{
  mAlarms.removeRef(alarm);
  emit eventUpdated(this);
}

void Incidence::clearAlarms()
{
  mAlarms.clear();
  emit eventUpdated(this);
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

void Incidence::setDuration(int seconds)
{
  mDuration = seconds;
  setHasDuration(true);
}

int Incidence::duration() const
{
  return mDuration;
}

void Incidence::setHasDuration(bool)
{
  mHasDuration = true;
}

bool Incidence::hasDuration() const
{
  return mHasDuration;
}

#include "incidence.moc"
