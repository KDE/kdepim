/*
    This file is part of libkcal.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qfile.h>
#include <cstdlib>

#include <kdebug.h>
#include <klocale.h>
#include <kmdcodec.h>

extern "C" {
  #include <ical.h>
  #include <icalparser.h>
  #include <icalrestriction.h>
}

#include "calendar.h"
#include "journal.h"
#include "icalformat.h"
#include "icalformatimpl.h"
#include "compat.h"

#define _ICAL_VERSION "2.0"

using namespace KCal;

/* Static helpers */
static QDateTime ICalDate2QDate(const icaltimetype& t)
{
  // Outlook sends dates starting from 1601-01-01, but QDate()
  // can only handle dates starting 1752-09-14.
  const int year = (t.year>=1754) ? t.year : 1754;
  return QDateTime(QDate(year,t.month,t.day), QTime(t.hour,t.minute,t.second));
}

static void _dumpIcaltime( const icaltimetype& t)
{
  kdDebug(5800) << "--- Y: " << t.year << " M: " << t.month << " D: " << t.day
      << endl;
  kdDebug(5800) << "--- H: " << t.hour << " M: " << t.minute << " S: " << t.second
      << endl;
  kdDebug(5800) << "--- isUtc: " << icaltime_is_utc( t )<< endl;
  kdDebug(5800) << "--- zoneId: " << icaltimezone_get_tzid( const_cast<icaltimezone*>( t.zone ) )<< endl;
}

static QString quoteForParam( const QString &text )
{
  QString tmp = text;
  tmp.remove( '"' );
  if ( tmp.contains( ';' ) || tmp.contains( ':' ) || tmp.contains( ',' ) )
    return tmp; // libical quotes in this case already, see icalparameter_as_ical_string()
  return QString::fromLatin1( "\"" ) + tmp + QString::fromLatin1( "\"" );
}

const int gSecondsPerMinute = 60;
const int gSecondsPerHour   = gSecondsPerMinute * 60;
const int gSecondsPerDay    = gSecondsPerHour   * 24;
const int gSecondsPerWeek   = gSecondsPerDay    * 7;

ICalFormatImpl::ICalFormatImpl( ICalFormat *parent ) :
  mParent( parent ), mCompat( new Compat )
{
}

ICalFormatImpl::~ICalFormatImpl()
{
  delete mCompat;
}

class ICalFormatImpl::ToComponentVisitor : public IncidenceBase::Visitor
{
  public:
    ToComponentVisitor( ICalFormatImpl *impl, Scheduler::Method m ) : mImpl( impl ), mComponent( 0 ), mMethod( m ) {}

    bool visit( Event *e ) { mComponent = mImpl->writeEvent( e ); return true; }
    bool visit( Todo *e ) { mComponent = mImpl->writeTodo( e ); return true; }
    bool visit( Journal *e ) { mComponent = mImpl->writeJournal( e ); return true; }
    bool visit( FreeBusy *fb ) { mComponent = mImpl->writeFreeBusy( fb, mMethod ); return true; }

    icalcomponent *component() { return mComponent; }

  private:
    ICalFormatImpl *mImpl;
    icalcomponent *mComponent;
    Scheduler::Method mMethod;
};

icalcomponent *ICalFormatImpl::writeIncidence( IncidenceBase *incidence, Scheduler::Method method )
{
  ToComponentVisitor v( this, method );
  if ( incidence->accept(v) )
    return v.component();
  else return 0;
}

icalcomponent *ICalFormatImpl::writeTodo(Todo *todo)
{
  QString tmpStr;
  QStringList tmpStrList;

  icalcomponent *vtodo = icalcomponent_new(ICAL_VTODO_COMPONENT);

  writeIncidence(vtodo,todo);

  // due date
  if (todo->hasDueDate()) {
    icaltimetype due;
    if (todo->doesFloat()) {
      due = writeICalDate(todo->dtDue(true).date());
    } else {
      due = writeICalDateTime(todo->dtDue(true));
    }
    icalcomponent_add_property(vtodo,icalproperty_new_due(due));
  }

  // start time
  if ( todo->hasStartDate() || todo->doesRecur() ) {
    icaltimetype start;
    if (todo->doesFloat()) {
//      kdDebug(5800) << " Incidence " << todo->summary() << " floats." << endl;
      start = writeICalDate(todo->dtStart(true).date());
    } else {
//      kdDebug(5800) << " incidence " << todo->summary() << " has time." << endl;
      start = writeICalDateTime(todo->dtStart(true));
    }
    icalcomponent_add_property(vtodo,icalproperty_new_dtstart(start));
  }

  // completion date
  if (todo->isCompleted()) {
    if (!todo->hasCompletedDate()) {
      // If todo was created by KOrganizer <2.2 it has no correct completion
      // date. Set it to now.
      todo->setCompleted(QDateTime::currentDateTime());
    }
    icaltimetype completed = writeICalDateTime(todo->completed());
    icalcomponent_add_property(vtodo,icalproperty_new_completed(completed));
  }

  icalcomponent_add_property(vtodo,
      icalproperty_new_percentcomplete(todo->percentComplete()));

  if( todo->doesRecur() ) {
    icalcomponent_add_property(vtodo,
        icalproperty_new_recurrenceid( writeICalDateTime( todo->dtDue())));
  }

  return vtodo;
}

icalcomponent *ICalFormatImpl::writeEvent(Event *event)
{
#if 0
  kdDebug(5800) << "Write Event '" << event->summary() << "' (" << event->uid()
                << ")" << endl;
#endif

  QString tmpStr;
  QStringList tmpStrList;

  icalcomponent *vevent = icalcomponent_new(ICAL_VEVENT_COMPONENT);

  writeIncidence(vevent,event);

  // start time
  icaltimetype start;
  if (event->doesFloat()) {
//    kdDebug(5800) << " Incidence " << event->summary() << " floats." << endl;
    start = writeICalDate(event->dtStart().date());
  } else {
//    kdDebug(5800) << " incidence " << event->summary() << " has time." << endl;
    start = writeICalDateTime(event->dtStart());
  }
  icalcomponent_add_property(vevent,icalproperty_new_dtstart(start));

  if (event->hasEndDate()) {
    // End time.
    // RFC2445 says that if DTEND is present, it has to be greater than DTSTART.
    icaltimetype end;
    if (event->doesFloat()) {
//      kdDebug(5800) << " Event " << event->summary() << " floats." << endl;
      // +1 day because end date is non-inclusive.
      end = writeICalDate( event->dtEnd().date().addDays( 1 ) );
      icalcomponent_add_property(vevent,icalproperty_new_dtend(end));
    } else {
//      kdDebug(5800) << " Event " << event->summary() << " has time." << endl;
      if (event->dtEnd() != event->dtStart()) {
        end = writeICalDateTime(event->dtEnd());
        icalcomponent_add_property(vevent,icalproperty_new_dtend(end));
      }
    }
  }

// TODO: resources
#if 0
  // resources
  tmpStrList = anEvent->resources();
  tmpStr = tmpStrList.join(";");
  if (!tmpStr.isEmpty())
    addPropValue(vevent, VCResourcesProp, tmpStr.utf8());

#endif

  // Transparency
  switch( event->transparency() ) {
  case Event::Transparent:
    icalcomponent_add_property(
      vevent,
      icalproperty_new_transp( ICAL_TRANSP_TRANSPARENT ) );
    break;
  case Event::Opaque:
    icalcomponent_add_property(
      vevent,
      icalproperty_new_transp( ICAL_TRANSP_OPAQUE ) );
    break;
  }

  return vevent;
}

icalcomponent *ICalFormatImpl::writeFreeBusy(FreeBusy *freebusy,
                                             Scheduler::Method method)
{
#if QT_VERSION >= 300
  kdDebug(5800) << "icalformatimpl: writeFreeBusy: startDate: "
    << freebusy->dtStart().toString("ddd MMMM d yyyy: h:m:s ap") << " End Date: "
    << freebusy->dtEnd().toString("ddd MMMM d yyyy: h:m:s ap") << endl;
#endif

  icalcomponent *vfreebusy = icalcomponent_new(ICAL_VFREEBUSY_COMPONENT);

  writeIncidenceBase(vfreebusy,freebusy);

  icalcomponent_add_property(vfreebusy, icalproperty_new_dtstart(
      writeICalDateTime(freebusy->dtStart())));

  icalcomponent_add_property(vfreebusy, icalproperty_new_dtend(
      writeICalDateTime(freebusy->dtEnd())));

  if (method == Scheduler::Request) {
    icalcomponent_add_property(vfreebusy,icalproperty_new_uid(
       freebusy->uid().utf8()));
  }

  //Loops through all the periods in the freebusy object
  QValueList<Period> list = freebusy->busyPeriods();
  QValueList<Period>::Iterator it;
  icalperiodtype period = icalperiodtype_null_period();
  for (it = list.begin(); it!= list.end(); ++it) {
    period.start = writeICalDateTime((*it).start());
    if ( (*it).hasDuration() ) {
      period.duration = writeICalDuration( (*it).duration().asSeconds() );
    } else {
      period.end = writeICalDateTime((*it).end());
    }
    icalcomponent_add_property(vfreebusy, icalproperty_new_freebusy(period) );
  }

  return vfreebusy;
}

icalcomponent *ICalFormatImpl::writeJournal(Journal *journal)
{
  icalcomponent *vjournal = icalcomponent_new(ICAL_VJOURNAL_COMPONENT);

  writeIncidence(vjournal,journal);

  // start time
  if (journal->dtStart().isValid()) {
    icaltimetype start;
    if (journal->doesFloat()) {
//      kdDebug(5800) << " Incidence " << event->summary() << " floats." << endl;
      start = writeICalDate(journal->dtStart().date());
    } else {
//      kdDebug(5800) << " incidence " << event->summary() << " has time." << endl;
      start = writeICalDateTime(journal->dtStart());
    }
    icalcomponent_add_property(vjournal,icalproperty_new_dtstart(start));
  }

  return vjournal;
}

