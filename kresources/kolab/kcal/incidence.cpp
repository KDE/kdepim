/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "incidence.h"

#include <libkcal/journal.h>
#include <kdebug.h>

using namespace Kolab;


Incidence::Incidence( KCal::Incidence* incidence )
{
  if ( incidence )
    setFields( incidence );
}

Incidence::~Incidence()
{
}

void Incidence::setSummary( const QString& summary )
{
  mSummary = summary;
}

QString Incidence::summary() const
{
  return mSummary;
}

void Incidence::setLocation( const QString& location )
{
  mLocation = location;
}

QString Incidence::location() const
{
  return mLocation;
}

void Incidence::setOrganizer( const Email& organizer )
{
  mOrganizer = organizer;
}

KolabBase::Email Incidence::organizer() const
{
  return mOrganizer;
}

void Incidence::setStartDate( const QDateTime& startDate )
{
  mStartDate = startDate;
}

QDateTime Incidence::startDate() const
{
  return mStartDate;
}

void Incidence::setAlarm( int alarm )
{
  mAlarm = alarm;
}

int Incidence::alarm() const
{
  return mAlarm;
}

void Incidence::setRecurrence( const Recurrence& recurrence )
{
  mRecurrence = recurrence;
}

Incidence::Recurrence Incidence::recurrence() const
{
  return mRecurrence;
}

void Incidence::addAttendee( const Attendee& attendee )
{
  mAttendees.append( attendee );
}

QValueList<Incidence::Attendee>& Incidence::attendees()
{
  return mAttendees;
}

const QValueList<Incidence::Attendee>& Incidence::attendees() const
{
  return mAttendees;
}

bool Incidence::loadAttendeeAttribute( QDomElement& element,
                                       Attendee& attendee )
{
  for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      QString tagName = e.tagName();

      if ( tagName == "display-name" )
        attendee.displayName = e.text();
      else if ( tagName == "smtp-address" )
        attendee.smtpAddress = e.text();
      else if ( tagName == "status" )
        attendee.status = e.text();
      else if ( tagName == "request-response" )
        // This sets reqResp to false, if the text is "false". Otherwise it
        // sets it to true. This means the default setting is true.
        attendee.requestResponse = ( e.text().lower() != "false" );
      else if ( tagName == "invitationSent" )
        // Like above, only this defaults to false
        attendee.invitationSent = ( e.text().lower() != "true" );
      else if ( tagName == "role" )
        attendee.role = e.text();
      else
        // TODO: Unhandled tag - save for later storage
        kdDebug() << "Warning: Unhandled tag " << e.tagName() << endl;
    } else
      kdDebug() << "Node is not a comment or an element???" << endl;
  }

  return true;
}

void Incidence::saveAttendeeAttribute( QDomElement& element,
                                       const Attendee& attendee ) const
{
  writeString( element, "display-name", attendee.displayName );
  writeString( element, "smtp-address", attendee.smtpAddress );
  writeString( element, "status", attendee.status );
  writeString( element, "request-response",
               ( attendee.requestResponse ? "true" : "false" ) );
  writeString( element, "invitationSent",
               ( attendee.invitationSent ? "true" : "false" ) );
  writeString( element, "role", attendee.role );
}

void Incidence::saveAttendees( QDomElement& element ) const
{
  QValueList<Attendee>::ConstIterator it = mAttendees.begin();
  for ( ; it != mAttendees.end(); ++it )
    saveAttendeeAttribute( element, *it );
}

bool Incidence::loadAttribute( QDomElement& element )
{
  QString tagName = element.tagName();

  if ( tagName == "summary" )
    setSummary( element.text() );
  else if ( tagName == "location" )
    setLocation( element.text() );
  else if ( tagName == "organizer" ) {
    Email email;
    if ( loadEmailAttribute( element, email ) ) {
      setOrganizer( email );
      return true;
    } else
      return false;
  } else if ( tagName == "start-date" )
    setStartDate( stringToDateTime( element.text() ) );
  else if ( tagName == "recurrence" )
    // TODO
    ;
  else if ( tagName == "attendee" ) {
    Attendee attendee;
    if ( loadAttendeeAttribute( element, attendee ) ) {
      addAttendee( attendee );
      return true;
    } else
      return false;
  } else
    return KolabBase::loadAttribute( element );

  // We handled this
  return true;
}

bool Incidence::saveAttributes( QDomElement& element ) const
{
  // Save the base class elements
  KolabBase::saveAttributes( element );

  writeString( element, "summary", summary() );
  writeString( element, "location", location() );
  saveEmailAttribute( element, organizer(), "organizer" );
  writeString( element, "start-date", dateTimeToString( startDate() ) );
  // saveRecurrenceAttribute();
  saveAttendees( element );
  return true;
}


void Incidence::setFields( const KCal::Incidence* incidence )
{
  kdDebug() << "NYFI: " << k_funcinfo << endl;
  KolabBase::setFields( incidence );
}

void Incidence::saveTo( KCal::Incidence* incidence )
{
  kdDebug() << "NYFI: " << k_funcinfo << endl;
  KolabBase::saveTo( incidence );
}

