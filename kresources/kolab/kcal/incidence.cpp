/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>

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

#include <qvaluelist.h>

#include <libkcal/journal.h>
#include <korganizer/version.h>
#include <kdebug.h>

using namespace Kolab;


Incidence::Incidence( const QString& tz, KCal::Incidence* incidence )
  : KolabBase( tz ), mFloatingStatus( Unset ), mHasAlarm( false )
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
  if ( mFloatingStatus == AllDay )
    kdDebug() << "ERROR: Time on start date but no time on the event\n";
  mFloatingStatus = HasTime;
}

void Incidence::setStartDate( const QDate& startDate )
{
  mStartDate = startDate;
  if ( mFloatingStatus == HasTime )
    kdDebug() << "ERROR: No time on start date but time on the event\n";
  mFloatingStatus = AllDay;
}

void Incidence::setStartDate( const QString& startDate )
{
  if ( startDate.length() > 10 )
    // This is a date + time
    setStartDate( stringToDateTime( startDate ) );
  else
    // This is only a date
    setStartDate( stringToDate( startDate ) );
}

QDateTime Incidence::startDate() const
{
  return mStartDate;
}

void Incidence::setAlarm( float alarm )
{
  mAlarm = alarm;
  mHasAlarm = true;
}

float Incidence::alarm() const
{
  return mAlarm;
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
      else if ( tagName == "invitation-sent" )
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
  QDomElement e = element.ownerDocument().createElement( "attendee" );
  element.appendChild( e );
  writeString( e, "display-name", attendee.displayName );
  writeString( e, "smtp-address", attendee.smtpAddress );
  writeString( e, "status", attendee.status );
  writeString( e, "request-response",
               ( attendee.requestResponse ? "true" : "false" ) );
  writeString( e, "invitation-sent",
               ( attendee.invitationSent ? "true" : "false" ) );
  writeString( e, "role", attendee.role );
}

void Incidence::saveAttendees( QDomElement& element ) const
{
  QValueList<Attendee>::ConstIterator it = mAttendees.begin();
  for ( ; it != mAttendees.end(); ++it )
    saveAttendeeAttribute( element, *it );
}

void Incidence::saveAttachments( QDomElement& element ) const
{
  KCal::Attachment::List::ConstIterator it = mAttachments.begin();
  for ( ; it != mAttachments.end(); ++it ) {
    KCal::Attachment *a = (*it);
    if ( a->isUri() ) {
      writeString( element, "link-attachment", a->uri() );
    } else if ( a->isBinary() ) {
      // TODO
    }
  }
}

void Incidence::saveRecurrence( QDomElement& element ) const
{
  QDomElement e = element.ownerDocument().createElement( "recurrence" );
  element.appendChild( e );
  e.setAttribute( "cycle", mRecurrence.cycle );
  if ( !mRecurrence.type.isEmpty() )
    e.setAttribute( "type", mRecurrence.type );
  writeString( e, "interval", QString::number( mRecurrence.interval ) );
  for( QStringList::ConstIterator it = mRecurrence.days.begin(); it != mRecurrence.days.end(); ++it ) {
    writeString( e, "day", *it );
  }
  if ( !mRecurrence.dayNumber.isEmpty() )
    writeString( e, "daynumber", mRecurrence.dayNumber );
  if ( !mRecurrence.date.isEmpty() )
    writeString( e, "date", mRecurrence.date );
  if ( !mRecurrence.month.isEmpty() )
    writeString( e, "month", mRecurrence.month );
  if ( !mRecurrence.rangeType.isEmpty() ) {
    QDomElement range = element.ownerDocument().createElement( "range" );
    e.appendChild( range );
    range.setAttribute( "type", mRecurrence.rangeType );
    QDomText t = element.ownerDocument().createTextNode( mRecurrence.range );
    range.appendChild( t );
  }
  for( QValueList<QDate>::ConstIterator it = mRecurrence.exclusions.begin();
       it != mRecurrence.exclusions.end(); ++it ) {
    writeString( e, "exclusion", dateToString( *it ) );
  }
}

