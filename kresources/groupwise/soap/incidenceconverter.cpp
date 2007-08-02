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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <klocale.h>
#include <kcodecs.h>
#include <libkdepim/kpimprefs.h>
#include <kcal/event.h>
#include <kcal/journal.h>
#include <kcal/recurrence.h>
#include <kcal/kcalversion.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addressee.h>
#include <kdebug.h>

#include "incidenceconverter.h"
//Added by qt3to4:
#include <QList>
#include <Q3PtrList>

#define GW_MAX_RECURRENCES 50

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
  kDebug() <<"IncidenceConverter::convertFromAppointment()";
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

    if ( appointment->startDay != 0 )
      event->setDtStart( QDateTime::fromString( QString::fromUtf8( appointment->startDay->c_str() ), Qt::ISODate ) );

    if ( appointment->endDay != 0 )
//TODO: Use KDateTime::fromString()?
      event->setDtEnd( KDateTime( QDateTime::fromString( QString::fromUtf8( appointment->endDay->c_str() ), Qt::ISODate ), mTimezone ).addDays( -1 ) );
    kDebug() <<" all day event.";
  }
  else
  {
    event->setFloats( false );

    if ( appointment->startDate != 0 )
      event->setDtStart( charToKDateTime( appointment->startDate, mTimezone ) );

    if ( appointment->endDate != 0 )
      event->setDtEnd( charToKDateTime( appointment->endDate, mTimezone ) );
  }

  kDebug() <<"start date:" << event->dtStart();
  kDebug() <<"end date:" << event->dtEnd();

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
  kDebug() <<"IncidenceConverter::convertToAppointment()";
  if ( !event )
    return 0;

  ngwt__Appointment* appointment = soap_new_ngwt__Appointment( soap(), -1 );
  appointment->startDate = 0;
  appointment->endDate = 0;
  appointment->startDay = 0;
  appointment->endDay = 0;
  appointment->acceptLevel = 0;
  appointment->alarm = 0;
  appointment->allDayEvent = 0;
  appointment->place = 0;
  appointment->timezone = 0;

  if ( !convertToCalendarItem( event, appointment ) ) {
    soap_dealloc( soap(), appointment );
    return 0;
  }

  if ( event->floats() ) {
    bool *allDayEvent = (bool*)soap_malloc( soap(), 1 );
    (*allDayEvent ) = true;

    appointment->allDayEvent = allDayEvent;

    if ( event->dtStart().isValid() )
//      appointment->startDate = qDateToChar( event->dtStart().date() );
      appointment->startDay = qDateToString( event->dtStart().date() );
    if ( event->hasEndDate() )
//      appointment->endDate = qDateToChar( event->dtEnd().date() );
      appointment->endDay = qDateToString( event->dtEnd().date() );
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

  if ( task->startDate ) {
    todo->setHasStartDate( true );
    todo->setDtStart( stringToKDateTime( task->startDate, mTimezone ) );
  }

  if ( task->dueDate ) {
    todo->setHasDueDate( true );
    todo->setDtDue( stringToKDateTime( task->dueDate, mTimezone ) );
  }

  if ( task->taskPriority ) {
    QString priority = stringToQString( task->taskPriority );

    // FIXME: Store priority string somewhere

    int p = priority.toInt();
    if ( p == 0 ) p = 3;

    todo->setPriority( p );
  }

  if ( task->completed && (*task->completed) == true )
    todo->setCompleted( true );

  todo->setLocation( i18n( "Novell GroupWise does not support locations for to-dos." ) );
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

KCal::Journal* IncidenceConverter::convertFromNote( ngwt__Note* note)
{
  if ( !note )
    return 0;

  KCal::Journal *journal = new KCal::Journal();

  if ( !convertFromCalendarItem( note, journal ) ) {
    delete journal;
    return 0;
  }

  if ( note->startDate ) {
    journal->setDtStart( stringToKDateTime( note->startDate, mTimezone ) );
  }

  return journal;
}

ngwt__Note* IncidenceConverter::convertToNote( KCal::Journal* journal )
{
  if ( !journal )
    return 0;
  ngwt__Note* note = soap_new_ngwt__Note( soap(), -1 );
  note->startDate = 0;

  if ( !convertToCalendarItem( journal, note ) ) {
    soap_dealloc( soap(), note );
    return 0;
  }

  if ( journal->floats() ) {
    if ( journal->dtStart().isValid() )
      note->startDate = qDateToString( journal->dtStart().date() );
  } else {
    if ( journal->dtStart().isValid() )
      note->startDate = qDateTimeToString( journal->dtStart(), mTimezone );
  }

  if ( !note->subject )
    note->subject = qStringToString( QString("NO SUBJECT") );
  return note;
}

bool IncidenceConverter::convertToCalendarItem( KCal::Incidence* incidence, ngwt__CalendarItem* item )
{
  kDebug() << k_funcinfo;
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
  item->subType = 0;
  // ngwt__BoxEntry
  item->status = 0;
  item->thread = 0;
  item->msgId = 0;
  item->messageId = 0;
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
    containerRef->__item = incidence->customProperty( "GWRESOURCE", "CONTAINER" ).toUtf8().data()/*.utf8()*/;
    container->push_back( containerRef );

    item->container = *container;
  }

  // secrecy
  item->class_ = (ngwt__ItemClass *)soap_malloc( soap(), sizeof( ngwt__ItemClass ) );
  switch ( incidence->secrecy() )
  {
    case KCal::Event::SecrecyPublic:
      *item->class_ = Public;
      break;
    case KCal::Event::SecrecyPrivate:
      *item->class_ = Private;
      break;
    case KCal::Event::SecrecyConfidential:
      *item->class_ = Private;
      break;
  }

  // options
  item->options = soap_new_ngwt__ItemOptions( soap(), -1 );
  item->options->concealSubject = 0;
  item->options->delayDeliveryUntil = 0;
  item->options->expires = 0;
  item->options->hidden = 0;
  item->options->priority = Standard;

  // summary
  if ( !incidence->summary().isEmpty() )
    item->subject = qStringToString( incidence->summary() );

// TODO: reinstate when we know that this isn't causing problems with recurrence
//   if ( incidence->created().isValid() ) {
//     item->created = qDateTimeToChar( incidence->created(), mTimezone );
//   } else
//     item->created = 0;

//   if ( incidence->lastModified().isValid() )
//     item->modified = qDateTimeToChar( incidence->lastModified(), mTimezone );

  setItemDescription( incidence, item );

  item->source = (ngwt__ItemSource *)soap_malloc( soap(), sizeof( ngwt__ItemSource ) );
#if 1
  if ( incidence->attendeeCount() > 0 ) {
    setAttendees( incidence, item );
    *item->source = sent_;
  }
  else
    *item->source = personal_;
#endif

  setRecurrence( incidence, item );
  return true;
}

