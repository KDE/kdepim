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

#include "incidencebase.h"

using namespace KCal;

IncidenceBase::IncidenceBase() :
  mReadOnly(false), mFloats(true), mDuration(0), mHasDuration(false),
  mPilotId(0), mSyncStatus(SYNCMOD)
{
  setUid(CalFormat::createUniqueId());

  mAttendees.setAutoDelete( true );
}

IncidenceBase::IncidenceBase(const IncidenceBase &i) :
  CustomProperties( i )
{
  mReadOnly = i.mReadOnly;
  mDtStart = i.mDtStart;
  mDuration = i.mDuration;
  mHasDuration = i.mHasDuration;
  mOrganizer = i.mOrganizer;
  mUid = i.mUid;
  QPtrList<Attendee> attendees = i.attendees();
  for( Attendee *a = attendees.first(); a; a = attendees.next() ) {
    mAttendees.append( new Attendee( *a ) );
  }
  mFloats = i.mFloats;
  mLastModified = i.mLastModified;
  mPilotId = i.mPilotId;
  mSyncStatus = i.mSyncStatus;

  // The copied object is a new one, so it isn't observed by the observer
  // of the original object.
  mObservers.clear();
  
  mAttendees.setAutoDelete( true );
}

IncidenceBase::~IncidenceBase()
{
}


bool KCal::operator==( const IncidenceBase& i1, const IncidenceBase& i2 )
{
    if( i1.attendees().count() != i2.attendees().count() ) {
        return false; // no need to check further
    }

    QPtrListIterator<Attendee> a1( i1.attendees() );
    QPtrListIterator<Attendee> a2( i2.attendees() );
    for( ; a1.current() && a2.current(); ++a1, ++a2 )
        if( *a1.current() == *a2.current() )
            continue;
        else {
            return false;
        }

    return ( i1.dtStart() == i2.dtStart() &&
             i1.organizer() == i2.organizer() &&
             i1.uid() == i2.uid() &&
             // Don't compare lastModified, otherwise the operator is not
             // of much use. We are not comparing for identity, after all.
             i1.doesFloat() == i2.doesFloat() &&
             i1.duration() == i2.duration() &&
             i1.hasDuration() == i2.hasDuration() &&
             i1.pilotId() == i2.pilotId() &&
             i1.syncStatus() == i2.syncStatus() );
    // no need to compare mObserver
}

        


void IncidenceBase::setUid(const QString &uid)
{
  mUid = uid;
  updated();
}

QString IncidenceBase::uid() const
{
  return mUid;
}

void IncidenceBase::setLastModified(const QDateTime &lm)
{
  // DON'T! updated() because we call this from
  // Calendar::updateEvent().
  mLastModified = lm;
}

QDateTime IncidenceBase::lastModified() const
{
  return mLastModified;
}

void IncidenceBase::setOrganizer(const QString &o)
{
  // we don't check for readonly here, because it is
  // possible that by setting the organizer we are changing
  // the event's readonly status...
  mOrganizer = o;
  if (mOrganizer.left(7).upper() == "MAILTO:")
    mOrganizer = mOrganizer.remove(0,7);

  updated();
}

QString IncidenceBase::organizer() const
{
  return mOrganizer;
}

void IncidenceBase::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;
}

void IncidenceBase::setDtStart(const QDateTime &dtStart)
{
//  if (mReadOnly) return;
  mDtStart = dtStart;
  updated();
}

QDateTime IncidenceBase::dtStart() const
{
  return mDtStart;
}

QString IncidenceBase::dtStartTimeStr() const
{
  return KGlobal::locale()->formatTime(dtStart().time());
}

QString IncidenceBase::dtStartDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(dtStart().date(),shortfmt);
}

QString IncidenceBase::dtStartStr() const
{
  return KGlobal::locale()->formatDateTime(dtStart());
}


bool IncidenceBase::doesFloat() const
{
  return mFloats;
}

void IncidenceBase::setFloats(bool f)
{
  if (mReadOnly) return;
  mFloats = f;
  updated();
}


void IncidenceBase::addAttendee(Attendee *a, bool doupdate)
{
//  kdDebug(5800) << "IncidenceBase::addAttendee()" << endl;
  if (mReadOnly) return;
//  kdDebug(5800) << "IncidenceBase::addAttendee() weiter" << endl;
  if (a->name().left(7).upper() == "MAILTO:")
    a->setName(a->name().remove(0,7));

  mAttendees.append(a);
  if (doupdate) updated();
}

#if 0
void IncidenceBase::removeAttendee(Attendee *a)
{
  if (mReadOnly) return;
  mAttendees.removeRef(a);
  updated();
}

void IncidenceBase::removeAttendee(const char *n)
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

void IncidenceBase::clearAttendees()
{
  if (mReadOnly) return;
  mAttendees.clear();
}

#if 0
Attendee *IncidenceBase::getAttendee(const char *n) const
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

Attendee *IncidenceBase::attendeeByMail(const QString &email)
{
  QPtrListIterator<Attendee> qli(mAttendees);

  qli.toFirst();
  while (qli) {
    if (qli.current()->email() == email)
      return qli.current();
    ++qli;
  }
  return 0L;
}

Attendee *IncidenceBase::attendeeByMails(const QStringList &emails, const QString& email)
{
  QPtrListIterator<Attendee> qli(mAttendees);

  QStringList mails = emails;
  if (!email.isEmpty()) {
    mails.append(email);
  }
  qli.toFirst();
  while (qli) {
    for ( QStringList::Iterator it = mails.begin(); it != mails.end(); ++it ) {
      if (qli.current()->email() == *it)
        return qli.current();
     }

    ++qli;
  }
  return 0L;
}

void IncidenceBase::setDuration(int seconds)
{
  mDuration = seconds;
  setHasDuration(true);
}

int IncidenceBase::duration() const
{
  return mDuration;
}

void IncidenceBase::setHasDuration(bool)
{
  mHasDuration = true;
}

bool IncidenceBase::hasDuration() const
{
  return mHasDuration;
}

void IncidenceBase::setSyncStatus(int stat)
{
  if (mReadOnly) return;
  mSyncStatus = stat;
}

int IncidenceBase::syncStatus() const
{
  return mSyncStatus;
}

void IncidenceBase::setPilotId( int id )
{
  if (mReadOnly) return;

  mPilotId = id;
}

int IncidenceBase::pilotId() const
{
  return mPilotId;
}

void IncidenceBase::registerObserver( IncidenceBase::Observer *observer )
{
  if( !mObservers.contains(observer) ) mObservers.append( observer );
}

void IncidenceBase::unRegisterObserver( IncidenceBase::Observer *observer )
{
  mObservers.remove( observer );
}

void IncidenceBase::updated()
{
  QPtrListIterator<Observer> it(mObservers);
  while( it.current() ) {
    Observer *o = it.current();
    ++it;
    o->incidenceUpdated( this );
  }
}