void Incidence::loadRecurrence( const QDomElement& element )
{
  mRecurrence.interval = 0;
  mRecurrence.cycle = element.attribute( "cycle" );
  mRecurrence.type = element.attribute( "type" );
  for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      QString tagName = e.tagName();

      if ( tagName == "interval" )
        mRecurrence.interval = e.text().toInt();
      else if ( tagName == "day" ) // can be present multiple times
        mRecurrence.days.append( e.text() );
      else if ( tagName == "daynumber" )
        mRecurrence.dayNumber = e.text();
      else if ( tagName == "date" )
        mRecurrence.date = e.text();
      else if ( tagName == "month" )
        mRecurrence.month = e.text();
      else if ( tagName == "range" ) {
        mRecurrence.rangeType = e.attribute( "type" );
        mRecurrence.range = e.text();
      } else if ( tagName == "exclusion" ) {
        mRecurrence.exclusions.append( stringToDate( e.text() ) );
      } else
        // TODO: Unhandled tag - save for later storage
        kdDebug() << "Warning: Unhandled tag " << e.tagName() << endl;
    }
  }
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
    setStartDate( element.text() );
  else if ( tagName == "recurrence" )
    loadRecurrence( element );
  else if ( tagName == "attendee" ) {
    Attendee attendee;
    if ( loadAttendeeAttribute( element, attendee ) ) {
      addAttendee( attendee );
      return true;
    } else
      return false;
  } else if ( tagName == "inline-attachment" ) {
    // TODO
  } else if ( tagName == "link-attachment" ) {
    mAttachments.push_back( new KCal::Attachment( element.text() ) );
  } else if ( tagName == "alarm" )
    // Alarms should be minutes before. Libkcal uses event time + alarm time
    setAlarm( - element.text().toInt() );
  else if ( tagName == "x-custom" )
    loadCustomAttributes( element );
  else {
    bool ok = KolabBase::loadAttribute( element );
    if ( !ok ) {
        // Unhandled tag - save for later storage
        kdDebug() << "Saving unhandled tag " << element.tagName() << endl;
        Custom c;
        c.key = QCString( "X-KDE-KolabUnhandled-" ) + element.tagName().latin1();
        c.value = element.text();
        mCustomList.append( c );
    }
  }
  // We handled this
  return true;
}

bool Incidence::saveAttributes( QDomElement& element ) const
{
  // Save the base class elements
  KolabBase::saveAttributes( element );

  if ( mFloatingStatus == HasTime )
    writeString( element, "start-date", dateTimeToString( startDate() ) );
  else
    writeString( element, "start-date", dateToString( startDate().date() ) );
  writeString( element, "summary", summary() );
  writeString( element, "location", location() );
  saveEmailAttribute( element, organizer(), "organizer" );
  if ( !mRecurrence.cycle.isEmpty() )
    saveRecurrence( element );
  saveAttendees( element );
  saveAttachments( element );
  if ( mHasAlarm ) {
    // Alarms should be minutes before. Libkcal uses event time + alarm time
    int alarmTime = qRound( -alarm() );
    writeString( element, "alarm", QString::number( alarmTime ) );
  }
  saveCustomAttributes( element );
  return true;
}

void Incidence::saveCustomAttributes( QDomElement& element ) const
{
  QValueList<Custom>::ConstIterator it = mCustomList.begin();
  for ( ; it != mCustomList.end(); ++it ) {
    QString key = (*it).key;
    Q_ASSERT( !key.isEmpty() );
    if ( key.startsWith( "X-KDE-KolabUnhandled-" ) ) {
      key = key.mid( strlen( "X-KDE-KolabUnhandled-" ) );
      writeString( element, key, (*it).value );
    } else {
      // Let's use attributes so that other tag-preserving-code doesn't need sub-elements
      QDomElement e = element.ownerDocument().createElement( "x-custom" );
      element.appendChild( e );
      e.setAttribute( "key", key );
      e.setAttribute( "value", (*it).value );
    }
  }
}

void Incidence::loadCustomAttributes( QDomElement& element )
{
  Custom custom;
  custom.key = element.attribute( "key" ).latin1();
  custom.value = element.attribute( "value" );
  mCustomList.append( custom );
}

static KCal::Attendee::PartStat attendeeStringToStatus( const QString& s )
{
  if ( s == "none" )
    return KCal::Attendee::NeedsAction;
  if ( s == "tentative" )
    return KCal::Attendee::Tentative;
  if ( s == "declined" )
    return KCal::Attendee::Declined;

  // Default:
  return KCal::Attendee::Accepted;
}