void IncidenceConverter::setAttendees( KCal::Incidence *incidence,
  ngwt__CalendarItem *item )
{
  item->distribution = soap_new_ngwt__Distribution( soap(), -1 );

  item->distribution->from = soap_new_ngwt__From( soap(), -1 );

  // ngwt__From
  item->distribution->from->replyTo = 0;
  // ngwt__NameAndEmail
  item->distribution->from->displayName = 0;
  item->distribution->from->email = 0;
  item->distribution->from->uuid = 0;

  item->distribution->from->displayName = qStringToString( incidence->organizer().name() );
  item->distribution->from->email = qStringToString( incidence->organizer().email() );

  if ( !mFromName.isEmpty() ) item->distribution->from->displayName = qStringToString( mFromName );
  if ( !mFromEmail.isEmpty() ) item->distribution->from->email = qStringToString( mFromEmail );
  if ( !mFromUuid.isEmpty() ) item->distribution->from->uuid = qStringToString( mFromUuid );

  QString to; // To list consists of display names of organizer and attendees separated by ";  "
  to += incidence->organizer().name();
  item->distribution->sendoptions = soap_new_ngwt__SendOptions( soap(), -1 );

  item->distribution->sendoptions->requestReply = 0;
  item->distribution->sendoptions->mimeEncoding = 0;
  item->distribution->sendoptions->notification = 0;

  item->distribution->sendoptions->statusTracking = soap_new_ngwt__StatusTracking( soap(),
    -1 );

  item->distribution->sendoptions->statusTracking->autoDelete = false;
  item->distribution->sendoptions->statusTracking->__item = All_;

  item->distribution->recipients = soap_new_ngwt__RecipientList( soap(), -1 );
  item->distribution->recipients->recipient = *( soap_new_std__vectorTemplateOfPointerTongwt__Recipient( soap(), -1 ) );

  KCal::Attendee::List attendees = incidence->attendees();
  KCal::Attendee::List::ConstIterator it;
  for( it = attendees.begin(); it != attendees.end(); ++it ) {
    if ( !to.isEmpty() )
      to += QString::fromLatin1( ";  %1" ).arg( (*it)->name() );
    kDebug() <<"IncidenceConverter::setAttendees(), adding" << (*it)->fullName()
      << endl;
    QString uuid;
    QList<KABC::Addressee> addList = KABC::StdAddressBook::self()->findByEmail( (*it)->email() );
    if ( !addList.first().isEmpty() )
      uuid = addList.first().custom( "GWRESOURCE", "UUID" ); //uuid may be mandatory for the recipients list to be stored on the server...
    item->distribution->recipients->recipient.push_back( createRecipient( (*it)->name(), (*it)->email(), uuid ) );
  }
  item->distribution->to = qStringToString( to );
  item->distribution->cc = 0;
}