void ICalFormatImpl::writeIncidence(icalcomponent *parent,Incidence *incidence)
{
  // pilot sync stuff
// TODO: move this application-specific code to kpilot
  if (incidence->pilotId()) {
    // NOTE: we can't do setNonKDECustomProperty here because this changes
    // data and triggers an updated() event...
    // incidence->setNonKDECustomProperty("X-PILOTSTAT", QString::number(incidence->syncStatus()));
    // incidence->setNonKDECustomProperty("X-PILOTID", QString::number(incidence->pilotId()));

    icalproperty *p = 0;
    p = icalproperty_new_x(QString::number(incidence->syncStatus()).utf8());
    icalproperty_set_x_name(p,"X-PILOTSTAT");
    icalcomponent_add_property(parent,p);

    p = icalproperty_new_x(QString::number(incidence->pilotId()).utf8());
    icalproperty_set_x_name(p,"X-PILOTID");
    icalcomponent_add_property(parent,p);
  }

  if ( incidence->schedulingID() != incidence->uid() )
    // We need to store the UID in here. The rawSchedulingID will
    // go into the iCal UID component
    incidence->setCustomProperty( "LIBKCAL", "ID", incidence->uid() );
  else
    incidence->removeCustomProperty( "LIBKCAL", "ID" );

  writeIncidenceBase(parent,incidence);

  // creation date
  icalcomponent_add_property(parent,icalproperty_new_created(
      writeICalDateTime(incidence->created())));

  // unique id
  // If the scheduling ID is different from the real UID, the real
  // one is stored on X-REALID above
  if ( !incidence->schedulingID().isEmpty() ) {
    icalcomponent_add_property(parent,icalproperty_new_uid(
        incidence->schedulingID().utf8()));
  }

  // revision
  if ( incidence->revision() > 0 ) { // 0 is default, so don't write that out
    icalcomponent_add_property(parent,icalproperty_new_sequence(
        incidence->revision()));
  }

  // last modification date
  if ( incidence->lastModified().isValid() ) {
   icalcomponent_add_property(parent,icalproperty_new_lastmodified(
       writeICalDateTime(incidence->lastModified())));
  }

  // description
  if (!incidence->description().isEmpty()) {
    icalcomponent_add_property(parent,icalproperty_new_description(
        incidence->description().utf8()));
  }

  // summary
  if (!incidence->summary().isEmpty()) {
    icalcomponent_add_property(parent,icalproperty_new_summary(
        incidence->summary().utf8()));
  }

  // location
  if (!incidence->location().isEmpty()) {
    icalcomponent_add_property(parent,icalproperty_new_location(
        incidence->location().utf8()));
  }

  // status
  icalproperty_status status = ICAL_STATUS_NONE;
  switch (incidence->status()) {
    case Incidence::StatusTentative:    status = ICAL_STATUS_TENTATIVE;  break;
    case Incidence::StatusConfirmed:    status = ICAL_STATUS_CONFIRMED;  break;
    case Incidence::StatusCompleted:    status = ICAL_STATUS_COMPLETED;  break;
    case Incidence::StatusNeedsAction:  status = ICAL_STATUS_NEEDSACTION;  break;
    case Incidence::StatusCanceled:     status = ICAL_STATUS_CANCELLED;  break;
    case Incidence::StatusInProcess:    status = ICAL_STATUS_INPROCESS;  break;
    case Incidence::StatusDraft:        status = ICAL_STATUS_DRAFT;  break;
    case Incidence::StatusFinal:        status = ICAL_STATUS_FINAL;  break;
    case Incidence::StatusX: {
      icalproperty* p = icalproperty_new_status(ICAL_STATUS_X);
      icalvalue_set_x(icalproperty_get_value(p), incidence->statusStr().utf8());
      icalcomponent_add_property(parent, p);
      break;
    }
    case Incidence::StatusNone:
    default:
      break;
  }
  if (status != ICAL_STATUS_NONE)
    icalcomponent_add_property(parent, icalproperty_new_status(status));

  // secrecy
  icalproperty_class secClass;
  switch (incidence->secrecy()) {
    case Incidence::SecrecyPublic:
      secClass = ICAL_CLASS_PUBLIC;
      break;
    case Incidence::SecrecyConfidential:
      secClass = ICAL_CLASS_CONFIDENTIAL;
      break;
    case Incidence::SecrecyPrivate:
    default:
      secClass = ICAL_CLASS_PRIVATE;
      break;
  }
  if ( secClass != ICAL_CLASS_PUBLIC ) {
    icalcomponent_add_property(parent,icalproperty_new_class(secClass));
  }

  // priority
  if ( incidence->priority() > 0 ) { // 0 is undefined priority
    icalcomponent_add_property(parent,icalproperty_new_priority(
        incidence->priority()));
  }

  // categories
  QStringList categories = incidence->categories();
  QStringList::Iterator it;
  for(it = categories.begin(); it != categories.end(); ++it ) {
    icalcomponent_add_property(parent,icalproperty_new_categories((*it).utf8()));
  }

  // related event
  if ( !incidence->relatedToUid().isEmpty() ) {
    icalcomponent_add_property(parent,icalproperty_new_relatedto(
        incidence->relatedToUid().utf8()));
  }

//   kdDebug(5800) << "Write recurrence for '" << incidence->summary() << "' (" << incidence->uid()
//             << ")" << endl;

  RecurrenceRule::List rrules( incidence->recurrence()->rRules() );
  RecurrenceRule::List::ConstIterator rit;
  for ( rit = rrules.begin(); rit != rrules.end(); ++rit ) {
    icalcomponent_add_property( parent, icalproperty_new_rrule(
                                writeRecurrenceRule( (*rit) ) ) );
  }

  RecurrenceRule::List exrules( incidence->recurrence()->exRules() );
  RecurrenceRule::List::ConstIterator exit;
  for ( exit = exrules.begin(); exit != exrules.end(); ++exit ) {
    icalcomponent_add_property( parent, icalproperty_new_rrule(
                                writeRecurrenceRule( (*exit) ) ) );
  }

  DateList dateList = incidence->recurrence()->exDates();
  DateList::ConstIterator exIt;
  for(exIt = dateList.begin(); exIt != dateList.end(); ++exIt) {
    icalcomponent_add_property(parent,icalproperty_new_exdate(
        writeICalDate(*exIt)));
  }
  DateTimeList dateTimeList = incidence->recurrence()->exDateTimes();
  DateTimeList::ConstIterator extIt;
  for(extIt = dateTimeList.begin(); extIt != dateTimeList.end(); ++extIt) {
    icalcomponent_add_property(parent,icalproperty_new_exdate(
        writeICalDateTime(*extIt)));
  }


  dateList = incidence->recurrence()->rDates();
  DateList::ConstIterator rdIt;
  for( rdIt = dateList.begin(); rdIt != dateList.end(); ++rdIt) {
     icalcomponent_add_property( parent, icalproperty_new_rdate(
         writeICalDatePeriod(*rdIt) ) );
  }
  dateTimeList = incidence->recurrence()->rDateTimes();
  DateTimeList::ConstIterator rdtIt;
  for( rdtIt = dateTimeList.begin(); rdtIt != dateTimeList.end(); ++rdtIt) {
     icalcomponent_add_property( parent, icalproperty_new_rdate(
         writeICalDateTimePeriod(*rdtIt) ) );
  }

  // attachments
  Attachment::List attachments = incidence->attachments();
  Attachment::List::ConstIterator atIt;
  for ( atIt = attachments.begin(); atIt != attachments.end(); ++atIt ) {
    icalcomponent_add_property( parent, writeAttachment( *atIt ) );
  }

  // alarms
  Alarm::List::ConstIterator alarmIt;
  for ( alarmIt = incidence->alarms().begin();
        alarmIt != incidence->alarms().end(); ++alarmIt ) {
    if ( (*alarmIt)->enabled() ) {
//      kdDebug(5800) << "Write alarm for " << incidence->summary() << endl;
      icalcomponent_add_component( parent, writeAlarm( *alarmIt ) );
    }
  }

  // duration
  if (incidence->hasDuration()) {
    icaldurationtype duration;
    duration = writeICalDuration( incidence->duration() );
    icalcomponent_add_property(parent,icalproperty_new_duration(duration));
  }
}

void ICalFormatImpl::writeIncidenceBase( icalcomponent *parent,
                                         IncidenceBase * incidenceBase )
{
  icalcomponent_add_property( parent, icalproperty_new_dtstamp(
      writeICalDateTime( QDateTime::currentDateTime() ) ) );

  // organizer stuff
  if ( !incidenceBase->organizer().isEmpty() ) {
    icalcomponent_add_property( parent, writeOrganizer( incidenceBase->organizer() ) );
  }

  // attendees
  if ( incidenceBase->attendeeCount() > 0 ) {
    Attendee::List::ConstIterator it;
    for( it = incidenceBase->attendees().begin();
         it != incidenceBase->attendees().end(); ++it ) {
      icalcomponent_add_property( parent, writeAttendee( *it ) );
    }
  }

  // comments
  QStringList comments = incidenceBase->comments();
  for (QStringList::Iterator it=comments.begin(); it!=comments.end(); ++it) {
    icalcomponent_add_property(parent, icalproperty_new_comment((*it).utf8()));
  }

  // custom properties
  writeCustomProperties( parent, incidenceBase );
}

void ICalFormatImpl::writeCustomProperties(icalcomponent *parent,CustomProperties *properties)
{
  QMap<QCString, QString> custom = properties->customProperties();
  for (QMap<QCString, QString>::Iterator c = custom.begin();  c != custom.end();  ++c) {
    icalproperty *p = icalproperty_new_x(c.data().utf8());
    icalproperty_set_x_name(p,c.key());
    icalcomponent_add_property(parent,p);
  }
}