static QString attendeeStatusToString( KCal::Attendee::PartStat status )
{
  switch( status ) {
  case KCal::Attendee::NeedsAction:
    return "none";
  case KCal::Attendee::Accepted:
    return "accepted";
  case KCal::Attendee::Declined:
    return "declined";
  case KCal::Attendee::Tentative:
    return "tentative";
  case KCal::Attendee::Delegated:
  case KCal::Attendee::Completed:
  case KCal::Attendee::InProcess:
    // These don't have any meaning in the Kolab format, so just use:
    return "accepted";
  }

  // Default for the case that there are more added later:
  return "accepted";
}

static KCal::Attendee::Role attendeeStringToRole( const QString& s )
{
  if ( s == "optional" )
    return KCal::Attendee::OptParticipant;
  if ( s == "resource" )
    return KCal::Attendee::NonParticipant;
  return KCal::Attendee::ReqParticipant;
}

static QString attendeeRoleToString( KCal::Attendee::Role role )
{
  switch( role ) {
  case KCal::Attendee::ReqParticipant:
    return "required";
  case KCal::Attendee::OptParticipant:
    return "optional";
  case KCal::Attendee::Chair:
    // We don't have the notion of chair, so use
    return "required";
  case KCal::Attendee::NonParticipant:
    // In Kolab, a non-participant is a resource
    return "resource";
  }

  // Default for the case that there are more added later:
  return "required";
}

static const char *s_weekDayName[] =
{
  "monday", "tuesday", "wednesday", "thursday", "friday", "saturday", "sunday"
};

static const char *s_monthName[] =
{
  "january", "february", "march", "april", "may", "june", "july",
  "august", "september", "october", "november", "december"
};

void Incidence::setRecurrence( KCal::Recurrence* recur )
{
  mRecurrence.interval = recur->frequency();
  switch ( recur->doesRecur() ) {
  case KCal::Recurrence::rMinutely: // Not handled by the kolab XML
    mRecurrence.cycle = "minutely";
    break;
  case KCal::Recurrence::rHourly:  // Not handled by the kolab XML
    mRecurrence.cycle = "hourly";
    break;
  case KCal::Recurrence::rDaily:
    mRecurrence.cycle = "daily";
    break;
  case KCal::Recurrence::rWeekly: // every X weeks
    mRecurrence.cycle = "weekly";
    {
      QBitArray arr = recur->days();
      for ( uint idx = 0 ; idx < 7 ; ++idx )
        if ( arr.testBit( idx ) )
          mRecurrence.days.append( s_weekDayName[idx] );
    }
    break;
  case KCal::Recurrence::rMonthlyPos: {
    mRecurrence.cycle = "monthly";
    mRecurrence.type = "weekday";
    const QPtrList<KCal::Recurrence::rMonthPos> &monthPositions = recur->monthPositions();
    if ( !monthPositions.isEmpty() ) {
      KCal::Recurrence::rMonthPos monthPos = *monthPositions.getFirst();
      QBitArray arr = monthPos.rDays;
      for ( uint idx = 0 ; idx < 7 ; ++idx )
        if ( arr.testBit( idx ) )
          mRecurrence.days.append( s_weekDayName[idx] );
      mRecurrence.dayNumber = QString::number( monthPos.rPos );
      // Not handled: monthPos.negative (nth days before end of month)
    }
    break;
  }
  case KCal::Recurrence::rMonthlyDay: {
    mRecurrence.cycle = "monthly";
    mRecurrence.type = "daynumber";
    const QPtrList<int> &monthDays = recur->monthDays();
    // ####### Kolab XML limitation: only the first month day is used
    if ( !monthDays.isEmpty() )
      mRecurrence.date = QString::number( *monthDays.getFirst() );
    break;
  }
  case KCal::Recurrence::rYearlyMonth: // (day n of Month Y)
  {
    mRecurrence.cycle = "yearly";
    mRecurrence.type = "monthday";
    QPtrList<int> rmd = recur->monthDays();
    int day = !rmd.isEmpty() ? day = *rmd.first() : day = recur->parent()->dtStart().date().day();
    mRecurrence.date = QString::number( day );
    QPtrList<int> months = recur->yearNums();
    if ( !months.isEmpty() )
      mRecurrence.month = s_monthName[ *months.first() ];
    break;
  }
  case KCal::Recurrence::rYearlyDay: // YearlyDay (day N of the year)
    mRecurrence.cycle = "yearly";
    mRecurrence.type = "yearday";
    mRecurrence.dayNumber = QString::number( *recur->yearNums().getFirst() );
    break;
  case KCal::Recurrence::rYearlyPos: // (weekday X of week N of month Y)
    // ##### Not in Kolab XML
    break;
  }
  int howMany = recur->duration();
  if ( howMany > 0 ) {
    mRecurrence.rangeType = "number";
    mRecurrence.range = QString::number( howMany );
  } else if ( howMany == 0 ) {
    mRecurrence.rangeType = "date";
    mRecurrence.range = dateToString( recur->endDate() );
  } else {
    mRecurrence.rangeType = "none";
  }
}