ngwt__Recipient *IncidenceConverter::createRecipient( const QString &name,
  const QString &email, const QString &uuid )
{
  ngwt__Recipient *recipient = soap_new_ngwt__Recipient( soap(), -1 );

  recipient->recipientStatus = 0;
  if ( !uuid.isEmpty() ) recipient->uuid = qStringToString( uuid );
  else recipient->uuid = 0;
  if ( !name.isEmpty() ) {
    kDebug() <<"- recipient name:" << name;
    recipient->displayName = qStringToString( name );
  } else {
    recipient->displayName = 0;
  }
  if ( !email.isEmpty() ) {
    kDebug() <<"- recipient email:" << email;
    recipient->email = qStringToString( email );
  } else {
    recipient->email = 0;
  }
  recipient->distType = TO;
  recipient->recipType = User;
  return recipient;
}

bool IncidenceConverter::convertFromCalendarItem( ngwt__CalendarItem* item,
  KCal::Incidence* incidence )
{
  incidence->setCustomProperty( "GWRESOURCE", "UID",
                                stringToQString( item->id ) );

  if ( item->subject && !item->subject->empty() )
    incidence->setSummary( stringToQString( item->subject ) );

  kDebug() <<"SUMMARY:" << incidence->summary();

  if ( item->created )
    incidence->setCreated( charToKDateTime( item->created, mTimezone ) );

  if ( item->modified != 0 )
    incidence->setLastModified( charToKDateTime( item->modified, mTimezone ) );

  getItemDescription( item, incidence );
  getAttendees( item, incidence );

  if ( item->recurrenceKey )
    incidence->setCustomProperty( "GWRESOURCE", "RECURRENCEKEY", QString::number( *item->recurrenceKey ) );

/*
  // This must just be a very early cut at recurrence
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
        kDebug() <<"Incidence description decodes to:" << description;
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
    data.id = data.type = data.option = 0;
    data.soap = 0;

    part->id = 0;
    part->__item = data;
    part->contentId = 0;
    std::string *str = soap_new_std__string( soap(), -1 );
    str->append( "text/plain" );
    part->contentType = str;

    part->length = 0; // this is optional and sending the actual length of the source string truncates the data.

    message->part.push_back( part );

    item->message = message;
  } else
    item->message = 0;
}

void IncidenceConverter::getAttendees( ngwt__CalendarItem *item, KCal::Incidence *incidence )
{
//   kDebug() <<"IncidenceConverter::getAttendees()" << ( item->subject ? item->subject->c_str() :"no subject" )
//     << endl;

  if ( item->distribution && item->distribution->from ) {
/*    kDebug() <<"-- from";*/
    KCal::Person organizer( stringToQString( item->distribution->from->displayName ),
                            stringToQString( item->distribution->from->email ) );
    incidence->setOrganizer( organizer );
  }

  if ( item->distribution && item->distribution->recipients ) {
/*    kDebug() <<"-- recipients";*/
    std::vector<ngwt__Recipient*> recipients = item->distribution->recipients->recipient;
    std::vector<ngwt__Recipient*>::const_iterator it;

    for ( it = recipients.begin(); it != recipients.end(); ++it ) {
/*      kDebug() <<"---- recipient";
 */   ngwt__Recipient *recipient = *it;
      KCal::Attendee *attendee = new KCal::Attendee(
        stringToQString( recipient->displayName ),
        stringToQString( recipient->email ) );

      // set our status
      if ( *(recipient->email) == *(qStringToString( mFromEmail )) )
        if ( item->status->accepted )
          attendee->setStatus( ( *item->status->accepted ) ? KCal::Attendee::Accepted : KCal::Attendee::NeedsAction );

      incidence->addAttendee( attendee );
    }
  }
}