icalproperty *ICalFormatImpl::writeOrganizer( const Person &organizer )
{
  icalproperty *p = icalproperty_new_organizer("MAILTO:" + organizer.email().utf8());

  if (!organizer.name().isEmpty()) {
    icalproperty_add_parameter( p, icalparameter_new_cn(quoteForParam(organizer.name()).utf8()) );
  }
  // TODO: Write dir, sent-by and language

  return p;
}


icalproperty *ICalFormatImpl::writeAttendee(Attendee *attendee)
{
  icalproperty *p = icalproperty_new_attendee("mailto:" + attendee->email().utf8());

  if (!attendee->name().isEmpty()) {
    icalproperty_add_parameter(p,icalparameter_new_cn(quoteForParam(attendee->name()).utf8()));
  }


  icalproperty_add_parameter(p,icalparameter_new_rsvp(
          attendee->RSVP() ? ICAL_RSVP_TRUE : ICAL_RSVP_FALSE ));

  icalparameter_partstat status = ICAL_PARTSTAT_NEEDSACTION;
  switch (attendee->status()) {
    default:
    case Attendee::NeedsAction:
      status = ICAL_PARTSTAT_NEEDSACTION;
      break;
    case Attendee::Accepted:
      status = ICAL_PARTSTAT_ACCEPTED;
      break;
    case Attendee::Declined:
      status = ICAL_PARTSTAT_DECLINED;
      break;
    case Attendee::Tentative:
      status = ICAL_PARTSTAT_TENTATIVE;
      break;
    case Attendee::Delegated:
      status = ICAL_PARTSTAT_DELEGATED;
      break;
    case Attendee::Completed:
      status = ICAL_PARTSTAT_COMPLETED;
      break;
    case Attendee::InProcess:
      status = ICAL_PARTSTAT_INPROCESS;
      break;
  }
  icalproperty_add_parameter(p,icalparameter_new_partstat(status));

  icalparameter_role role = ICAL_ROLE_REQPARTICIPANT;
  switch (attendee->role()) {
    case Attendee::Chair:
      role = ICAL_ROLE_CHAIR;
      break;
    default:
    case Attendee::ReqParticipant:
      role = ICAL_ROLE_REQPARTICIPANT;
      break;
    case Attendee::OptParticipant:
      role = ICAL_ROLE_OPTPARTICIPANT;
      break;
    case Attendee::NonParticipant:
      role = ICAL_ROLE_NONPARTICIPANT;
      break;
  }
  icalproperty_add_parameter(p,icalparameter_new_role(role));

  if (!attendee->uid().isEmpty()) {
    icalparameter* icalparameter_uid = icalparameter_new_x(attendee->uid().utf8());
    icalparameter_set_xname(icalparameter_uid,"X-UID");
    icalproperty_add_parameter(p,icalparameter_uid);
  }

  if ( !attendee->delegate().isEmpty() ) {
    icalparameter* icalparameter_delegate = icalparameter_new_delegatedto( attendee->delegate().utf8() );
    icalproperty_add_parameter( p, icalparameter_delegate );
  }

  if ( !attendee->delegator().isEmpty() ) {
    icalparameter* icalparameter_delegator = icalparameter_new_delegatedfrom( attendee->delegator().utf8() );
    icalproperty_add_parameter( p, icalparameter_delegator );
  }

  return p;
}

icalproperty *ICalFormatImpl::writeAttachment(Attachment *att)
{
  icalattach *attach;
  if (att->isUri())
      attach = icalattach_new_from_url( att->uri().utf8().data());
  else
      attach = icalattach_new_from_data ( (unsigned char *)att->data(), 0, 0);
  icalproperty *p = icalproperty_new_attach(attach);

  if ( !att->mimeType().isEmpty() ) {
    icalproperty_add_parameter( p,
        icalparameter_new_fmttype( att->mimeType().utf8().data() ) );
  }

  if ( att->isBinary() ) {
    icalproperty_add_parameter( p,
        icalparameter_new_value( ICAL_VALUE_BINARY ) );
    icalproperty_add_parameter( p,
        icalparameter_new_encoding( ICAL_ENCODING_BASE64 ) );
  }

  if ( att->showInline() ) {
    icalparameter* icalparameter_inline = icalparameter_new_x( "inline" );
    icalparameter_set_xname( icalparameter_inline, "X-CONTENT-DISPOSITION" );
    icalproperty_add_parameter( p, icalparameter_inline );
  }

  if ( !att->label().isEmpty() ) {
    icalparameter* icalparameter_label = icalparameter_new_x( att->label().utf8() );
    icalparameter_set_xname( icalparameter_label, "X-LABEL" );
    icalproperty_add_parameter( p, icalparameter_label );
  }

  return p;
}

icalrecurrencetype ICalFormatImpl::writeRecurrenceRule( RecurrenceRule *recur )
{
//  kdDebug(5800) << "ICalFormatImpl::writeRecurrenceRule()" << endl;

  icalrecurrencetype r;
  icalrecurrencetype_clear(&r);

  switch( recur->recurrenceType() ) {
    case RecurrenceRule::rSecondly:
      r.freq = ICAL_SECONDLY_RECURRENCE;
      break;
    case RecurrenceRule::rMinutely:
      r.freq = ICAL_MINUTELY_RECURRENCE;
      break;
    case RecurrenceRule::rHourly:
      r.freq = ICAL_HOURLY_RECURRENCE;
      break;
    case RecurrenceRule::rDaily:
      r.freq = ICAL_DAILY_RECURRENCE;
      break;
    case RecurrenceRule::rWeekly:
      r.freq = ICAL_WEEKLY_RECURRENCE;
      break;
    case RecurrenceRule::rMonthly:
      r.freq = ICAL_MONTHLY_RECURRENCE;
      break;
    case RecurrenceRule::rYearly:
      r.freq = ICAL_YEARLY_RECURRENCE;
      break;
    default:
      r.freq = ICAL_NO_RECURRENCE;
      kdDebug(5800) << "ICalFormatImpl::writeRecurrence(): no recurrence" << endl;
      break;
  }

  int index = 0;
  QValueList<int> bys;
  QValueList<int>::ConstIterator it;

  // Now write out the BY* parts:
  bys = recur->bySeconds();
  index = 0;
  for ( it = bys.begin(); it != bys.end(); ++it ) {
    r.by_second[index++] = *it;
  }

  bys = recur->byMinutes();
  index = 0;
  for ( it = bys.begin(); it != bys.end(); ++it ) {
    r.by_minute[index++] = *it;
  }

  bys = recur->byHours();
  index = 0;
  for ( it = bys.begin(); it != bys.end(); ++it ) {
    r.by_hour[index++] = *it;
  }

  bys = recur->byMonthDays();
  index = 0;
  for ( it = bys.begin(); it != bys.end(); ++it ) {
    r.by_month_day[index++] = icalrecurrencetype_day_position( (*it) * 8 );
  }

  bys = recur->byYearDays();
  index = 0;
  for ( it = bys.begin(); it != bys.end(); ++it ) {
    r.by_year_day[index++] = *it;
  }

  bys = recur->byWeekNumbers();
  index = 0;
  for ( it = bys.begin(); it != bys.end(); ++it ) {
     r.by_week_no[index++] = *it;
  }

  bys = recur->byMonths();
  index = 0;
  for ( it = bys.begin(); it != bys.end(); ++it ) {
    r.by_month[index++] = *it;
  }

  bys = recur->bySetPos();
  index = 0;
  for ( it = bys.begin(); it != bys.end(); ++it ) {
     r.by_set_pos[index++] = *it;
  }


  QValueList<RecurrenceRule::WDayPos> byd = recur->byDays();
  int day;
  index = 0;
  for ( QValueList<RecurrenceRule::WDayPos>::ConstIterator dit = byd.begin();
        dit != byd.end(); ++dit ) {
    day = (*dit).day() % 7 + 1;     // convert from Monday=1 to Sunday=1
    if ( (*dit).pos() < 0 ) {
      day += (-(*dit).pos())*8;
      day = -day;
    } else {
      day += (*dit).pos()*8;
    }
    r.by_day[index++] = day;
  }

  r.week_start = static_cast<icalrecurrencetype_weekday>(
                                             recur->weekStart()%7 + 1);

  if ( recur->frequency() > 1 ) {
    // Dont' write out INTERVAL=1, because that's the default anyway
    r.interval = recur->frequency();
  }

  if ( recur->duration() > 0 ) {
    r.count = recur->duration();
  } else if ( recur->duration() == -1 ) {
    r.count = 0;
  } else {
    if ( recur->doesFloat() )
      r.until = writeICalDate(recur->endDt().date());
    else
      r.until = writeICalDateTime(recur->endDt());
  }

// Debug output
#if 0
  const char *str = icalrecurrencetype_as_string(&r);
  if (str) {
    kdDebug(5800) << " String: " << str << endl;
  } else {
    kdDebug(5800) << " No String" << endl;
  }
#endif

  return r;
}


