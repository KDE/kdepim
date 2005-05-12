/*
    This file is part of kdepim.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include "incidenceconverter.h"

#include <klocale.h>
#include <kmdcodec.h>
#include <libkdepim/kpimprefs.h>
#include <libkcal/event.h>

#include <kdebug.h>

IncidenceConverter::IncidenceConverter( struct soap* soap )
  : GWConverter( soap )
{
  mTimezone = KPimPrefs::timezone();
}

void IncidenceConverter::setFrom( const QString &name,
  const QString &email, const QString &uuid )
{
  mFromName = name;
  mFromEmail = email;
  mFromUuid = uuid;
}

KCal::Event* IncidenceConverter::convertFromAppointment( ngwt__Appointment* appointment )
{
  if ( !appointment )
    return 0;

  KCal::Event *event = new KCal::Event();

  if ( !convertFromCalendarItem( appointment, event ) ) {
    delete event;
    return 0;
  }

  if ( appointment->allDayEvent && (*appointment->allDayEvent) )
  {
    event->setFloats( true );
    // startDate actually belongs to CalendarItem, but the way it is interpreted depends on 
    // whether allDayEvent from Appointment is true
    if ( appointment->startDate != 0 )
      event->setDtStart( charToQDate( appointment->startDate ) );

    if ( appointment->endDate != 0 )
      event->setDtEnd( charToQDate( appointment->endDate ).addDays( -1 ) );
  }
  else
  {
    event->setFloats( false );

    if ( appointment->startDate != 0 )
      event->setDtStart( charToQDateTime( appointment->startDate, mTimezone ) );

    if ( appointment->endDate != 0 )
      event->setDtEnd( charToQDateTime( appointment->endDate, mTimezone ) );
  }


  if ( appointment->alarm ) {
    KCal::Alarm *alarm = event->newAlarm();
    alarm->setStartOffset( appointment->alarm->__item * -1 );
    alarm->setEnabled( appointment->alarm->enabled );
  }

  if ( appointment->place )
    event->setLocation( stringToQString( appointment->place ) );

  if ( appointment->acceptLevel ) {
    if ( *appointment->acceptLevel == Tentative )
      event->setTransparency( KCal::Event::Transparent );
    else
      event->setTransparency( KCal::Event::Opaque );
  }

  return event;
}

ngwt__Appointment* IncidenceConverter::convertToAppointment( KCal::Event* event )
{
  if ( !event )
    return 0;

  ngwt__Appointment* appointment = soap_new_ngwt__Appointment( soap(), -1 );
  appointment->startDate = 0;
  appointment->endDate = 0;
  appointment->acceptLevel = 0;
  appointment->alarm = 0;
  appointment->allDayEvent = 0;
  appointment->place = 0;
  appointment->timezone = 0;

  if ( !convertToCalendarItem( event, appointment ) ) {
    soap_dealloc( soap(), appointment );
    return 0;
  }

  if ( event->doesFloat() ) {
    bool *allDayEvent = (bool*)soap_malloc( soap(), 1 );
    (*allDayEvent ) = true;

    appointment->allDayEvent = allDayEvent;

    if ( event->dtStart().isValid() )
      appointment->startDate = qDateToChar( event->dtStart().date() );

    if ( event->hasEndDate() )
      appointment->endDate = qDateToChar( event->dtEnd().date() );
  } else {
    appointment->allDayEvent = 0;

    if ( event->dtStart().isValid() )
      appointment->startDate = qDateTimeToChar( event->dtStart(), mTimezone );

    if ( event->hasEndDate() )
      appointment->endDate = qDateTimeToChar( event->dtEnd(), mTimezone );
  }

  enum ngwt__AcceptLevel * al = (enum ngwt__AcceptLevel*)soap_malloc(soap(), sizeof(enum ngwt__AcceptLevel));
  *al = Busy;
  appointment->acceptLevel = al;

  KCal::Alarm::List alarms = event->alarms();
  if ( !alarms.isEmpty() ) {
    ngwt__Alarm* alarm = soap_new_ngwt__Alarm( soap(), -1 );
    alarm->__item = alarms.first()->startOffset().asSeconds() * -1;
    bool * enabled = (bool *)soap_malloc(soap(), sizeof(bool));
    *enabled = alarms.first()->enabled();
    alarm->enabled = enabled;

    appointment->alarm = alarm;
  } else
    appointment->alarm = 0;

  if ( !event->location().isEmpty() ) {
    std::string* location = qStringToString( event->location() );

    appointment->place = location;
  } else
    appointment->place = 0;

  appointment->timezone = 0;

  return appointment;
}

KCal::Todo* IncidenceConverter::convertFromTask( ngwt__Task* task )
{
  if ( !task )
    return 0;

  KCal::Todo *todo = new KCal::Todo();

  if ( !convertFromCalendarItem( task, todo ) ) {
    delete todo;
    return 0;
  }

  if ( task->startDate != 0 )
    todo->setDtStart( QDate::fromString( stringToQString( task->startDate), Qt::ISODate ) );

  todo->setDtDue( QDate::fromString( stringToQString( task->dueDate ), Qt::ISODate ) );

  if ( task->taskPriority ) {
    QString priority = stringToQString( task->taskPriority );

    // FIXME: Store priority string somewhere

    int p = priority.toInt();
    if ( p == 0 ) p = 3;
    
    todo->setPriority( p );
  }

  if ( task->completed && (*task->completed) == true )
    todo->setCompleted( true );

  todo->setLocation( i18n( "Novell GroupWise does not support locations for To-dos." ) );
  return todo;
}

ngwt__Task* IncidenceConverter::convertToTask( KCal::Todo* todo )
{
  if ( !todo )
    return 0;
  ngwt__Task* task = soap_new_ngwt__Task( soap(), -1 );
  task->startDate = 0;
  task->dueDate = 0;
  task->taskPriority = 0;
  task->completed = 0;

  if ( !convertToCalendarItem( todo, task ) ) {
    soap_dealloc( soap(), task );
    return 0;
  }

  if ( todo->dtStart().isValid() )
    task->startDate = qDateTimeToString( todo->dtStart(), mTimezone );

  if ( todo->hasDueDate() ) {
    task->dueDate = qDateTimeToString( todo->dtDue() );
  }

  // FIXME: Restore custom priorities
  QString priority = QString::number( todo->priority() );
  task->taskPriority = qStringToString( priority );

  task->completed = (bool*)soap_malloc( soap(), 1 );
  if ( todo->isCompleted() )
    (*task->completed) = true;
  else
    (*task->completed) = false;

  return task;
}

bool IncidenceConverter::convertToCalendarItem( KCal::Incidence* incidence, ngwt__CalendarItem* item )
{
  //TODO: support the new iCal standard recurrence rule

  // ngwt__CalendarItem
  item->rdate = 0;
  item->rrule = 0;
  item->exdate = 0;
  item->recurrenceKey = 0;
  item->iCalId = 0;
  // ngwt__Mail
  item->subject = 0;
  item->originalSubject = 0;
  item->subjectPrefix = 0;
  item->distribution = 0;
  item->message = 0;
  item->attachments = 0;
  item->options = 0;
  item->link = 0;
  item->hasAttachment = false;
  item->size = 0;
  // ngwt__BoxEntry
  item->status = 0;
  item->thread = 0;
  item->msgId = 0;
  item->source = 0;
  item->returnSentItemsId = 0;
  item->delivered = 0;
  item->class_ = 0;
  item->security = 0;
  item->comment = 0;
  // ngwt__ContainerItem
  item->categories = 0;
  item->created = 0;
  item->customs = 0;
  // ngwt__Item
  item->id = 0;
  item->name = 0;
  item->version = 0;
  item->modified = 0;
  item->changes = 0;

  QString id = incidence->customProperty( "GWRESOURCE", "UID" );
  if ( !id.isEmpty() ) item->id = qStringToString( id );

  // Container
  if ( !incidence->customProperty( "GWRESOURCE", "CONTAINER" ).isEmpty() ) {
    std::vector<ngwt__ContainerRef*>* container = soap_new_std__vectorTemplateOfPointerTongwt__ContainerRef( soap(), -1 );
    ngwt__ContainerRef* containerRef = soap_new_ngwt__ContainerRef( soap(), -1 );
    containerRef->deleted = 0;
    containerRef->__item = incidence->customProperty( "GWRESOURCE", "CONTAINER" ).utf8();
    container->push_back( containerRef );

    item->container = *container;
  }

  if ( !incidence->summary().isEmpty() )
    item->subject = qStringToString( incidence->summary() );

  if ( incidence->created().isValid() ) {
    item->created = qDateTimeToChar( incidence->created(), mTimezone );
  } else
    item->created = 0;

  if ( incidence->lastModified().isValid() )
    item->modified = qDateTimeToChar( incidence->lastModified(), mTimezone );

  setItemDescription( incidence, item );

#if 1
  if ( incidence->attendeeCount() > 0 ) {
    setAttendees( incidence, item );
  }
#endif

  return true;
}

void IncidenceConverter::setAttendees( KCal::Incidence *incidence,
  ngwt__CalendarItem *item )
{
  ngwt__Distribution *dist = soap_new_ngwt__Distribution( soap(), -1 );
  item->distribution = dist;

  ngwt__From *from = soap_new_ngwt__From( soap(), -1 );
  dist->from = from;

  // ngwt__From
  from->replyTo = 0;
  // ngwt__NameAndEmail
  from->displayName = 0;
  from->email = 0;
  from->uuid = 0;

  from->displayName = qStringToString( incidence->organizer().name() );
  from->email = qStringToString( incidence->organizer().email() );

  if ( !mFromName.isEmpty() ) from->displayName = qStringToString( mFromName );
  if ( !mFromEmail.isEmpty() ) from->email = qStringToString( mFromEmail );
  if ( !mFromUuid.isEmpty() ) from->uuid = qStringToString( mFromUuid );

  QString to = "To";
  dist->to = qStringToString( to );
  dist->cc = 0;

  ngwt__SendOptions *sendOptions = soap_new_ngwt__SendOptions( soap(), -1 );
  dist->sendoptions = sendOptions;

  sendOptions->requestReply = 0;
  sendOptions->mimeEncoding = 0;
  sendOptions->notification = 0;

  ngwt__StatusTracking *statusTracking = soap_new_ngwt__StatusTracking( soap(),
    -1 );
  sendOptions->statusTracking = statusTracking;

  statusTracking->autoDelete = false;
  statusTracking->__item = All_;

  ngwt__RecipientList *recipientList = soap_new_ngwt__RecipientList( soap(), -1 );
  dist->recipients = recipientList;
  
  std::vector<ngwt__Recipient * > *recipients = 
    soap_new_std__vectorTemplateOfPointerTongwt__Recipient( soap(), -1 );
 
  recipients->push_back( createRecipient( mFromName, mFromEmail, mFromUuid ) );

  KCal::Attendee::List attendees = incidence->attendees();
  KCal::Attendee::List::ConstIterator it;
  for( it = attendees.begin(); it != attendees.end(); ++it ) {
    kdDebug() << "IncidenceConverter::setAttendees(), adding " << (*it)->fullName()
      << endl;
    recipients->push_back( createRecipient( (*it)->name(), (*it)->email() ) );
  }
 recipientList->recipient = *recipients;
}

ngwt__Recipient *IncidenceConverter::createRecipient( const QString &name,
  const QString &email, const QString &uuid )
{
  ngwt__Recipient *recipient = soap_new_ngwt__Recipient( soap(), -1 );

  recipient->recipientStatus = 0;
  if ( !uuid.isEmpty() ) recipient->uuid = qStringToString( uuid );
  else recipient->uuid = 0;
  if ( !name.isEmpty() ) {
    kdDebug() << "- recipient name: " << name << endl; 
    recipient->displayName = qStringToString( name );
  } else {
    recipient->displayName = 0;
  }
  if ( !email.isEmpty() ) {
    kdDebug() << "- recipient email: " << email << endl; 
    recipient->email = qStringToString( email );
  } else {
    recipient->email = 0;
  }
  recipient->distType = TO;
  recipient->recipType = User_;
  return recipient;
}

bool IncidenceConverter::convertFromCalendarItem( ngwt__CalendarItem* item,
  KCal::Incidence* incidence )
{
  incidence->setCustomProperty( "GWRESOURCE", "UID",
                                stringToQString( item->id ) );

  if ( item->subject && !item->subject->empty() )
    incidence->setSummary( stringToQString( item->subject ) );

//  kdDebug() << "SUMMARY: " << incidence->summary() << endl;

  if ( item->created )
    incidence->setCreated( charToQDateTime( item->created, mTimezone ) );

  if ( item->modified != 0 )
    incidence->setLastModified( charToQDateTime( item->modified, mTimezone ) );

  getItemDescription( item, incidence );
  getAttendees( item, incidence );
/*
  if ( item->rdate && item->rdate->date ) {
    std::vector<xsd__date>* dateList = item->rdate->date;

    std::vector<xsd__date>::const_iterator it;
    for ( it = dateList->begin(); it != dateList->end(); ++it ) {
      QDate date = QDate::fromString( s2q( *it ), Qt::ISODate );
      if ( date.isValid() )
    }
  }
*/
/*
    bool*                                isRecurring                    0;
    std::string*                         iCalId                         0;
*/

  return true;
}