void IncidenceConverter::setRecurrence( KCal::Incidence * incidence, ngwt__CalendarItem * item )
{
  kDebug() << k_funcinfo;
  ngwt__Frequency * freq;
  const KCal::Recurrence * recur = incidence->recurrence();

  if ( incidence->doesRecur() )
  {
    item->rrule = soap_new_ngwt__RecurrenceRule( soap(), -1 );
    item->rrule->frequency = 0; //
    item->rrule->count = 0;
    item->rrule->until = 0; //
    item->rrule->interval = 0; //
    item->rrule->byDay = 0;
    item->rrule->byYearDay = 0;
    item->rrule->byMonth = 0;
    freq = (ngwt__Frequency *)soap_malloc( soap(), sizeof( ngwt__Frequency ) );
    // interval
    if ( recur->frequency() > 1 ) {
      item->rrule->interval = (unsigned long *)soap_malloc( soap(), sizeof( unsigned long * ) );
      *item->rrule->interval = recur->frequency();
    }
    // end date
    if ( recur->duration() > 0 )     // number of recurrences. If end date is set we don't use this.
    {
      item->rrule->count = (long unsigned int *)soap_malloc( soap(), sizeof( long unsigned int * ) );
      *item->rrule->count = recur->duration();
    }
#if LIBKCAL_IS_VERSION( 1, 3, 0 )
    else if ( recur->endDateTime().isValid() )
#else
		else if ( recur->endDate().isValid() )
#endif
      item->rrule->until = qDateToString( recur->endDate() );
    else // GROUPWISE doesn't accept infinite recurrence so end after GW_MAX_RECURRENCES recurrences
    {
      item->rrule->count = (long unsigned int *)soap_malloc( soap(), sizeof( long unsigned int * ) );
      *item->rrule->count = GW_MAX_RECURRENCES;
    }

    // recurrence date - try setting it using the recurrence start date - didn't help
/*    std::string startDate;
    startDate.append( recur->recurStart().date().toString( Qt::ISODate ).utf8() );
    item->rdate = soap_new_ngwt__RecurrenceDateType( soap(), -1 );
    item->rdate->date.push_back( startDate );*/
    // exceptions list - try sending empty list even if no exceptions
#if LIBKCAL_IS_VERSION( 1, 3, 0 )
    KCal::DateList exceptions = recur->exDates();
#else
		KCal::DateList exceptions = incidence->exDates();
#endif
    if ( !exceptions.isEmpty() )
    {
      item->exdate = soap_new_ngwt__RecurrenceDateType( soap(), -1 );
      for ( KCal::DateList::ConstIterator it = exceptions.begin(); it != exceptions.end(); ++it )
      {
        std::string startDate;
        startDate.append( (*it).toString( Qt::ISODate ).utf8() );
        item->exdate->date.push_back( startDate );
      }
    }
  }

#if LIBKCAL_IS_VERSION( 1, 3, 0 )
  if ( incidence->recurrenceType() == KCal::Recurrence::rDaily )
#else
  if ( incidence->doesRecur() == KCal::Recurrence::rDaily )
#endif
  {
    kDebug() <<"incidence recurs daily";
    *freq = Daily;
    item->rrule->frequency = freq;
  }
#if LIBKCAL_IS_VERSION( 1, 3, 0 )
  else if ( incidence->recurrenceType() == KCal::Recurrence::rWeekly )
#else
  else if ( incidence->doesRecur() == KCal::Recurrence::rWeekly )
#endif
  {
    kDebug() <<"incidence recurs weekly";
    *freq = Weekly;
    item->rrule->frequency = freq;
    // now change the bitArray of the days of the week that it recurs on to a ngwt__DayOfWeekList *
    QBitArray ba = recur->days();
    ngwt__DayOfWeekList * weeklyDays = soap_new_ngwt__DayOfWeekList( soap(), -1 );
    for ( int i = 0; i < 7; ++i )
    {
      if ( ba[i] )
      {
        ngwt__DayOfWeek * day = soap_new_ngwt__DayOfWeek( soap(), -1 );
        day->occurrence = 0;
        switch( i )
        {
          case 0:
            day->__item = Monday;
            break;
          case 1:
            day->__item = Tuesday;
            break;
          case 2:
            day->__item = Wednesday;
            break;
          case 3:
            day->__item = Thursday;
            break;
          case 4:
            day->__item = Friday;
            break;
          case 5:
            day->__item = Saturday;
            break;
          case 6:
            day->__item = Sunday;
            break;
        }
        weeklyDays->day.push_back( day );
      }
    }
    // add the list of days to the recurrence rule
    item->rrule->byDay = weeklyDays;
  }
#if LIBKCAL_IS_VERSION( 1, 3, 0 )
   else if ( incidence->recurrenceType() == KCal::Recurrence::rMonthlyDay )
#else
   else if ( incidence->doesRecur() == KCal::Recurrence::rMonthlyDay )
#endif
  {
    kDebug() <<"incidence recurs monthly";
    ;
    *freq = Monthly;
    item->rrule->frequency = freq;

    // TODO: translate '3rd wednesday of month' etc into rdates
  }
#if LIBKCAL_IS_VERSION( 1, 3, 0 )
  else if ( incidence->recurrenceType() == KCal::Recurrence::rYearlyDay )
#else
  else if ( incidence->doesRecur() == KCal::Recurrence::rYearlyDay )
#endif
  {
    kDebug() <<"incidence recurs yearly on day #";
    *freq = Yearly;
    item->rrule->frequency = freq;
    // TODO: translate '1st sunday in may'
    ngwt__DayOfYearList * daysOfYear = soap_new_ngwt__DayOfYearList( soap(), -1 );
#if LIBKCAL_IS_VERSION( 1, 3, 0 )
    QList<int> rmd;
    rmd = recur->yearMonths();
    daysOfYear->day.push_back( rmd.first() );
#else
    Q3PtrList<int> rmd;
    rmd = recur->yearNums();
    daysOfYear->day.push_back( *rmd.first() );
#endif

    item->rrule->byYearDay = daysOfYear;
    // no need to do MonthList recurrence as these will appear as separate instances when fetched from GW
  }
#if LIBKCAL_IS_VERSION( 1, 3, 0 )
  else if ( incidence->recurrenceType() == KCal::Recurrence::rYearlyMonth )
#else
  else if ( incidence->doesRecur() == KCal::Recurrence::rYearlyMonth )
#endif
  {
    kDebug() <<"incidence recurs yearly on monthday";
    *freq = Yearly;
    item->rrule->frequency = freq;
  }
}