icalcomponent *ICalFormatImpl::writeAlarm(Alarm *alarm)
{
// kdDebug(5800) << " ICalFormatImpl::writeAlarm" << endl;
  icalcomponent *a = icalcomponent_new(ICAL_VALARM_COMPONENT);

  icalproperty_action action;
  icalattach *attach = 0;

  switch (alarm->type()) {
    case Alarm::Procedure:
      action = ICAL_ACTION_PROCEDURE;
      attach = icalattach_new_from_url(QFile::encodeName(alarm->programFile()).data());
      icalcomponent_add_property(a,icalproperty_new_attach(attach));
      if (!alarm->programArguments().isEmpty()) {
        icalcomponent_add_property(a,icalproperty_new_description(alarm->programArguments().utf8()));
      }
      break;
    case Alarm::Audio:
      action = ICAL_ACTION_AUDIO;
// kdDebug(5800) << " It's an audio action, file: " << alarm->audioFile() << endl;
      if (!alarm->audioFile().isEmpty()) {
        attach = icalattach_new_from_url(QFile::encodeName( alarm->audioFile() ).data());
        icalcomponent_add_property(a,icalproperty_new_attach(attach));
      }
      break;
    case Alarm::Email: {
      action = ICAL_ACTION_EMAIL;
      QValueList<Person> addresses = alarm->mailAddresses();
      for (QValueList<Person>::Iterator ad = addresses.begin();  ad != addresses.end();  ++ad) {
        icalproperty *p = icalproperty_new_attendee("MAILTO:" + (*ad).email().utf8());
        if (!(*ad).name().isEmpty()) {
          icalproperty_add_parameter(p,icalparameter_new_cn(quoteForParam((*ad).name()).utf8()));
        }
        icalcomponent_add_property(a,p);
      }
      icalcomponent_add_property(a,icalproperty_new_summary(alarm->mailSubject().utf8()));
      icalcomponent_add_property(a,icalproperty_new_description(alarm->mailText().utf8()));
      QStringList attachments = alarm->mailAttachments();
      if (attachments.count() > 0) {
        for (QStringList::Iterator at = attachments.begin();  at != attachments.end();  ++at) {
          attach = icalattach_new_from_url(QFile::encodeName( *at ).data());
          icalcomponent_add_property(a,icalproperty_new_attach(attach));
        }
      }
      break;
    }
    case Alarm::Display:
      action = ICAL_ACTION_DISPLAY;
      icalcomponent_add_property(a,icalproperty_new_description(alarm->text().utf8()));
      break;
    case Alarm::Invalid:
    default:
      kdDebug(5800) << "Unknown type of alarm" << endl;
      action = ICAL_ACTION_NONE;
      break;
  }
  icalcomponent_add_property(a,icalproperty_new_action(action));

  // Trigger time
  icaltriggertype trigger;
  if ( alarm->hasTime() ) {
    trigger.time = writeICalDateTime(alarm->time());
    trigger.duration = icaldurationtype_null_duration();
  } else {
    trigger.time = icaltime_null_time();
    Duration offset;
    if ( alarm->hasStartOffset() )
      offset = alarm->startOffset();
    else
      offset = alarm->endOffset();
    trigger.duration = icaldurationtype_from_int( offset.asSeconds() );
  }
  icalproperty *p = icalproperty_new_trigger(trigger);
  if ( alarm->hasEndOffset() )
    icalproperty_add_parameter(p,icalparameter_new_related(ICAL_RELATED_END));
  icalcomponent_add_property(a,p);

  // Repeat count and duration
  if (alarm->repeatCount()) {
    icalcomponent_add_property(a,icalproperty_new_repeat(alarm->repeatCount()));
    icalcomponent_add_property(a,icalproperty_new_duration(
                             icaldurationtype_from_int(alarm->snoozeTime()*60)));
  }

  // Custom properties
  QMap<QCString, QString> custom = alarm->customProperties();
  for (QMap<QCString, QString>::Iterator c = custom.begin();  c != custom.end();  ++c) {
    icalproperty *p = icalproperty_new_x(c.data().utf8());
    icalproperty_set_x_name(p,c.key());
    icalcomponent_add_property(a,p);
  }

  return a;
}

Todo *ICalFormatImpl::readTodo(icalcomponent *vtodo)
{
  Todo *todo = new Todo;

  readIncidence(vtodo, 0, todo); // FIXME timezone

  icalproperty *p = icalcomponent_get_first_property(vtodo,ICAL_ANY_PROPERTY);

//  int intvalue;
  icaltimetype icaltime;

  QStringList categories;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_DUE_PROPERTY:  // due date
        icaltime = icalproperty_get_due(p);
        if (icaltime.is_date) {
          todo->setDtDue(QDateTime(readICalDate(icaltime),QTime(0,0,0)),true);
        } else {
          todo->setDtDue(readICalDateTime(icaltime),true);
          todo->setFloats(false);
        }
        todo->setHasDueDate(true);
        break;

      case ICAL_COMPLETED_PROPERTY:  // completion date
        icaltime = icalproperty_get_completed(p);
        todo->setCompleted(readICalDateTime(icaltime));
        break;

      case ICAL_PERCENTCOMPLETE_PROPERTY:  // Percent completed
        todo->setPercentComplete(icalproperty_get_percentcomplete(p));
        break;

      case ICAL_RELATEDTO_PROPERTY:  // related todo (parent)
        todo->setRelatedToUid(QString::fromUtf8(icalproperty_get_relatedto(p)));
        mTodosRelate.append(todo);
        break;

      case ICAL_DTSTART_PROPERTY: {
        // Flag that todo has start date. Value is read in by readIncidence().
        if ( todo->comments().grep("NoStartDate").count() )
          todo->setHasStartDate( false );
        else
          todo->setHasStartDate( true );
        break;
      }

      case ICAL_RECURRENCEID_PROPERTY:
        icaltime = icalproperty_get_recurrenceid(p);
        todo->setDtRecurrence( readICalDateTime(icaltime) );
        break;

      default:
//        kdDebug(5800) << "ICALFormat::readTodo(): Unknown property: " << kind
//                  << endl;
        break;
    }

    p = icalcomponent_get_next_property(vtodo,ICAL_ANY_PROPERTY);
  }

  if (mCompat) mCompat->fixEmptySummary( todo );

  return todo;
}

Event *ICalFormatImpl::readEvent( icalcomponent *vevent, icalcomponent *vtimezone )
{
  Event *event = new Event;

  // FIXME where is this freed?
  icaltimezone *tz = icaltimezone_new();
  if ( !icaltimezone_set_component( tz, vtimezone ) ) {
    icaltimezone_free( tz, 1 );
    tz = 0;
  }

  readIncidence( vevent, tz, event);

  icalproperty *p = icalcomponent_get_first_property(vevent,ICAL_ANY_PROPERTY);

//  int intvalue;
  icaltimetype icaltime;

  QStringList categories;
  icalproperty_transp transparency;

  bool dtEndProcessed = false;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_DTEND_PROPERTY:  // start date and time
        icaltime = icalproperty_get_dtend(p);
        if (icaltime.is_date) {
          // End date is non-inclusive
          QDate endDate = readICalDate( icaltime ).addDays( -1 );
          if ( mCompat ) mCompat->fixFloatingEnd( endDate );
          if ( endDate < event->dtStart().date() ) {
            endDate = event->dtStart().date();
          }
          event->setDtEnd( QDateTime( endDate, QTime( 0, 0, 0 ) ) );
        } else {
          event->setDtEnd(readICalDateTime(icaltime, tz));
          event->setFloats( false );
        }
        dtEndProcessed = true;
        break;

      case ICAL_RELATEDTO_PROPERTY:  // related event (parent)
        event->setRelatedToUid(QString::fromUtf8(icalproperty_get_relatedto(p)));
        mEventsRelate.append(event);
        break;


      case ICAL_TRANSP_PROPERTY:  // Transparency
        transparency = icalproperty_get_transp(p);
        if( transparency == ICAL_TRANSP_TRANSPARENT )
          event->setTransparency( Event::Transparent );
        else
          event->setTransparency( Event::Opaque );
        break;

      default:
//        kdDebug(5800) << "ICALFormat::readEvent(): Unknown property: " << kind
//                  << endl;
        break;
    }

    p = icalcomponent_get_next_property(vevent,ICAL_ANY_PROPERTY);
  }

  // according to rfc2445 the dtend shouldn't be written when it equals
  // start date. so assign one equal to start date.
  if ( !dtEndProcessed && !event->hasDuration() ) {
    event->setDtEnd( event->dtStart() );
  }

  QString msade = event->nonKDECustomProperty("X-MICROSOFT-CDO-ALLDAYEVENT");
  if (!msade.isEmpty()) {
    bool floats = (msade == QString::fromLatin1("TRUE"));
    event->setFloats(floats);
  }

  if ( mCompat ) mCompat->fixEmptySummary( event );

  return event;
}

FreeBusy *ICalFormatImpl::readFreeBusy(icalcomponent *vfreebusy)
{
  FreeBusy *freebusy = new FreeBusy;

  readIncidenceBase(vfreebusy, freebusy);

  icalproperty *p = icalcomponent_get_first_property(vfreebusy,ICAL_ANY_PROPERTY);

  icaltimetype icaltime;
  PeriodList periods;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_DTSTART_PROPERTY:  // start date and time
        icaltime = icalproperty_get_dtstart(p);
        freebusy->setDtStart(readICalDateTime(icaltime));
        break;

      case ICAL_DTEND_PROPERTY:  // end Date and Time
        icaltime = icalproperty_get_dtend(p);
        freebusy->setDtEnd(readICalDateTime(icaltime));
        break;

      case ICAL_FREEBUSY_PROPERTY: { //Any FreeBusy Times
        icalperiodtype icalperiod = icalproperty_get_freebusy(p);
        QDateTime period_start = readICalDateTime(icalperiod.start);
        Period period;
        if ( !icaltime_is_null_time(icalperiod.end) ) {
          QDateTime period_end = readICalDateTime(icalperiod.end);
          period = Period(period_start, period_end);
        } else {
          Duration duration = readICalDuration( icalperiod.duration );
          period = Period(period_start, duration);
        }
        QCString param = icalproperty_get_parameter_as_string( p, "X-SUMMARY" );
        period.setSummary( QString::fromUtf8( KCodecs::base64Decode( param ) ) );
        param = icalproperty_get_parameter_as_string( p, "X-LOCATION" );
        period.setLocation( QString::fromUtf8( KCodecs::base64Decode( param ) ) );
        periods.append( period );
        break;}

      default:
//        kdDebug(5800) << "ICalFormatImpl::readFreeBusy(): Unknown property: "
//                      << kind << endl;
      break;
    }
    p = icalcomponent_get_next_property(vfreebusy,ICAL_ANY_PROPERTY);
  }
  freebusy->addPeriods( periods );

  return freebusy;
}