void Incidence::setFields( const KCal::Incidence* incidence )
{
  KolabBase::setFields( incidence );

  if ( incidence->doesFloat() ) {
    // This is a floating event. Don't timezone move this one
    mFloatingStatus = AllDay;
    setStartDate( incidence->dtStart().date() );
  } else {
    mFloatingStatus = HasTime;
    setStartDate( localToUTC( incidence->dtStart() ) );
  }

  setSummary( incidence->summary() );
  setLocation( incidence->location() );

  // Alarm
  mHasAlarm = false; // Will be set to true, if we actually have one
  if ( incidence->isAlarmEnabled() ) {
    const KCal::Alarm::List& alarms = incidence->alarms();
    if ( !alarms.isEmpty() ) {
      const KCal::Alarm* alarm = alarms.first();
      if ( alarm->hasStartOffset() ) {
        int dur = alarm->startOffset().asSeconds();
        setAlarm( (float)dur / 60.0 );
      }
    }
  }

  Email org( incidence->organizer().name(), incidence->organizer().email() );
  setOrganizer( org );

  // Attendees:
  KCal::Attendee::List attendees = incidence->attendees();
  KCal::Attendee::List::ConstIterator it;
  for ( it = attendees.begin(); it != attendees.end(); ++it ) {
    KCal::Attendee* kcalAttendee = *it;
    Attendee attendee;

    attendee.displayName = kcalAttendee->name();
    attendee.smtpAddress = kcalAttendee->email();
    attendee.status = attendeeStatusToString( kcalAttendee->status() );
    attendee.requestResponse = kcalAttendee->RSVP();
    // TODO: KCal::Attendee::mFlag is not accessible
    // attendee.invitationSent = kcalAttendee->mFlag;
    // DF: Hmm? mFlag is set to true and never used at all.... Did you mean another field?
    attendee.role = attendeeRoleToString( kcalAttendee->role() );

    addAttendee( attendee );
  }

  mAttachments.clear();

  // Attachments
  KCal::Attachment::List attachments = incidence->attachments();
  KCal::Attachment::List::ConstIterator it2;
  for ( it2 = attachments.begin(); it2 != attachments.end(); ++it2 ) {
    KCal::Attachment *a = *it2;
    mAttachments.push_back( a );
  }

  if ( incidence->doesRecur() ) {
    setRecurrence( incidence->recurrence() );
    mRecurrence.exclusions = incidence->exDates();
  }

  // Unhandled tags and other custom properties (see libkcal/customproperties.h)
  const QMap<QCString, QString> map = incidence->customProperties();
  QMap<QCString, QString>::ConstIterator cit = map.begin();
  for ( ; cit != map.end() ; ++cit ) {
    Custom c;
    c.key = cit.key();
    c.value = cit.data();
    mCustomList.append( c );
  }
}

static QBitArray daysListToBitArray( const QStringList& days )
{
  QBitArray arr( 7 );
  arr.fill( false );
  for( QStringList::ConstIterator it = days.begin(); it != days.end(); ++it ) {
    for ( uint i = 0; i < 7 ; ++i )
      if ( *it == s_weekDayName[i] )
        arr.setBit( i, true );
  }
  return arr;
}