void IncidenceConverter::getItemDescription( ngwt__CalendarItem *item, KCal::Incidence *incidence )
{
  if ( item->message ) {

    std::vector<ngwt__MessagePart*> parts = item->message->part;
    std::vector<ngwt__MessagePart*>::const_iterator it = parts.begin();

    for ( ; it != parts.end(); ++it ) {
      xsd__base64Binary data = (*it)->__item;

      // text/plain should be the description
      if ( stringToQString( (*it)->contentType ) == "text/plain" ) {
        QString description = QString::fromUtf8( (char*)data.__ptr, data.__size );
        incidence->setDescription( description );
        kdDebug() << "Incidence description decodes to: " << description << endl; 
        return;
      }
    }
  }
}

void IncidenceConverter::setItemDescription( KCal::Incidence *incidence,
  ngwt__CalendarItem *item )
{
  if ( !incidence->description().isEmpty() ) {
    ngwt__MessageBody *message = soap_new_ngwt__MessageBody( soap(), -1 );
    message->part =
      *soap_new_std__vectorTemplateOfPointerTongwt__MessagePart( soap(), -1 );

    ngwt__MessagePart *part = soap_new_ngwt__MessagePart( soap(), -1 );

    xsd__base64Binary data;
    data.__ptr =
      (unsigned char*)qStringToChar( incidence->description().utf8() );
    data.__size = incidence->description().utf8().length();

    part->__item = data;
    part->contentId = 0;
    std::string *str = soap_new_std__string( soap(), -1 );
    str->append( "text/plain" );
    part->contentType = str;

    int * len = (int*)soap_malloc( soap(), sizeof( int ) );
    *len = incidence->description().utf8().length();
    part->length = len;

    message->part.push_back( part );

    item->message = message;
  } else
    item->message = 0;
}

void IncidenceConverter::getAttendees( ngwt__CalendarItem *item, KCal::Incidence *incidence )
{
  kdDebug() << "IncidenceConverter::getAttendees()" << ( item->subject ? item->subject->c_str() : "no subject" )
    << endl;

  if ( item->distribution && item->distribution->from ) {
    kdDebug() << "-- from" << endl;
    KCal::Person organizer( stringToQString( item->distribution->from->displayName ),
                            stringToQString( item->distribution->from->email ) );
    incidence->setOrganizer( organizer );
  }

  if ( item->distribution && item->distribution->recipients ) {
    kdDebug() << "-- recipients" << endl;
    std::vector<ngwt__Recipient*> recipients = item->distribution->recipients->recipient;
    std::vector<ngwt__Recipient*>::const_iterator it;

    for ( it = recipients.begin(); it != recipients.end(); ++it ) {
      kdDebug() << "---- recipient " << endl;
      ngwt__Recipient *recipient = *it;
      KCal::Attendee *attendee = new KCal::Attendee(
        stringToQString( recipient->displayName ),
        stringToQString( recipient->email ) );

      incidence->addAttendee( attendee );
    }
  }
}