Journal *ICalFormatImpl::readJournal(icalcomponent *vjournal)
{
  Journal *journal = new Journal;

  readIncidence(vjournal, 0, journal); // FIXME tz?

  return journal;
}

Attendee *ICalFormatImpl::readAttendee(icalproperty *attendee)
{
  icalparameter *p = 0;

  QString email = QString::fromUtf8(icalproperty_get_attendee(attendee));
  if ( email.startsWith( "mailto:", false ) ) {
    email = email.mid( 7 );
  }

  QString name;
  QString uid = QString::null;
  p = icalproperty_get_first_parameter(attendee,ICAL_CN_PARAMETER);
  if (p) {
    name = QString::fromUtf8(icalparameter_get_cn(p));
  } else {
  }

  bool rsvp=false;
  p = icalproperty_get_first_parameter(attendee,ICAL_RSVP_PARAMETER);
  if (p) {
    icalparameter_rsvp rsvpParameter = icalparameter_get_rsvp(p);
    if (rsvpParameter == ICAL_RSVP_TRUE) rsvp = true;
  }

  Attendee::PartStat status = Attendee::NeedsAction;
  p = icalproperty_get_first_parameter(attendee,ICAL_PARTSTAT_PARAMETER);
  if (p) {
    icalparameter_partstat partStatParameter = icalparameter_get_partstat(p);
    switch(partStatParameter) {
      default:
      case ICAL_PARTSTAT_NEEDSACTION:
        status = Attendee::NeedsAction;
        break;
      case ICAL_PARTSTAT_ACCEPTED:
        status = Attendee::Accepted;
        break;
      case ICAL_PARTSTAT_DECLINED:
        status = Attendee::Declined;
        break;
      case ICAL_PARTSTAT_TENTATIVE:
        status = Attendee::Tentative;
        break;
      case ICAL_PARTSTAT_DELEGATED:
        status = Attendee::Delegated;
        break;
      case ICAL_PARTSTAT_COMPLETED:
        status = Attendee::Completed;
        break;
      case ICAL_PARTSTAT_INPROCESS:
        status = Attendee::InProcess;
        break;
    }
  }

  Attendee::Role role = Attendee::ReqParticipant;
  p = icalproperty_get_first_parameter(attendee,ICAL_ROLE_PARAMETER);
  if (p) {
    icalparameter_role roleParameter = icalparameter_get_role(p);
    switch(roleParameter) {
      case ICAL_ROLE_CHAIR:
        role = Attendee::Chair;
        break;
      default:
      case ICAL_ROLE_REQPARTICIPANT:
        role = Attendee::ReqParticipant;
        break;
      case ICAL_ROLE_OPTPARTICIPANT:
        role = Attendee::OptParticipant;
        break;
      case ICAL_ROLE_NONPARTICIPANT:
        role = Attendee::NonParticipant;
        break;
    }
  }

  p = icalproperty_get_first_parameter(attendee,ICAL_X_PARAMETER);
  uid = icalparameter_get_xvalue(p);
  // This should be added, but there seems to be a libical bug here.
  // TODO: does this work now in libical-0.24 or greater?
  /*while (p) {
   // if (icalparameter_get_xname(p) == "X-UID") {
    uid = icalparameter_get_xvalue(p);
    p = icalproperty_get_next_parameter(attendee,ICAL_X_PARAMETER);
  } */

  Attendee *a = new Attendee( name, email, rsvp, status, role, uid );

  p = icalproperty_get_first_parameter( attendee, ICAL_DELEGATEDTO_PARAMETER );
  if ( p )
    a->setDelegate( icalparameter_get_delegatedto( p ) );

  p = icalproperty_get_first_parameter( attendee, ICAL_DELEGATEDFROM_PARAMETER );
  if ( p )
    a->setDelegator( icalparameter_get_delegatedfrom( p ) );

  return a;
}

Person ICalFormatImpl::readOrganizer( icalproperty *organizer )
{
  QString email = QString::fromUtf8(icalproperty_get_organizer(organizer));
  if ( email.startsWith( "mailto:", false ) ) {
    email = email.mid( 7 );
  }
  QString cn;

  icalparameter *p = icalproperty_get_first_parameter(
             organizer, ICAL_CN_PARAMETER );

  if ( p ) {
    cn = QString::fromUtf8( icalparameter_get_cn( p ) );
  }
  Person org( cn, email );
  // TODO: Treat sent-by, dir and language here, too
  return org;
}

Attachment *ICalFormatImpl::readAttachment(icalproperty *attach)
{
  Attachment *attachment = 0;

  icalvalue_kind value_kind = icalvalue_isa(icalproperty_get_value(attach));

  if ( value_kind == ICAL_ATTACH_VALUE || value_kind == ICAL_BINARY_VALUE ) {
    icalattach *a = icalproperty_get_attach(attach);

    int isurl = icalattach_get_is_url (a);
    if (isurl == 0)
      attachment = new Attachment((const char*)icalattach_get_data(a));
    else {
      attachment = new Attachment(QString::fromUtf8(icalattach_get_url(a)));
    }
  }
  else if ( value_kind == ICAL_URI_VALUE ) {
    attachment = new Attachment(QString::fromUtf8(icalvalue_get_uri(icalproperty_get_value(attach))));
  }

  icalparameter *p = icalproperty_get_first_parameter(attach, ICAL_FMTTYPE_PARAMETER);
  if (p && attachment)
    attachment->setMimeType(QString(icalparameter_get_fmttype(p)));

  p = icalproperty_get_first_parameter(attach,ICAL_X_PARAMETER);
  while (p) {
   if ( strncmp (icalparameter_get_xname(p), "X-LABEL", 7) == 0 )
     attachment->setLabel( icalparameter_get_xvalue(p) );
    p = icalproperty_get_next_parameter(attach, ICAL_X_PARAMETER);
  }

  return attachment;
}

void ICalFormatImpl::readIncidence(icalcomponent *parent, icaltimezone *tz, Incidence *incidence)
{
  readIncidenceBase(parent,incidence);

  icalproperty *p = icalcomponent_get_first_property(parent,ICAL_ANY_PROPERTY);

  const char *text;
  int intvalue, inttext;
  icaltimetype icaltime;
  icaldurationtype icalduration;

  QStringList categories;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_CREATED_PROPERTY:
        icaltime = icalproperty_get_created(p);
        incidence->setCreated(readICalDateTime(icaltime, tz));
        break;

      case ICAL_SEQUENCE_PROPERTY:  // sequence
        intvalue = icalproperty_get_sequence(p);
        incidence->setRevision(intvalue);
        break;

      case ICAL_LASTMODIFIED_PROPERTY:  // last modification date
        icaltime = icalproperty_get_lastmodified(p);
        incidence->setLastModified(readICalDateTime(icaltime, tz));
        break;

      case ICAL_DTSTART_PROPERTY:  // start date and time
        icaltime = icalproperty_get_dtstart(p);
        if (icaltime.is_date) {
          incidence->setDtStart(QDateTime(readICalDate(icaltime),QTime(0,0,0)));
          incidence->setFloats( true );
        } else {
          incidence->setDtStart(readICalDateTime(icaltime, tz));
          incidence->setFloats( false );
        }
        break;

      case ICAL_DURATION_PROPERTY:  // start date and time
        icalduration = icalproperty_get_duration(p);
        incidence->setDuration(readICalDuration(icalduration));
        break;

      case ICAL_DESCRIPTION_PROPERTY:  // description
        text = icalproperty_get_description(p);
        incidence->setDescription(QString::fromUtf8(text));
        break;

      case ICAL_SUMMARY_PROPERTY:  // summary
        text = icalproperty_get_summary(p);
        incidence->setSummary(QString::fromUtf8(text));
        break;

      case ICAL_LOCATION_PROPERTY:  // location
        text = icalproperty_get_location(p);
        incidence->setLocation(QString::fromUtf8(text));
        break;

      case ICAL_STATUS_PROPERTY: {  // status
        Incidence::Status stat;
        switch (icalproperty_get_status(p)) {
          case ICAL_STATUS_TENTATIVE:   stat = Incidence::StatusTentative; break;
          case ICAL_STATUS_CONFIRMED:   stat = Incidence::StatusConfirmed; break;
          case ICAL_STATUS_COMPLETED:   stat = Incidence::StatusCompleted; break;
          case ICAL_STATUS_NEEDSACTION: stat = Incidence::StatusNeedsAction; break;
          case ICAL_STATUS_CANCELLED:   stat = Incidence::StatusCanceled; break;
          case ICAL_STATUS_INPROCESS:   stat = Incidence::StatusInProcess; break;
          case ICAL_STATUS_DRAFT:       stat = Incidence::StatusDraft; break;
          case ICAL_STATUS_FINAL:       stat = Incidence::StatusFinal; break;
          case ICAL_STATUS_X:
            incidence->setCustomStatus(QString::fromUtf8(icalvalue_get_x(icalproperty_get_value(p))));
            stat = Incidence::StatusX;
            break;
          case ICAL_STATUS_NONE:
          default:                      stat = Incidence::StatusNone; break;
        }
        if (stat != Incidence::StatusX)
          incidence->setStatus(stat);
        break;
      }

      case ICAL_PRIORITY_PROPERTY:  // priority
        intvalue = icalproperty_get_priority( p );
        if ( mCompat )
          intvalue = mCompat->fixPriority( intvalue );
        incidence->setPriority( intvalue );
        break;

      case ICAL_CATEGORIES_PROPERTY:  // categories
        text = icalproperty_get_categories(p);
        categories.append(QString::fromUtf8(text));
        break;

      case ICAL_RRULE_PROPERTY:
        readRecurrenceRule( p, incidence );
        break;

      case ICAL_RDATE_PROPERTY: {
        icaldatetimeperiodtype rd = icalproperty_get_rdate( p );
        if ( icaltime_is_valid_time( rd.time ) ) {
          if ( icaltime_is_date( rd.time ) ) {
            incidence->recurrence()->addRDate( readICalDate( rd.time ) );
          } else {
            incidence->recurrence()->addRDateTime( readICalDateTime( rd.time, tz ) );
          }
        } else {
          // TODO: RDates as period are not yet implemented!
        }
        break; }

      case ICAL_EXRULE_PROPERTY:
        readExceptionRule( p, incidence );
        break;

      case ICAL_EXDATE_PROPERTY:
        icaltime = icalproperty_get_exdate(p);
        if ( icaltime_is_date(icaltime) ) {
          incidence->recurrence()->addExDate( readICalDate(icaltime) );
        } else {
          incidence->recurrence()->addExDateTime( readICalDateTime(icaltime, tz) );
        }
        break;

      case ICAL_CLASS_PROPERTY:
        inttext = icalproperty_get_class(p);
        if (inttext == ICAL_CLASS_PUBLIC ) {
          incidence->setSecrecy(Incidence::SecrecyPublic);
        } else if (inttext == ICAL_CLASS_CONFIDENTIAL ) {
          incidence->setSecrecy(Incidence::SecrecyConfidential);
        } else {
          incidence->setSecrecy(Incidence::SecrecyPrivate);
        }
        break;

      case ICAL_ATTACH_PROPERTY:  // attachments
        incidence->addAttachment(readAttachment(p));
        break;

      default:
//        kdDebug(5800) << "ICALFormat::readIncidence(): Unknown property: " << kind
//                  << endl;
        break;
    }

    p = icalcomponent_get_next_property(parent,ICAL_ANY_PROPERTY);
  }

  // Set the scheduling ID
  const QString uid = incidence->customProperty( "LIBKCAL", "ID" );
  if ( !uid.isNull() ) {
    // The UID stored in incidencebase is actually the scheduling ID
    // It has to be stored in the iCal UID component for compatibility
    // with other iCal applications
    incidence->setSchedulingID( incidence->uid() );
    incidence->setUid( uid );
  }

  // Now that recurrence and exception stuff is completely set up,
  // do any backwards compatibility adjustments.
  if ( incidence->doesRecur() && mCompat )
      mCompat->fixRecurrence( incidence );

  // add categories
  incidence->setCategories(categories);

  // iterate through all alarms
  for (icalcomponent *alarm = icalcomponent_get_first_component(parent,ICAL_VALARM_COMPONENT);
       alarm;
       alarm = icalcomponent_get_next_component(parent,ICAL_VALARM_COMPONENT)) {
    readAlarm(alarm,incidence);
  }
  // Fix incorrect alarm settings by other applications (like outloook 9)
  if ( mCompat ) mCompat->fixAlarms( incidence );

}