void Incidence::saveTo( KCal::Incidence* incidence )
{
  KolabBase::saveTo( incidence );

  if ( mFloatingStatus == AllDay ) {
    // This is a floating event. Don't timezone move this one
    incidence->setDtStart( startDate() );
    incidence->setFloats( true );
  } else {
    incidence->setDtStart( utcToLocal( startDate() ) );
    incidence->setFloats( false );
  }

  incidence->setSummary( summary() );
  incidence->setLocation( location() );

  if ( mHasAlarm ) {
    KCal::Alarm* alarm = incidence->newAlarm();
    alarm->setStartOffset( qRound( mAlarm * 60.0 ) );
    alarm->setEnabled( true );
  }

  if ( organizer().displayName.isEmpty() )
    incidence->setOrganizer( organizer().smtpAddress );
  else
    incidence->setOrganizer( organizer().displayName + "<"
                             + organizer().smtpAddress + ">" );

  incidence->clearAttendees();
  QValueList<Attendee>::ConstIterator it;
  for ( it = mAttendees.begin(); it != mAttendees.end(); ++it ) {
    KCal::Attendee::PartStat status = attendeeStringToStatus( (*it).status );
    KCal::Attendee::Role role = attendeeStringToRole( (*it).role );
    incidence->addAttendee( new KCal::Attendee( (*it).displayName,
                                                (*it).smtpAddress,
                                                (*it).requestResponse,
                                                status, role ) );
  }

  incidence->clearAttachments();
  KCal::Attachment::List::ConstIterator it2;
  for ( it2 = mAttachments.begin(); it2 != mAttachments.end(); ++it2 ) {
    KCal::Attachment *a = (*it2);
    // TODO should we copy?
    incidence->addAttachment( a );
  }

  if ( !mRecurrence.cycle.isEmpty() ) {
    KCal::Recurrence* recur = incidence->recurrence(); // yeah, this creates it
    // done below recur->setFrequency( mRecurrence.interval );
    if ( mRecurrence.cycle == "minutely" ) {
      recur->setMinutely( mRecurrence.interval, -1 );
    } else if ( mRecurrence.cycle == "hourly" ) {
      recur->setHourly( mRecurrence.interval, -1 );
    } else if ( mRecurrence.cycle == "daily" ) {
      recur->setDaily( mRecurrence.interval, -1 );
    } else if ( mRecurrence.cycle == "weekly" ) {
      QBitArray rDays = daysListToBitArray( mRecurrence.days );
      recur->setWeekly( mRecurrence.interval, rDays, -1 );
    } else if ( mRecurrence.cycle == "monthly" ) {
      if ( mRecurrence.type == "weekday" ) {
        recur->setMonthly( KCal::Recurrence::rMonthlyPos, mRecurrence.interval, -1 );
        recur->addMonthlyPos( mRecurrence.dayNumber.toInt(), daysListToBitArray( mRecurrence.days ) );
      } else if ( mRecurrence.type == "daynumber" ) {
        recur->setMonthly( KCal::Recurrence::rMonthlyDay, mRecurrence.interval, -1 );
        recur->addMonthlyDay( mRecurrence.date.toInt() );
      } else kdWarning() << "Unhandled monthly recurrence type " << mRecurrence.type << endl;
    } else if ( mRecurrence.cycle == "yearly" ) {
      if ( mRecurrence.type == "monthday" ) {
        recur->setYearly( KCal::Recurrence::rYearlyMonth, mRecurrence.interval, -1 );
        recur->setYearlyByDate( mRecurrence.date.toInt(),
                                KCal::Recurrence::rMar1, // whichever
                                mRecurrence.interval,
                                -1 );
        for ( int i = 0; i < 12; ++i )
          if ( s_monthName[ i ] == mRecurrence.month )
            recur->addYearlyNum( i );
      } else if ( mRecurrence.type == "yearday" ) {
        recur->setYearly( KCal::Recurrence::rYearlyDay, mRecurrence.interval, -1 );
        recur->addYearlyNum( mRecurrence.dayNumber.toInt() );
      } else kdWarning() << "Unhandled yearly recurrence type " << mRecurrence.type << endl;
    } else kdWarning() << "Unhandled recurrence cycle " << mRecurrence.cycle << endl;

    if ( mRecurrence.rangeType == "number" ) {
      recur->setDuration( mRecurrence.range.toInt() );
    } else if ( mRecurrence.rangeType == "date" ) {
      recur->setEndDate( stringToDate( mRecurrence.range ) );
    } // "none" is default since -1 is passed everywhere above

    incidence->setExDates( mRecurrence.exclusions );
  }

  for( QValueList<Custom>::ConstIterator it = mCustomList.begin(); it != mCustomList.end(); ++it ) {
    incidence->setNonKDECustomProperty( (*it).key, (*it).value );
  }

}

QString Incidence::productID() const
{
  return QString( "KOrganizer " ) + korgVersion + ", Kolab resource";
}

// Unhandled KCal::Incidence fields:
// revision, status (unused), priority (done in tasks), attendee.uid,
// mComments, mReadOnly
