/*
    This file is part of libkcal.

    Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>
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
#include <kurl.h>

#include "calformat.h"

#include "incidencebase.h"

using namespace KCal;

IncidenceBase::IncidenceBase()
  : mReadOnly( false ), mFloats( true ), mDuration( 0 ), mHasDuration( false ),
    mPilotId( 0 ), mSyncStatus( SYNCMOD )
{
  setUid( CalFormat::createUniqueId() );

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
  Attendee::List attendees = i.attendees();
  Attendee::List::ConstIterator it;
  for( it = attendees.begin(); it != attendees.end(); ++it ) {
    mAttendees.append( new Attendee( *(*it) ) );
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


bool IncidenceBase::operator==( const IncidenceBase& i2 ) const
{
  if( attendees().count() != i2.attendees().count() ) {
      return false; // no need to check further
  }

  Attendee::List al1 = attendees();
  Attendee::List al2 = i2.attendees();
  Attendee::List::ConstIterator a1 = al1.begin();
  Attendee::List::ConstIterator a2 = al2.begin();
  for( ; a1 != al1.end() && a2 != al2.end(); ++a1, ++a2 ) {
    if( **a1 == **a2 )
        continue;
    else {
        return false;
    }
  }

  if ( !CustomProperties::operator==(i2) )
    return false;

  return ( dtStart() == i2.dtStart() &&
           organizer() == i2.organizer() &&
           uid() == i2.uid() &&
           // Don't compare lastModified, otherwise the operator is not
           // of much use. We are not comparing for identity, after all.
           doesFloat() == i2.doesFloat() &&
           duration() == i2.duration() &&
           hasDuration() == i2.hasDuration() &&
           pilotId() == i2.pilotId() &&
           syncStatus() == i2.syncStatus() );
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

  // Remove milliseconds part.
  QDateTime current = lm;
  QTime t = current.time();
  t.setHMS( t.hour(), t.minute(), t.second(), 0 );
  current.setTime( t );

  mLastModified = current;
}

QDateTime IncidenceBase::lastModified() const
{
  return mLastModified;
}

void IncidenceBase::setOrganizer( const Person &o )
{
  // we don't check for readonly here, because it is
  // possible that by setting the organizer we are changing
  // the event's readonly status...
  mOrganizer = o;

  updated();
}

void IncidenceBase::setOrganizer(const QString &o)
{
  QString mail( o );
  if ( mail.startsWith("MAILTO:", Qt::CaseInsensitive) )
    mail = mail.remove( 0, 7 );
  // split the string into full name plus email.
  Person organizer( mail );
  setOrganizer( organizer );
}

Person IncidenceBase::organizer() const
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


void IncidenceBase::addComment(const QString& comment)
{
  mComments += comment;
}

bool IncidenceBase::removeComment( const QString& comment)
{
  bool found = false;
  QStringList::Iterator i;

  for ( i = mComments.begin(); !found && i != mComments.end(); ++i ) {
    if ( (*i) == comment ) {
      found = true;
      mComments.erase(i);
    }
  }

  return found;
}

void IncidenceBase::clearComments()
{
  mComments.clear();
}

QStringList IncidenceBase::comments() const
{
  return mComments;
}


void IncidenceBase::addAttendee(Attendee *a, bool doupdate)
{
//  kdDebug(5800) << "IncidenceBase::addAttendee()" << endl;
  if (mReadOnly) return;
//  kdDebug(5800) << "IncidenceBase::addAttendee() weiter" << endl;
  if (a->name().left(7).toUpper() == "MAILTO:")
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

Attendee *IncidenceBase::attendeeByMail( const QString &email ) const
{
  Attendee::List::ConstIterator it;
  for( it = mAttendees.begin(); it != mAttendees.end(); ++it ) {
    if ( (*it)->email() == email ) return *it;
  }

  return 0;
}

Attendee *IncidenceBase::attendeeByMails( const QStringList &emails,
                                          const QString &email) const
{
  QStringList mails = emails;
  if ( !email.isEmpty() ) mails.append( email );

  Attendee::List::ConstIterator itA;
  for( itA = mAttendees.begin(); itA != mAttendees.end(); ++itA ) {
    for ( QStringList::Iterator it = mails.begin(); it != mails.end(); ++it ) {
      if ( (*itA)->email() == (*it) ) return *itA;
    }
  }

  return 0;
}

Attendee *IncidenceBase::attendeeByUid( const QString &uid ) const
{
  Attendee::List::ConstIterator it;
  for( it = mAttendees.begin(); it != mAttendees.end(); ++it ) {
    if ( (*it)->uid() == uid ) return *it;
  }

  return 0;
}


void IncidenceBase::setDuration(int seconds)
{
  mDuration = seconds;
  setHasDuration(true);
  updated();
}

int IncidenceBase::duration() const
{
  return mDuration;
}

void IncidenceBase::setHasDuration(bool hasDuration)
{
  mHasDuration = hasDuration;
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

void IncidenceBase::setPilotId( unsigned long id )
{
  if (mReadOnly) return;
  mPilotId = id;
  updated();
}

unsigned long IncidenceBase::pilotId() const
{
  return mPilotId;
}

void IncidenceBase::registerObserver( IncidenceBase::Observer *observer )
{
  if( !mObservers.contains( observer ) ) mObservers.append( observer );
}

void IncidenceBase::unRegisterObserver( IncidenceBase::Observer *observer )
{
  mObservers.removeAll( observer );
}

void IncidenceBase::updated()
{
  foreach ( Observer *o, mObservers )
    o->incidenceUpdated( this );
}

KURL IncidenceBase::uri() const
{
  return KURL( QString( "urn:x-ical:" ) + uid() );
}