void ICalFormatImpl::readIncidenceBase(icalcomponent *parent,IncidenceBase *incidenceBase)
{
  icalproperty *p = icalcomponent_get_first_property(parent,ICAL_ANY_PROPERTY);

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_UID_PROPERTY:  // unique id
        incidenceBase->setUid(QString::fromUtf8(icalproperty_get_uid(p)));
        break;

      case ICAL_ORGANIZER_PROPERTY:  // organizer
        incidenceBase->setOrganizer( readOrganizer(p));
        break;

      case ICAL_ATTENDEE_PROPERTY:  // attendee
        incidenceBase->addAttendee(readAttendee(p));
        break;

      case ICAL_COMMENT_PROPERTY:
        incidenceBase->addComment(
            QString::fromUtf8(icalproperty_get_comment(p)));
        break;

      default:
        break;
    }

    p = icalcomponent_get_next_property(parent,ICAL_ANY_PROPERTY);
  }

  // kpilot stuff
  // TODO: move this application-specific code to kpilot
  // need to get X-PILOT* attributes out, set correct properties, and get
  // rid of them...
  // Pointer fun, as per libical documentation
  // (documented in UsingLibical.txt)
  icalproperty *next =0;

  for ( p = icalcomponent_get_first_property(parent,ICAL_X_PROPERTY);
       p != 0;
       p = next )
  {

    next = icalcomponent_get_next_property(parent,ICAL_X_PROPERTY);

    QString value = QString::fromUtf8(icalproperty_get_x(p));
    QString name = icalproperty_get_x_name(p);

    if (name == "X-PILOTID" && !value.isEmpty()) {
      incidenceBase->setPilotId(value.toInt());
      icalcomponent_remove_property(parent,p);
    } else if (name == "X-PILOTSTAT" && !value.isEmpty()) {
      incidenceBase->setSyncStatus(value.toInt());
      icalcomponent_remove_property(parent,p);
    }
  }

  // custom properties
  readCustomProperties(parent, incidenceBase);
}

void ICalFormatImpl::readCustomProperties(icalcomponent *parent,CustomProperties *properties)
{
  QMap<QCString, QString> customProperties;
  QString lastProperty;

  icalproperty *p = icalcomponent_get_first_property(parent,ICAL_X_PROPERTY);

  while (p) {

    QString value = QString::fromUtf8(icalproperty_get_x(p));
    const char *name = icalproperty_get_x_name(p);
    if ( lastProperty != name ) {
      customProperties[name] = value;
    } else {
      customProperties[name] = customProperties[name].append( "," ).append( value );
    }
    // kdDebug(5800) << "Set custom property [" << name << '=' << value << ']' << endl;
    p = icalcomponent_get_next_property(parent,ICAL_X_PROPERTY);
    lastProperty = name;
  }

  properties->setCustomProperties(customProperties);
}



void ICalFormatImpl::readRecurrenceRule(icalproperty *rrule,Incidence *incidence )
{
//  kdDebug(5800) << "Read recurrence for " << incidence->summary() << endl;

  Recurrence *recur = incidence->recurrence();

  struct icalrecurrencetype r = icalproperty_get_rrule(rrule);
//   dumpIcalRecurrence(r);

  RecurrenceRule *recurrule = new RecurrenceRule( /*incidence*/ );
  recurrule->setStartDt( incidence->dtStart() );
  readRecurrence( r, recurrule );
  recur->addRRule( recurrule );
}

void ICalFormatImpl::readExceptionRule( icalproperty *rrule, Incidence *incidence )
{
//  kdDebug(5800) << "Read recurrence for " << incidence->summary() << endl;

  struct icalrecurrencetype r = icalproperty_get_exrule(rrule);
//   dumpIcalRecurrence(r);

  RecurrenceRule *recurrule = new RecurrenceRule( /*incidence*/ );
  recurrule->setStartDt( incidence->dtStart() );
  readRecurrence( r, recurrule );

  Recurrence *recur = incidence->recurrence();
  recur->addExRule( recurrule );
}

