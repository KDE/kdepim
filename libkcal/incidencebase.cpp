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

#include "incidencebase.h"
#include "incidencebase.moc"

using namespace KCal;

IncidenceBase::IncidenceBase() :
  mReadOnly(false), mFloats(true), mDuration(0), mHasDuration(false)
{
  setVUID(CalFormat::createUniqueId());
}

IncidenceBase::IncidenceBase(const IncidenceBase &i) : QObject()
{
  mReadOnly = i.mReadOnly;
  mDtStart = i.mDtStart;
  mDuration = i.mDuration;
  mHasDuration = i.mHasDuration;
  mOrganizer = i.mOrganizer;
  mVUID = i.mVUID;
  mAttendees = i.attendees();
  mFloats = i.mFloats;
}

IncidenceBase::~IncidenceBase()
{
}

void IncidenceBase::setVUID(const QString &VUID)
{
  mVUID = VUID;
  emit eventUpdated(this);
}

QString IncidenceBase::VUID() const
{
  return mVUID;
}

void IncidenceBase::setOrganizer(const QString &o)
{
  // we don't check for readonly here, because it is
  // possible that by setting the organizer we are changing
  // the event's readonly status...
  mOrganizer = o;
  if (mOrganizer.left(7).upper() == "MAILTO:")
    mOrganizer = mOrganizer.remove(0,7);

  emit eventUpdated(this);
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
  emit eventUpdated(this);
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
  emit eventUpdated(this);
}


void IncidenceBase::addAttendee(Attendee *a)
{
//  kdDebug(5800) << "IncidenceBase::addAttendee()" << endl;
  if (mReadOnly) return;
//  kdDebug(5800) << "IncidenceBase::addAttendee() weiter" << endl;
  if (a->name().left(7).upper() == "MAILTO:")
    a->setName(a->name().remove(0,7));

  mAttendees.append(a);
  emit eventUpdated(this);
}

#if 0
void IncidenceBase::removeAttendee(Attendee *a)
{
  if (mReadOnly) return;
  mAttendees.removeRef(a);
  emit eventUpdated(this);
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
