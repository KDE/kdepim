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

#include <qstringlist.h>

#include <kdebug.h>
#include <klocale.h>

#include "attendee.h"

using namespace KCal;

Attendee::Attendee( const QString &name, const QString &email, bool _rsvp,
                    Attendee::PartStat s, Attendee::Role r, const QString &u)
  : Person( name, email )
{
  mRSVP = _rsvp;
  mStatus = s;
  mRole = r;
  mUid = u;
}

Attendee::~Attendee()
{
}

bool KCal::operator==( const Attendee& a1, const Attendee& a2 )
{
    return ( operator==( (const Person&)a1, (const Person&) a2 ) &&
             a1.RSVP() == a2.RSVP() &&
             a1.role() == a2.role() &&
             a1.status() == a2.status() &&
             a1.uid() == a2.uid() );
}

void Attendee::setStatus( Attendee::PartStat s )
{
  mStatus = s;
}

Attendee::PartStat Attendee::status() const
{
  return mStatus;
}

QString Attendee::statusStr() const
{
  return statusName( mStatus );
}

QString Attendee::statusName( Attendee::PartStat s )
{
  switch ( s ) {
    default:
    case NeedsAction:
      return i18n("Needs Action");
      break;
    case Accepted:
      return i18n("Accepted");
      break;
    case Declined:
      return i18n("Declined");
      break;
    case Tentative:
      return i18n("Tentative");
      break;
    case Delegated:
      return i18n("Delegated");
      break;
    case Completed:
      return i18n("Completed");
      break;
    case InProcess:
      return i18n("In Process");
      break;
  }
}

QStringList Attendee::statusList()
{
  QStringList list;
  list << statusName( NeedsAction );
  list << statusName( Accepted );
  list << statusName( Declined );
  list << statusName( Tentative );
  list << statusName( Delegated );
  list << statusName( Completed );
  list << statusName( InProcess );

  return list;
}


void Attendee::setRole( Attendee::Role r )
{
  mRole = r;
}

Attendee::Role Attendee::role() const
{
  return mRole;
}

QString Attendee::roleStr() const
{
  return roleName( mRole );
}

void Attendee::setUid( const QString &uid )
{
  mUid = uid;
}

QString Attendee::uid() const
{
  return mUid;
}

QString Attendee::roleName( Attendee::Role r )
{
  switch ( r ) {
    case Chair:
      return i18n("Chair");
      break;
    default:
    case ReqParticipant:
      return i18n("Participant");
      break;
    case OptParticipant:
      return i18n("Optional Participant");
      break;
    case NonParticipant:
      return i18n("Observer");
      break;
  }
}

QStringList Attendee::roleList()
{
  QStringList list;
  list << roleName( ReqParticipant );
  list << roleName( OptParticipant );
  list << roleName( NonParticipant );
  list << roleName( Chair );

  return list;
}