void ICalFormatImpl::readRecurrence( const struct icalrecurrencetype &r, RecurrenceRule* recur )
{
  // Generate the RRULE string
  recur->mRRule = QString( icalrecurrencetype_as_string( const_cast<struct icalrecurrencetype*>(&r) ) );
  // Period
  switch ( r.freq ) {
    case ICAL_SECONDLY_RECURRENCE: recur->setRecurrenceType( RecurrenceRule::rSecondly ); break;
    case ICAL_MINUTELY_RECURRENCE: recur->setRecurrenceType( RecurrenceRule::rMinutely ); break;
    case ICAL_HOURLY_RECURRENCE: recur->setRecurrenceType( RecurrenceRule::rHourly ); break;
    case ICAL_DAILY_RECURRENCE: recur->setRecurrenceType( RecurrenceRule::rDaily ); break;
    case ICAL_WEEKLY_RECURRENCE: recur->setRecurrenceType( RecurrenceRule::rWeekly ); break;
    case ICAL_MONTHLY_RECURRENCE: recur->setRecurrenceType( RecurrenceRule::rMonthly ); break;
    case ICAL_YEARLY_RECURRENCE: recur->setRecurrenceType( RecurrenceRule::rYearly ); break;
    case ICAL_NO_RECURRENCE:
    default:
        recur->setRecurrenceType( RecurrenceRule::rNone );
  }
  // Frequency
  recur->setFrequency( r.interval );

  // Duration & End Date
  if ( !icaltime_is_null_time( r.until ) ) {
    icaltimetype t;
    t = r.until;
    // Convert to the correct time zone! it's in UTC by specification.
    QDateTime endDate( readICalDateTime(t) );
    recur->setEndDt( endDate );
  } else {
    if (r.count == 0)
      recur->setDuration( -1 );
    else
      recur->setDuration( r.count );
  }

  // Week start setting
  int wkst = (r.week_start + 5)%7 + 1;
  recur->setWeekStart( wkst );

  // And now all BY*
  QValueList<int> lst;
  int i;
  int index = 0;

#define readSetByList(rrulecomp,setfunc) \
  index = 0; \
  lst.clear(); \
  while ( (i = r.rrulecomp[index++] ) != ICAL_RECURRENCE_ARRAY_MAX ) \
    lst.append( i ); \
  if ( !lst.isEmpty() ) recur->setfunc( lst );

  // BYSECOND, MINUTE and HOUR, MONTHDAY, YEARDAY, WEEKNUMBER, MONTH
  // and SETPOS are standard int lists, so we can treat them with the
  // same macro
  readSetByList( by_second, setBySeconds );
  readSetByList( by_minute, setByMinutes );
  readSetByList( by_hour, setByHours );
  readSetByList( by_month_day, setByMonthDays );
  readSetByList( by_year_day, setByYearDays );
  readSetByList( by_week_no, setByWeekNumbers );
  readSetByList( by_month, setByMonths );
  readSetByList( by_set_pos, setBySetPos );
#undef readSetByList

  // BYDAY is a special case, since it's not an int list
  QValueList<RecurrenceRule::WDayPos> wdlst;
  short day;
  index=0;
  while((day = r.by_day[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
    RecurrenceRule::WDayPos pos;
    pos.setDay( ( icalrecurrencetype_day_day_of_week( day ) + 5 )%7 + 1 );
    pos.setPos( icalrecurrencetype_day_position( day ) );
//     kdDebug(5800)<< "    o) By day, index="<<index-1<<", pos="<<pos.Pos<<", day="<<pos.Day<<endl;
    wdlst.append( pos );
  }
  if ( !wdlst.isEmpty() ) recur->setByDays( wdlst );


  // TODO Store all X- fields of the RRULE inside the recurrence (so they are
  // preserved
}


void ICalFormatImpl::readAlarm(icalcomponent *alarm,Incidence *incidence)
{
//   kdDebug(5800) << "Read alarm for " << incidence->summary() << endl;

  Alarm* ialarm = incidence->newAlarm();
  ialarm->setRepeatCount(0);
  ialarm->setEnabled(true);

  // Determine the alarm's action type
  icalproperty *p = icalcomponent_get_first_property(alarm,ICAL_ACTION_PROPERTY);
  Alarm::Type type = Alarm::Display;
  icalproperty_action action = ICAL_ACTION_DISPLAY;
  if ( !p ) {
    kdDebug(5800) << "Unknown type of alarm, using default" << endl;
//    return;
  } else {

    action = icalproperty_get_action(p);
    switch ( action ) {
      case ICAL_ACTION_DISPLAY:   type = Alarm::Display;  break;
      case ICAL_ACTION_AUDIO:     type = Alarm::Audio;  break;
      case ICAL_ACTION_PROCEDURE: type = Alarm::Procedure;  break;
      case ICAL_ACTION_EMAIL:     type = Alarm::Email;  break;
      default:
        kdDebug(5800) << "Unknown type of alarm: " << action << endl;
//        type = Alarm::Invalid;
    }
  }
  ialarm->setType(type);
// kdDebug(5800) << " alarm type =" << type << endl;

  p = icalcomponent_get_first_property(alarm,ICAL_ANY_PROPERTY);
  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);

    switch (kind) {

      case ICAL_TRIGGER_PROPERTY: {
        icaltriggertype trigger = icalproperty_get_trigger(p);
        if (icaltime_is_null_time(trigger.time)) {
          if (icaldurationtype_is_null_duration(trigger.duration)) {
            kdDebug(5800) << "ICalFormatImpl::readAlarm(): Trigger has no time and no duration." << endl;
          } else {
            Duration duration = icaldurationtype_as_int( trigger.duration );
            icalparameter *param = icalproperty_get_first_parameter(p,ICAL_RELATED_PARAMETER);
            if (param && icalparameter_get_related(param) == ICAL_RELATED_END)
              ialarm->setEndOffset(duration);
            else
              ialarm->setStartOffset(duration);
          }
        } else {
          ialarm->setTime(readICalDateTime(trigger.time));
        }
        break;
      }
      case ICAL_DURATION_PROPERTY: {
        icaldurationtype duration = icalproperty_get_duration(p);
        ialarm->setSnoozeTime(icaldurationtype_as_int(duration)/60);
        break;
      }
      case ICAL_REPEAT_PROPERTY:
        ialarm->setRepeatCount(icalproperty_get_repeat(p));
        break;

      // Only in DISPLAY and EMAIL and PROCEDURE alarms
      case ICAL_DESCRIPTION_PROPERTY: {
        QString description = QString::fromUtf8(icalproperty_get_description(p));
        switch ( action ) {
          case ICAL_ACTION_DISPLAY:
            ialarm->setText( description );
            break;
          case ICAL_ACTION_PROCEDURE:
            ialarm->setProgramArguments( description );
            break;
          case ICAL_ACTION_EMAIL:
            ialarm->setMailText( description );
            break;
          default:
            break;
        }
        break;
      }
      // Only in EMAIL alarm
      case ICAL_SUMMARY_PROPERTY:
        ialarm->setMailSubject(QString::fromUtf8(icalproperty_get_summary(p)));
        break;

      // Only in EMAIL alarm
      case ICAL_ATTENDEE_PROPERTY: {
        QString email = QString::fromUtf8(icalproperty_get_attendee(p));
        if ( email.startsWith("mailto:", false ) ) {
          email = email.mid( 7 );
        }
        QString name;
        icalparameter *param = icalproperty_get_first_parameter(p,ICAL_CN_PARAMETER);
        if (param) {
          name = QString::fromUtf8(icalparameter_get_cn(param));
        }
        ialarm->addMailAddress(Person(name, email));
        break;
      }
      // Only in AUDIO and EMAIL and PROCEDURE alarms
      case ICAL_ATTACH_PROPERTY: {
        Attachment *attach = readAttachment( p );
        if ( attach && attach->isUri() ) {
          switch ( action ) {
            case ICAL_ACTION_AUDIO:
              ialarm->setAudioFile( attach->uri() );
              break;
            case ICAL_ACTION_PROCEDURE:
              ialarm->setProgramFile( attach->uri() );
              break;
            case ICAL_ACTION_EMAIL:
              ialarm->addMailAttachment( attach->uri() );
              break;
            default:
              break;
          }
        } else {
          kdDebug() << "Alarm attachments currently only support URIs, but "
                       "no binary data" << endl;
        }
        delete attach;
        break;
      }
      default:
        break;
    }

    p = icalcomponent_get_next_property(alarm,ICAL_ANY_PROPERTY);
  }

  // custom properties
  readCustomProperties(alarm, ialarm);

  // TODO: check for consistency of alarm properties
}

icaldatetimeperiodtype ICalFormatImpl::writeICalDatePeriod( const QDate &date )
{
  icaldatetimeperiodtype t;
  t.time = writeICalDate( date );
  t.period = icalperiodtype_null_period();
  return t;
}

icaldatetimeperiodtype ICalFormatImpl::writeICalDateTimePeriod( const QDateTime &date )
{
  icaldatetimeperiodtype t;
  t.time = writeICalDateTime( date );
  t.period = icalperiodtype_null_period();
  return t;
}

icaltimetype ICalFormatImpl::writeICalDate(const QDate &date)
{
  icaltimetype t = icaltime_null_time();

  t.year = date.year();
  t.month = date.month();
  t.day = date.day();

  t.hour = 0;
  t.minute = 0;
  t.second = 0;

  t.is_date = 1;

  t.is_utc = 0;

  t.zone = 0;

  return t;
}

icaltimetype ICalFormatImpl::writeICalDateTime(const QDateTime &datetime)
{
  icaltimetype t = icaltime_null_time();

  t.year = datetime.date().year();
  t.month = datetime.date().month();
  t.day = datetime.date().day();

  t.hour = datetime.time().hour();
  t.minute = datetime.time().minute();
  t.second = datetime.time().second();

  t.is_date = 0;
  t.zone = icaltimezone_get_builtin_timezone ( mParent->timeZoneId().latin1() );
  t.is_utc = 0;

 // _dumpIcaltime( t );
  /* The QDateTime we get passed in is to be considered in the timezone of
   * the current calendar (mParent's), or, if there is none, to be floating.
   * In the later case store a floating time, in the former normalize to utc. */
  if (mParent->timeZoneId().isEmpty())
    t = icaltime_convert_to_zone( t, 0 ); //make floating timezone
  else {
    icaltimezone* tz = icaltimezone_get_builtin_timezone ( mParent->timeZoneId().latin1() );
    icaltimezone* utc = icaltimezone_get_utc_timezone();
    if ( tz != utc ) {
      t.zone = tz;
      t = icaltime_convert_to_zone( t, utc );
    } else {
      t.is_utc = 1;
      t.zone = utc;
    }
  }
//  _dumpIcaltime( t );

  return t;
}

QDateTime ICalFormatImpl::readICalDateTime( icaltimetype& t, icaltimezone* tz )
{
//   kdDebug(5800) << "ICalFormatImpl::readICalDateTime()" << endl;
  icaltimezone *zone = tz;
  if ( tz && t.is_utc == 0 ) { // Only use the TZ if time is not UTC.
    // FIXME: We'll need to make sure to apply the appropriate TZ, not just
    //        the first one found.
    t.zone = tz;
    t.is_utc = (tz == icaltimezone_get_utc_timezone())?1:0;
  } else {
    zone = icaltimezone_get_utc_timezone();
  }
  //_dumpIcaltime( t );

  // Convert to view time
  if ( !mParent->timeZoneId().isEmpty() && t.zone ) {
//    kdDebug(5800) << "--- Converting time from: " << icaltimezone_get_tzid( const_cast<icaltimezone*>( t.zone ) ) << " (" << ICalDate2QDate(t) << ")." << endl;
    icaltimezone* viewTimeZone = icaltimezone_get_builtin_timezone ( mParent->timeZoneId().latin1() );
    icaltimezone_convert_time(  &t, zone, viewTimeZone );
//    kdDebug(5800) << "--- Converted to zone " << mParent->timeZoneId() << " (" << ICalDate2QDate(t) << ")." << endl;
  }

  return ICalDate2QDate(t);
}

QDate ICalFormatImpl::readICalDate(icaltimetype t)
{
  return ICalDate2QDate(t).date();
}

icaldurationtype ICalFormatImpl::writeICalDuration(int seconds)
{
  icaldurationtype d;

  d.is_neg  = (seconds<0)?1:0;
  if (seconds<0) seconds = -seconds;

  d.weeks    = seconds / gSecondsPerWeek;
  seconds   %= gSecondsPerWeek;
  d.days     = seconds / gSecondsPerDay;
  seconds   %= gSecondsPerDay;
  d.hours    = seconds / gSecondsPerHour;
  seconds   %= gSecondsPerHour;
  d.minutes  = seconds / gSecondsPerMinute;
  seconds   %= gSecondsPerMinute;
  d.seconds  = seconds;

  return d;
}

int ICalFormatImpl::readICalDuration(icaldurationtype d)
{
  int result = 0;

  result += d.weeks   * gSecondsPerWeek;
  result += d.days    * gSecondsPerDay;
  result += d.hours   * gSecondsPerHour;
  result += d.minutes * gSecondsPerMinute;
  result += d.seconds;

  if (d.is_neg) result *= -1;

  return result;
}

icalcomponent *ICalFormatImpl::createCalendarComponent(Calendar *cal)
{
  icalcomponent *calendar;

  // Root component
  calendar = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);

  icalproperty *p;

  // Product Identifier
  p = icalproperty_new_prodid(CalFormat::productId().utf8());
  icalcomponent_add_property(calendar,p);

  // TODO: Add time zone

  // iCalendar version (2.0)
  p = icalproperty_new_version(const_cast<char *>(_ICAL_VERSION));
  icalcomponent_add_property(calendar,p);

  // Custom properties
  if( cal != 0 )
    writeCustomProperties(calendar, cal);

  return calendar;
}



// take a raw vcalendar (i.e. from a file on disk, clipboard, etc. etc.
// and break it down from its tree-like format into the dictionary format
// that is used internally in the ICalFormatImpl.
bool ICalFormatImpl::populate( Calendar *cal, icalcomponent *calendar)
{
  // this function will populate the caldict dictionary and other event
  // lists. It turns vevents into Events and then inserts them.

    if (!calendar) return false;

// TODO: check for METHOD

  icalproperty *p;

  p = icalcomponent_get_first_property(calendar,ICAL_PRODID_PROPERTY);
  if (!p) {
    kdDebug(5800) << "No PRODID property found" << endl;
    mLoadedProductId = "";
  } else {
    mLoadedProductId = QString::fromUtf8(icalproperty_get_prodid(p));
//    kdDebug(5800) << "VCALENDAR prodid: '" << mLoadedProductId << "'" << endl;

    delete mCompat;
    mCompat = CompatFactory::createCompat( mLoadedProductId );
  }

  p = icalcomponent_get_first_property(calendar,ICAL_VERSION_PROPERTY);
  if (!p) {
    kdDebug(5800) << "No VERSION property found" << endl;
    mParent->setException(new ErrorFormat(ErrorFormat::CalVersionUnknown));
    return false;
  } else {
    const char *version = icalproperty_get_version(p);
//    kdDebug(5800) << "VCALENDAR version: '" << version << "'" << endl;

    if (strcmp(version,"1.0") == 0) {
      kdDebug(5800) << "Expected iCalendar, got vCalendar" << endl;
      mParent->setException(new ErrorFormat(ErrorFormat::CalVersion1,
                            i18n("Expected iCalendar format")));
      return false;
    } else if (strcmp(version,"2.0") != 0) {
      kdDebug(5800) << "Expected iCalendar, got unknown format" << endl;
      mParent->setException(new ErrorFormat(ErrorFormat::CalVersionUnknown));
      return false;
    }
  }

  // custom properties
  readCustomProperties(calendar, cal);

// TODO: set time zone

  // read a VTIMEZONE if there is one
  icalcomponent *ctz =
    icalcomponent_get_first_component( calendar, ICAL_VTIMEZONE_COMPONENT );

  // Store all events with a relatedTo property in a list for post-processing
  mEventsRelate.clear();
  mTodosRelate.clear();
  // TODO: make sure that only actually added events go to this lists.

  icalcomponent *c;

  // Iterate through all todos
  c = icalcomponent_get_first_component(calendar,ICAL_VTODO_COMPONENT);
  while (c) {
//    kdDebug(5800) << "----Todo found" << endl;
    Todo *todo = readTodo(c);
    if (todo) {
      if (!cal->todo(todo->uid())) {
        cal->addTodo(todo);
      } else {
        delete todo;
        mTodosRelate.remove( todo );
      }
    }
    c = icalcomponent_get_next_component(calendar,ICAL_VTODO_COMPONENT);
  }

  // Iterate through all events
  c = icalcomponent_get_first_component(calendar,ICAL_VEVENT_COMPONENT);
  while (c) {
//    kdDebug(5800) << "----Event found" << endl;
    Event *event = readEvent(c, ctz);
    if (event) {
      if (!cal->event(event->uid())) {
        cal->addEvent(event);
      } else {
        delete event;
        mEventsRelate.remove( event );
      }
    }
    c = icalcomponent_get_next_component(calendar,ICAL_VEVENT_COMPONENT);
  }

  // Iterate through all journals
  c = icalcomponent_get_first_component(calendar,ICAL_VJOURNAL_COMPONENT);
  while (c) {
//    kdDebug(5800) << "----Journal found" << endl;
    Journal *journal = readJournal(c);
    if (journal) {
      if (!cal->journal(journal->uid())) {
        cal->addJournal(journal);
      } else {
        delete journal;
      }
    }
    c = icalcomponent_get_next_component(calendar,ICAL_VJOURNAL_COMPONENT);
  }

  // Post-Process list of events with relations, put Event objects in relation
  Event::List::ConstIterator eIt;
  for ( eIt = mEventsRelate.begin(); eIt != mEventsRelate.end(); ++eIt ) {
    (*eIt)->setRelatedTo( cal->incidence( (*eIt)->relatedToUid() ) );
  }
  Todo::List::ConstIterator tIt;
  for ( tIt = mTodosRelate.begin(); tIt != mTodosRelate.end(); ++tIt ) {
    (*tIt)->setRelatedTo( cal->incidence( (*tIt)->relatedToUid() ) );
   }

  return true;
}

QString ICalFormatImpl::extractErrorProperty(icalcomponent *c)
{
//  kdDebug(5800) << "ICalFormatImpl:extractErrorProperty: "
//            << icalcomponent_as_ical_string(c) << endl;

  QString errorMessage;

  icalproperty *error;
  error = icalcomponent_get_first_property(c,ICAL_XLICERROR_PROPERTY);
  while(error) {
    errorMessage += icalproperty_get_xlicerror(error);
    errorMessage += "\n";
    error = icalcomponent_get_next_property(c,ICAL_XLICERROR_PROPERTY);
  }

//  kdDebug(5800) << "ICalFormatImpl:extractErrorProperty: " << errorMessage << endl;

  return errorMessage;
}

void ICalFormatImpl::dumpIcalRecurrence(icalrecurrencetype r)
{
  int i;

  kdDebug(5800) << " Freq: " << r.freq << endl;
  kdDebug(5800) << " Until: " << icaltime_as_ical_string(r.until) << endl;
  kdDebug(5800) << " Count: " << r.count << endl;
  if (r.by_day[0] != ICAL_RECURRENCE_ARRAY_MAX) {
    int index = 0;
    QString out = " By Day: ";
    while((i = r.by_day[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
      out.append(QString::number(i) + " ");
    }
    kdDebug(5800) << out << endl;
  }
  if (r.by_month_day[0] != ICAL_RECURRENCE_ARRAY_MAX) {
    int index = 0;
    QString out = " By Month Day: ";
    while((i = r.by_month_day[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
      out.append(QString::number(i) + " ");
    }
    kdDebug(5800) << out << endl;
  }
  if (r.by_year_day[0] != ICAL_RECURRENCE_ARRAY_MAX) {
    int index = 0;
    QString out = " By Year Day: ";
    while((i = r.by_year_day[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
      out.append(QString::number(i) + " ");
    }
    kdDebug(5800) << out << endl;
  }
  if (r.by_month[0] != ICAL_RECURRENCE_ARRAY_MAX) {
    int index = 0;
    QString out = " By Month: ";
    while((i = r.by_month[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
      out.append(QString::number(i) + " ");
    }
    kdDebug(5800) << out << endl;
  }
  if (r.by_set_pos[0] != ICAL_RECURRENCE_ARRAY_MAX) {
    int index = 0;
    QString out = " By Set Pos: ";
    while((i = r.by_set_pos[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
      kdDebug(5800) << "========= " << i << endl;
      out.append(QString::number(i) + " ");
    }
    kdDebug(5800) << out << endl;
  }
}

icalcomponent *ICalFormatImpl::createScheduleComponent(IncidenceBase *incidence,
                                                   Scheduler::Method method)
{
  icalcomponent *message = createCalendarComponent();

  icalproperty_method icalmethod = ICAL_METHOD_NONE;

  switch (method) {
    case Scheduler::Publish:
      icalmethod = ICAL_METHOD_PUBLISH;
      break;
    case Scheduler::Request:
      icalmethod = ICAL_METHOD_REQUEST;
      break;
    case Scheduler::Refresh:
      icalmethod = ICAL_METHOD_REFRESH;
      break;
    case Scheduler::Cancel:
      icalmethod = ICAL_METHOD_CANCEL;
      break;
    case Scheduler::Add:
      icalmethod = ICAL_METHOD_ADD;
      break;
    case Scheduler::Reply:
      icalmethod = ICAL_METHOD_REPLY;
      break;
    case Scheduler::Counter:
      icalmethod = ICAL_METHOD_COUNTER;
      break;
    case Scheduler::Declinecounter:
      icalmethod = ICAL_METHOD_DECLINECOUNTER;
      break;
    default:
      kdDebug(5800) << "ICalFormat::createScheduleMessage(): Unknow method" << endl;
      return message;
  }

  icalcomponent_add_property(message,icalproperty_new_method(icalmethod));

  icalcomponent *inc = writeIncidence( incidence, method );
  /*
   * RFC 2446 states in section 3.4.3 ( REPLY to a VTODO ), that
   * a REQUEST-STATUS property has to be present. For the other two, event and
   * free busy, it can be there, but is optional. Until we do more
   * fine grained handling, assume all is well. Note that this is the
   * status of the _request_, not the attendee. Just to avoid confusion.
   * - till
   */
  if ( icalmethod == ICAL_METHOD_REPLY ) {
    struct icalreqstattype rst;
    rst.code = ICAL_2_0_SUCCESS_STATUS;
    rst.desc = 0;
    rst.debug = 0;
    icalcomponent_add_property( inc, icalproperty_new_requeststatus( rst ) );
  }
  icalcomponent_add_component( message, inc );

  return message;
}
