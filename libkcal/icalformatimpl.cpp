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

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qfile.h>
#include <cstdlib>

#include <kdebug.h>
#include <klocale.h>

extern "C" {
  #include <ical.h>
  #include <icalss.h>
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

namespace KCal {

/**
 * A TimezonePhase represents a setting within a timezone, e.g. standard or
 * daylight savings.
 */
typedef struct icaltimezonephase icaltimezonephase;
class TimezonePhase : private icaltimezonephase {
  public:
    /**
     * Contructor for a timezone phase.
     */
    TimezonePhase(ICalFormatImpl *parent, icalcomponent *c)
    {
      tzname = (const char *)0;
      is_stdandard = 1;
      mIsStandard = 1;
      dtstart = icaltime_null_time();
      offsetto = 0;
      tzoffsetfrom = 0;
      comment = (const char *)0;
      rdate.time = icaltime_null_time();
      rdate.period = icalperiodtype_null_period();
      rrule = (const char *)0;
      mRrule = new Recurrence((Incidence *)0);

      // Now do the ical reading.
      icalproperty *p = icalcomponent_get_first_property(c,ICAL_ANY_PROPERTY);
      while (p) {
        icalproperty_kind kind = icalproperty_isa(p);
        switch (kind) {

          case ICAL_TZNAME_PROPERTY:
            tzname = icalproperty_get_tzname(p);
            break;

          case ICAL_DTSTART_PROPERTY:
            dtstart = icalproperty_get_dtstart(p);
            break;

          case ICAL_TZOFFSETTO_PROPERTY:
            offsetto = icalproperty_get_tzoffsetto(p);
            break;

          case ICAL_TZOFFSETFROM_PROPERTY:
            tzoffsetfrom = icalproperty_get_tzoffsetfrom(p);
            break;

          case ICAL_COMMENT_PROPERTY:
            comment = icalproperty_get_comment(p);
            break;

          case ICAL_RDATE_PROPERTY:
            rdate = icalproperty_get_rdate(p);
            break;

          case ICAL_RRULE_PROPERTY:
            {
              struct icalrecurrencetype r = icalproperty_get_rrule(p);

              parent->readRecurrence(r,mRrule);
            }
            break;

          default:
            kdDebug(5800) << "TimezonePhase::TimezonePhase(): Unknown property: " << kind
                      << endl;
            break;
        }
        p = icalcomponent_get_next_property(c,ICAL_ANY_PROPERTY);
      }
    }

    /**
     * Destructor for a timezone phase.
     */
    ~TimezonePhase()
    {
      delete mRrule;
    }

    /**
     * Find the nearest start time of this phase just before a given time.
     */
    QDateTime nearestStart(const QDateTime &t) const
    {
      QDateTime tmp(QDate(dtstart.year,dtstart.month,dtstart.day), QTime(dtstart.hour,dtstart.minute,dtstart.second));
      // If this phase was not valid at the given time, give up.
      if (tmp > t) {
        kdDebug(5800) << "TimezonePhase::nearestStart(): Phase not valid" << endl;
        return QDateTime();
      }

      // The Recurrance class's getPreviousDateTime() logic was not designed for
      // start times which are not aligned with a reference time, but a little
      // magic is sufficient to work around that...
      QDateTime previous = mRrule->getPreviousDateTime(tmp);
      if (mRrule->getNextDateTime(previous) < tmp)
        previous = mRrule->getNextDateTime(previous);
      return previous;
    }

    /**
     * Offset of this phase in seconds.
     */
    int offset() const
    {
      return offsetto;
    }

    // Hide the missnamed "is_stdandard" variable in the base class.
    int mIsStandard;

    // Supplement the "rrule" in the base class.
    Recurrence *mRrule;
};

/**
 * A Timezone.
 */
typedef struct icaltimezonetype icaltimezonetype;
class Timezone : private icaltimezonetype {
  public:
    /**
     * Contructor for a timezone.
     */
    Timezone(ICalFormatImpl *parent, icalcomponent *vtimezone)
    {
      tzid = (const char *)0;
      last_mod = icaltime_null_time();
      tzurl = (const char *)0;

      // The phases list is defined to be terminated by a phase with a
      // null name.
      phases = (icaltimezonephase *)malloc(sizeof(*phases));
      phases[0].tzname = (const char *)0;
      mPhases.setAutoDelete( true );

      // Now do the ical reading.
      icalproperty *p = icalcomponent_get_first_property(vtimezone,ICAL_ANY_PROPERTY);
      while (p) {
        icalproperty_kind kind = icalproperty_isa(p);
        switch (kind) {

          case ICAL_TZID_PROPERTY:
            // The timezone id is basically a unique string which is used to
            // identify this timezone. Note that if it begins with a "/", then it
            // is suppsed to have some externally specified meaning, but we are
            // just after its unique value.
            tzid = icalproperty_get_tzid(p);
            break;

          case ICAL_TZURL_PROPERTY:
            tzurl = icalproperty_get_tzurl(p);
            break;

          default:
            kdDebug(5800) << "Timezone::Timezone(): Unknown property: " << kind
                      << endl;
            break;
        }
        p = icalcomponent_get_next_property(vtimezone,ICAL_ANY_PROPERTY);
      }
      kdDebug(5800) << "---zoneId: \"" << tzid << '"' << endl;

      icalcomponent *c;

      TimezonePhase *phase;

      // Iterate through all timezones before we do anything else. That way, the
      // information needed to interpret times in actually usefulobject is
      // available below.
      c = icalcomponent_get_first_component(vtimezone,ICAL_ANY_COMPONENT);
      while (c) {
        icalcomponent_kind kind = icalcomponent_isa(c);
        switch (kind) {

          case ICAL_XSTANDARD_COMPONENT:
            kdDebug(5800) << "---standard phase: found" << endl;
            phase = new TimezonePhase(parent,c);
            phase->mIsStandard = 1;
            mPhases.append(phase);
            break;

          case ICAL_XDAYLIGHT_COMPONENT:
            kdDebug(5800) << "---daylight phase: found" << endl;
            phase = new TimezonePhase(parent,c);
            phase->mIsStandard = 0;
            mPhases.append(phase);
            break;

          default:
            kdDebug(5800) << "Timezone::Timezone(): Unknown component: " << kind
                      << endl;
            break;
        }
        c = icalcomponent_get_next_component(vtimezone,ICAL_ANY_COMPONENT);
      }
    }

    /**
     * Destructor for a timezone.
     */
    ~Timezone()
    {
      free(phases);
    }

    /**
     * The string id of this timezone. Make sure we always have quotes!
     */
    QString id() const
    {
      if (tzid[0] != '"') {
        return QString("\"") + tzid + '"';
      } else {
        return tzid;
      }
    }

    /**
     * Find the nearest timezone phase just before a given time.
     */
    const TimezonePhase *nearestStart(const QDateTime &t)
    {
      unsigned i;
      unsigned result = 0;
      QDateTime previous;
      QDateTime next;

      // Main loop. Find the phase with the latest start date before t.
      for (i = 0; i < mPhases.count(); i++) {
        next = mPhases.at(i)->nearestStart(t);
        if (previous.isNull() || previous < next) {
          previous = next;
          result = i;
        }
      }
      return mPhases.at(result);
    }

    /**
     * Convert the given time to UTC.
     */
    int offset(icaltimetype t)
    {
      QDateTime tmp(QDate(t.year,t.month,t.day), QTime(t.hour,t.minute,t.second));
      const TimezonePhase *phase = nearestStart(tmp);

      if (phase) {
        return phase->offset();
      } else {
        kdError(5800) << "Timezone::offset() cannot find phase for " << tmp << endl;
        return 0;
      }
    }

    // Phases we have seen.
    QPtrList<TimezonePhase> mPhases;
};

}

const int gSecondsPerMinute = 60;
const int gSecondsPerHour   = gSecondsPerMinute * 60;
const int gSecondsPerDay    = gSecondsPerHour   * 24;
const int gSecondsPerWeek   = gSecondsPerDay    * 7;

ICalFormatImpl::ICalFormatImpl( ICalFormat *parent ) :
  mParent( parent ), mCalendarVersion( 0 ), mCompat( new Compat )
{
  mTimezones.setAutoDelete( true );
}

ICalFormatImpl::~ICalFormatImpl()
{
  if ( mCompat ) delete mCompat;
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
    // end time
    icaltimetype end;
    if (event->doesFloat()) {
//      kdDebug(5800) << " Event " << event->summary() << " floats." << endl;
      // +1 day because end date is non-inclusive.
      end = writeICalDate( event->dtEnd().date().addDays( 1 ) );
    } else {
//      kdDebug(5800) << " Event " << event->summary() << " has time." << endl;
      end = writeICalDateTime(event->dtEnd());
    }
    icalcomponent_add_property(vevent,icalproperty_new_dtend(end));
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
    icalcomponent_add_property(vevent, icalproperty_new_transp("TRANSPARENT"));
    break;
  case Event::Opaque:
    icalcomponent_add_property(vevent, icalproperty_new_transp("OPAQUE"));
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
  icalperiodtype period;
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
    incidence->setNonKDECustomProperty("X-PILOTID", QString::number(incidence->pilotId()));
    incidence->setNonKDECustomProperty("X-PILOTSTAT", QString::number(incidence->syncStatus()));
  }

  writeIncidenceBase(parent,incidence);

  // creation date
  icalcomponent_add_property(parent,icalproperty_new_created(
      writeICalDateTime(incidence->created())));

  // unique id
  icalcomponent_add_property(parent,icalproperty_new_uid(
      incidence->uid().utf8()));

  // revision
  icalcomponent_add_property(parent,icalproperty_new_sequence(
      incidence->revision()));

  // last modification date
  icalcomponent_add_property(parent,icalproperty_new_lastmodified(
      writeICalDateTime(incidence->lastModified())));

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
  const char *classStr;
  switch (incidence->secrecy()) {
    case Incidence::SecrecyPublic:
      classStr = "PUBLIC";
      break;
    case Incidence::SecrecyConfidential:
      classStr = "CONFIDENTIAL";
      break;
    case Incidence::SecrecyPrivate:
    default:
      classStr = "PRIVATE";
      break;
  }
  icalcomponent_add_property(parent,icalproperty_new_class(classStr));

  // priority
  icalcomponent_add_property(parent,icalproperty_new_priority(
      incidence->priority()));

  // categories
  QStringList categories = incidence->categories();
  QStringList::Iterator it;
  for(it = categories.begin(); it != categories.end(); ++it ) {
    icalcomponent_add_property(parent,icalproperty_new_categories((*it).utf8()));
  }
// TODO: Ensure correct concatenation of categories properties.

/*
  // categories
  tmpStrList = incidence->getCategories();
  tmpStr = "";
  QString catStr;
  for ( QStringList::Iterator it = tmpStrList.begin();
        it != tmpStrList.end();
        ++it ) {
    catStr = *it;
    if (catStr[0] == ' ')
      tmpStr += catStr.mid(1);
    else
      tmpStr += catStr;
    // this must be a ';' character as the vCalendar specification requires!
    // vcc.y has been hacked to translate the ';' to a ',' when the vcal is
    // read in.
    tmpStr += ";";
  }
  if (!tmpStr.isEmpty()) {
    tmpStr.truncate(tmpStr.length()-1);
    icalcomponent_add_property(parent,icalproperty_new_categories(
        writeText(incidence->getCategories().join(";"))));
  }
*/

  // related event
  if (incidence->relatedTo()) {
    icalcomponent_add_property(parent,icalproperty_new_relatedto(
        incidence->relatedTo()->uid().utf8()));
  }

  // recurrence rule stuff
  if (incidence->doesRecur()) {
    kdDebug(5800) << "Write recurrence for '" << incidence->summary() << "' (" << incidence->uid()
              << ")" << endl;
    icalcomponent_add_property(parent,writeRecurrenceRule(incidence->recurrence()));
  }

  // recurrence exception dates and date/times
  DateList dateList = incidence->exDates();
  DateList::ConstIterator exIt;
  for(exIt = dateList.begin(); exIt != dateList.end(); ++exIt) {
    icalcomponent_add_property(parent,icalproperty_new_exdate(
        writeICalDate(*exIt)));
  }
  DateTimeList dateTimeList = incidence->exDateTimes();
  DateTimeList::ConstIterator extIt;
  for(extIt = dateTimeList.begin(); extIt != dateTimeList.end(); ++extIt) {
    icalcomponent_add_property(parent,icalproperty_new_exdate(
        writeICalDateTime(*extIt)));
  }

  // attachments
  Attachment::List attachments = incidence->attachments();
  Attachment::List::ConstIterator atIt;
  for ( atIt = attachments.begin(); atIt != attachments.end(); ++atIt )
    icalcomponent_add_property( parent, writeAttachment( *atIt ) );

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

// @TODO: turned off as it always is set to PT0S (and must not occur together with DTEND

//  if (incidence->hasDuration()) {
//    icaldurationtype duration;
//    duration = writeICalDuration(incidence->duration());
//    icalcomponent_add_property(parent,icalproperty_new_duration(duration));
//  }
}

void ICalFormatImpl::writeIncidenceBase( icalcomponent *parent,
                                         IncidenceBase * incidenceBase )
{
  icalcomponent_add_property( parent, icalproperty_new_dtstamp(
      writeICalDateTime( QDateTime::currentDateTime() ) ) );

  // organizer stuff
  icalcomponent_add_property( parent, writeOrganizer( incidenceBase->organizer() ) );

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
    icalproperty_add_parameter( p, icalparameter_new_cn(organizer.name().utf8()) );
  }
  // TODO: Write dir, senty-by and language

  return p;
}


icalproperty *ICalFormatImpl::writeAttendee(Attendee *attendee)
{
  icalproperty *p = icalproperty_new_attendee("mailto:" + attendee->email().utf8());

  if (!attendee->name().isEmpty()) {
    icalproperty_add_parameter(p,icalparameter_new_cn(attendee->name().utf8()));
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

  return p;
}

icalproperty *ICalFormatImpl::writeAttachment(Attachment *att)
{
  icalattachtype *attach = icalattachtype_new();
  if ( att->isUri() )
    icalattachtype_set_url( attach, att->uri().utf8().data() );
  else
    icalattachtype_set_base64( attach, att->data(), 0 );

  icalproperty *p = icalproperty_new_attach( attach );
  icalattachtype_free( attach );

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
  return p;
}

icalproperty *ICalFormatImpl::writeRecurrenceRule(Recurrence *recur)
{
//  kdDebug(5800) << "ICalFormatImpl::writeRecurrenceRule()" << endl;

  icalrecurrencetype r;

  icalrecurrencetype_clear(&r);

  int index = 0;
  int index2 = 0;

  QPtrList<Recurrence::rMonthPos> tmpPositions;
  QPtrList<int> tmpDays;
  int *tmpDay;
  Recurrence::rMonthPos *tmpPos;
  bool datetime = false;
  int day;
  int i;

  switch(recur->doesRecur()) {
    case Recurrence::rMinutely:
      r.freq = ICAL_MINUTELY_RECURRENCE;
      datetime = true;
      break;
    case Recurrence::rHourly:
      r.freq = ICAL_HOURLY_RECURRENCE;
      datetime = true;
      break;
    case Recurrence::rDaily:
      r.freq = ICAL_DAILY_RECURRENCE;
      break;
    case Recurrence::rWeekly:
      r.freq = ICAL_WEEKLY_RECURRENCE;
      r.week_start = static_cast<icalrecurrencetype_weekday>(recur->weekStart()%7 + 1);
      for (i = 0; i < 7; i++) {
        if (recur->days().testBit(i)) {
          day = (i + 1)%7 + 1;     // convert from Monday=0 to Sunday=1
          r.by_day[index++] = icalrecurrencetype_day_day_of_week(day);
        }
      }
//      r.by_day[index] = ICAL_RECURRENCE_ARRAY_MAX;
      break;
    case Recurrence::rMonthlyPos:
      r.freq = ICAL_MONTHLY_RECURRENCE;

      tmpPositions = recur->monthPositions();
      for (tmpPos = tmpPositions.first();
           tmpPos;
           tmpPos = tmpPositions.next()) {
        for (i = 0; i < 7; i++) {
          if (tmpPos->rDays.testBit(i)) {
            day = (i + 1)%7 + 1;     // convert from Monday=0 to Sunday=1
            day += tmpPos->rPos*8;
            if (tmpPos->negative) day = -day;
            r.by_day[index++] = day;
          }
        }
      }
//      r.by_day[index] = ICAL_RECURRENCE_ARRAY_MAX;
      break;
    case Recurrence::rMonthlyDay:
      r.freq = ICAL_MONTHLY_RECURRENCE;

      tmpDays = recur->monthDays();
      for (tmpDay = tmpDays.first();
           tmpDay;
           tmpDay = tmpDays.next()) {
        r.by_month_day[index++] = icalrecurrencetype_day_position(*tmpDay*8);
      }
//      r.by_day[index] = ICAL_RECURRENCE_ARRAY_MAX;
      break;
    case Recurrence::rYearlyMonth:
    case Recurrence::rYearlyPos:
      r.freq = ICAL_YEARLY_RECURRENCE;

      tmpDays = recur->yearNums();
      for (tmpDay = tmpDays.first();
           tmpDay;
           tmpDay = tmpDays.next()) {
        r.by_month[index++] = *tmpDay;
      }
//      r.by_set_pos[index] = ICAL_RECURRENCE_ARRAY_MAX;
      if (recur->doesRecur() == Recurrence::rYearlyPos) {
        tmpPositions = recur->monthPositions();
        for (tmpPos = tmpPositions.first();
             tmpPos;
             tmpPos = tmpPositions.next()) {
          for (i = 0; i < 7; i++) {
            if (tmpPos->rDays.testBit(i)) {
              day = (i + 1)%7 + 1;     // convert from Monday=0 to Sunday=1
              day += tmpPos->rPos*8;
              if (tmpPos->negative) day = -day;
              r.by_day[index2++] = day;
            }
          }
        }
//        r.by_day[index2] = ICAL_RECURRENCE_ARRAY_MAX;
      }
      else {
        tmpDays = recur->monthDays();
        for (tmpDay = tmpDays.first();
             tmpDay;
             tmpDay = tmpDays.next()) {
          r.by_month_day[index2++] = icalrecurrencetype_day_position(*tmpDay*8);
        }
//        r.by_month_day[index2] = ICAL_RECURRENCE_ARRAY_MAX;
      }
      break;
    case Recurrence::rYearlyDay:
      r.freq = ICAL_YEARLY_RECURRENCE;

      tmpDays = recur->yearNums();
      for (tmpDay = tmpDays.first();
           tmpDay;
           tmpDay = tmpDays.next()) {
        r.by_year_day[index++] = *tmpDay;
      }
//      r.by_year_day[index] = ICAL_RECURRENCE_ARRAY_MAX;
      break;
    default:
      r.freq = ICAL_NO_RECURRENCE;
      kdDebug(5800) << "ICalFormatImpl::writeRecurrence(): no recurrence" << endl;
      break;
  }

  r.interval = recur->frequency();

  if (recur->duration() > 0) {
    r.count = recur->duration();
  } else if (recur->duration() == -1) {
    r.count = 0;
  } else {
    if (datetime)
      r.until = writeICalDateTime(recur->endDateTime());
    else
      r.until = writeICalDate(recur->endDate());
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

  return icalproperty_new_rrule(r);
}

icalcomponent *ICalFormatImpl::writeAlarm(Alarm *alarm)
{
  icalcomponent *a = icalcomponent_new(ICAL_VALARM_COMPONENT);

  icalproperty_action action;
  icalattachtype *attach = 0;

  switch (alarm->type()) {
    case Alarm::Procedure:
      action = ICAL_ACTION_PROCEDURE;
      attach = icalattachtype_new();
      icalattachtype_set_url(attach,QFile::encodeName(alarm->programFile()).data());
      icalcomponent_add_property(a,icalproperty_new_attach(attach));
      icalattachtype_free(attach);
      if (!alarm->programArguments().isEmpty()) {
        icalcomponent_add_property(a,icalproperty_new_description(alarm->programArguments().utf8()));
      }
      break;
    case Alarm::Audio:
      action = ICAL_ACTION_AUDIO;
      if (!alarm->audioFile().isEmpty()) {
        attach = icalattachtype_new();
        icalattachtype_set_url(attach,QFile::encodeName( alarm->audioFile() ).data());
        icalcomponent_add_property(a,icalproperty_new_attach(attach));
        icalattachtype_free(attach);
      }
      break;
    case Alarm::Email: {
      action = ICAL_ACTION_EMAIL;
      QValueList<Person> addresses = alarm->mailAddresses();
      for (QValueList<Person>::Iterator ad = addresses.begin();  ad != addresses.end();  ++ad) {
        icalproperty *p = icalproperty_new_attendee("MAILTO:" + (*ad).email().utf8());
        if (!(*ad).name().isEmpty()) {
          icalproperty_add_parameter(p,icalparameter_new_cn((*ad).name().utf8()));
        }
        icalcomponent_add_property(a,p);
      }
      icalcomponent_add_property(a,icalproperty_new_summary(alarm->mailSubject().utf8()));
      icalcomponent_add_property(a,icalproperty_new_description(alarm->mailText().utf8()));
      QStringList attachments = alarm->mailAttachments();
      if (attachments.count() > 0) {
        for (QStringList::Iterator at = attachments.begin();  at != attachments.end();  ++at) {
          attach = icalattachtype_new();
          icalattachtype_set_url(attach,QFile::encodeName( *at ).data());
          icalcomponent_add_property(a,icalproperty_new_attach(attach));
          icalattachtype_free(attach);
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

// Read a timezone and store it in a list where it can be accessed as needed
// by the other readXXX() routines. Note that no writeTimezone is needed
// because we always store in UTC.
void ICalFormatImpl::readTimezone(icalcomponent *vtimezone)
{
  Timezone *timezone = new Timezone(this, vtimezone);

  mTimezones.insert(timezone->id(), timezone);
}

Todo *ICalFormatImpl::readTodo(icalcomponent *vtodo)
{
  Todo *todo = new Todo;

  readIncidence(vtodo,todo);

  icalproperty *p = icalcomponent_get_first_property(vtodo,ICAL_ANY_PROPERTY);

//  int intvalue;
  icaltimetype icaltime;

  QStringList categories;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_DUE_PROPERTY:  // due date
        icaltime = icalproperty_get_due(p);
        readTzidParameter(p,icaltime);
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
        readTzidParameter(p,icaltime);
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
        readTzidParameter(p,icaltime);
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

Event *ICalFormatImpl::readEvent(icalcomponent *vevent)
{
  Event *event = new Event;

  readIncidence(vevent,event);

  icalproperty *p = icalcomponent_get_first_property(vevent,ICAL_ANY_PROPERTY);

//  int intvalue;
  icaltimetype icaltime;

  QStringList categories;
  QString transparency;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_DTEND_PROPERTY:  // start date and time
        icaltime = icalproperty_get_dtend(p);
        readTzidParameter(p,icaltime);
        if (icaltime.is_date) {
          // End date is non-inclusive
          QDate endDate = readICalDate( icaltime ).addDays( -1 );
          if ( mCompat ) mCompat->fixFloatingEnd( endDate );
          if ( endDate < event->dtStart().date() ) {
            endDate = event->dtStart().date();
          }
          event->setDtEnd( QDateTime( endDate, QTime( 0, 0, 0 ) ) );
        } else {
          event->setDtEnd(readICalDateTime(icaltime));
          event->setFloats( false );
        }
        break;

// TODO:
  // at this point, there should be at least a start or end time.
  // fix up for events that take up no time but have a time associated
#if 0
  if (!(vo = isAPropertyOf(vevent, VCDTstartProp)))
    anEvent->setDtStart(anEvent->dtEnd());
  if (!(vo = isAPropertyOf(vevent, VCDTendProp)))
    anEvent->setDtEnd(anEvent->dtStart());
#endif

#if 0
  // secrecy
  if ((vo = isAPropertyOf(vevent, VCClassProp)) != 0) {
    anEvent->setSecrecy(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }
  else
    anEvent->setSecrecy("PUBLIC");

  // attachments
  tmpStrList.clear();
  initPropIterator(&voi, vevent);
  while (moreIteration(&voi)) {
    vo = nextVObject(&voi);
    if (strcmp(vObjectName(vo), VCAttachProp) == 0) {
      tmpStrList.append(s = fakeCString(vObjectUStringZValue(vo)));
      deleteStr(s);
    }
  }
  anEvent->setAttachments(tmpStrList);

  // resources
  if ((vo = isAPropertyOf(vevent, VCResourcesProp)) != 0) {
    QString resources = (s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
    tmpStrList.clear();
    index1 = 0;
    index2 = 0;
    QString resource;
    while ((index2 = resources.find(';', index1)) != -1) {
      resource = resources.mid(index1, (index2 - index1));
      tmpStrList.append(resource);
      index1 = index2;
    }
    anEvent->setResources(tmpStrList);
  }
#endif

      case ICAL_RELATEDTO_PROPERTY:  // related event (parent)
        event->setRelatedToUid(QString::fromUtf8(icalproperty_get_relatedto(p)));
        mEventsRelate.append(event);
        break;


      case ICAL_TRANSP_PROPERTY:  // Transparency
        transparency = QString::fromUtf8(icalproperty_get_transp(p));
        if( transparency == "TRANSPARENT" )
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

  QString msade = event->nonKDECustomProperty("X-MICROSOFT-CDO-ALLDAYEVENT");
  if (!msade.isNull()) {
    bool floats = (msade == QString::fromLatin1("TRUE"));
    kdDebug(5800) << "ICALFormat::readEvent(): all day event: " << floats << endl;
    event->setFloats(floats);
    if (floats) {
      QDateTime endDate = event->dtEnd();
      event->setDtEnd(endDate.addDays(-1));
    }
  }

  if ( mCompat ) mCompat->fixEmptySummary( event );

  return event;
}

FreeBusy *ICalFormatImpl::readFreeBusy(icalcomponent *vfreebusy)
{
  FreeBusy *freebusy = new FreeBusy;

  readIncidenceBase(vfreebusy,freebusy);

  icalproperty *p = icalcomponent_get_first_property(vfreebusy,ICAL_ANY_PROPERTY);

  icaltimetype icaltime;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_DTSTART_PROPERTY:  // start date and time
        icaltime = icalproperty_get_dtstart(p);
        readTzidParameter(p,icaltime);
        freebusy->setDtStart(readICalDateTime(icaltime));
        break;

      case ICAL_DTEND_PROPERTY:  // end Date and Time
        icaltime = icalproperty_get_dtend(p);
        readTzidParameter(p,icaltime);
        freebusy->setDtEnd(readICalDateTime(icaltime));
        break;

      case ICAL_FREEBUSY_PROPERTY: { //Any FreeBusy Times
        icalperiodtype icalperiod = icalproperty_get_freebusy(p);
        readTzidParameter(p,icalperiod.start);
        QDateTime period_start = readICalDateTime(icalperiod.start);
        if ( !icaltime_is_null_time(icalperiod.end) ) {
          readTzidParameter(p,icalperiod.end);
          QDateTime period_end = readICalDateTime(icalperiod.end);
          freebusy->addPeriod( period_start, period_end );
        } else {
          Duration duration = readICalDuration( icalperiod.duration );
          freebusy->addPeriod( period_start, duration );
        }
        break;}

      default:
//        kdDebug(5800) << "ICalFormatImpl::readIncidence(): Unknown property: "
//                      << kind << endl;
      break;
    }
    p = icalcomponent_get_next_property(vfreebusy,ICAL_ANY_PROPERTY);
  }

  return freebusy;
}

Journal *ICalFormatImpl::readJournal(icalcomponent *vjournal)
{
  Journal *journal = new Journal;

  readIncidence(vjournal,journal);

  return journal;
}

Attendee *ICalFormatImpl::readAttendee(icalproperty *attendee)
{
  icalparameter *p = 0;

  QString email = QString::fromUtf8(icalproperty_get_attendee(attendee));

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
  /*while (p) {
   // if (icalparameter_get_xname(p) == "X-UID") {
    uid = icalparameter_get_xvalue(p);
    p = icalproperty_get_next_parameter(attendee,ICAL_X_PARAMETER);
  } */

  return new Attendee( name, email, rsvp, status, role, uid );
}

Person ICalFormatImpl::readOrganizer( icalproperty *organizer )
{
  QString email = QString::fromUtf8(icalproperty_get_organizer(organizer));
  if ( email.startsWith("mailto:", false ) ) {
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
  icalattachtype *a = icalproperty_get_attach(attach);
  icalparameter_value v = ICAL_VALUE_NONE;
  icalparameter_encoding e = ICAL_ENCODING_NONE;

  Attachment *attachment = 0;

  icalparameter *vp = icalproperty_get_first_parameter(attach, ICAL_VALUE_PARAMETER);
  if (vp)
    v = icalparameter_get_value(vp);

  icalparameter *ep = icalproperty_get_first_parameter(attach, ICAL_ENCODING_PARAMETER);
  if (ep)
    e = icalparameter_get_encoding(ep);

  if (v == ICAL_VALUE_BINARY && e == ICAL_ENCODING_BASE64) {
    attachment = new Attachment(icalattachtype_get_base64(a));
  } else if ((v == ICAL_VALUE_NONE || v == ICAL_VALUE_URI) && (e == ICAL_ENCODING_NONE || e == ICAL_ENCODING_8BIT)) {
    const char *u = icalattachtype_get_url(a);
    attachment = new Attachment( QString( u ) );
  } else {
    kdWarning(5800) << "Unsupported attachment format, discarding it!" << endl;
    return 0;
  }

  icalparameter *p = icalproperty_get_first_parameter(attach, ICAL_FMTTYPE_PARAMETER);
  if (p)
    attachment->setMimeType(QString(icalparameter_get_fmttype(p)));

  return attachment;
}

void ICalFormatImpl::readIncidence(icalcomponent *parent,Incidence *incidence)
{
  readIncidenceBase(parent,incidence);

  icalproperty *p = icalcomponent_get_first_property(parent,ICAL_ANY_PROPERTY);

  const char *text;
  int intvalue;
  icaltimetype icaltime;
  icaldurationtype icalduration;

  QStringList categories;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_CREATED_PROPERTY:
        icaltime = icalproperty_get_created(p);
        readTzidParameter(p,icaltime);
        incidence->setCreated(readICalDateTime(icaltime));
        break;

      case ICAL_SEQUENCE_PROPERTY:  // sequence
        intvalue = icalproperty_get_sequence(p);
        incidence->setRevision(intvalue);
        break;

      case ICAL_LASTMODIFIED_PROPERTY:  // last modification date
        icaltime = icalproperty_get_lastmodified(p);
        readTzidParameter(p,icaltime);
        incidence->setLastModified(readICalDateTime(icaltime));
        break;

      case ICAL_DTSTART_PROPERTY:  // start date and time
        icaltime = icalproperty_get_dtstart(p);
        readTzidParameter(p,icaltime);
        if (icaltime.is_date) {
          incidence->setDtStart(QDateTime(readICalDate(icaltime),QTime(0,0,0)));
        } else {
          incidence->setDtStart(readICalDateTime(icaltime));
          incidence->setFloats(false);
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
        readRecurrenceRule(p,incidence);
        break;

      case ICAL_EXDATE_PROPERTY:
        icaltime = icalproperty_get_exdate(p);
        readTzidParameter(p,icaltime);
        if (icaltime.is_date) {
          incidence->addExDate(readICalDate(icaltime));
        } else {
          incidence->addExDateTime(readICalDateTime(icaltime));
        }
        break;

      case ICAL_CLASS_PROPERTY:
        text = icalproperty_get_class(p);
        if (strcmp(text,"PUBLIC") == 0) {
          incidence->setSecrecy(Incidence::SecrecyPublic);
        } else if (strcmp(text,"CONFIDENTIAL") == 0) {
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

  // kpilot stuff
// TODO: move this application-specific code to kpilot
  QString kp = incidence->nonKDECustomProperty("X-PILOTID");
  if (!kp.isNull()) {
    incidence->setPilotId(kp.toInt());
  }
  kp = incidence->nonKDECustomProperty("X-PILOTSTAT");
  if (!kp.isNull()) {
    incidence->setSyncStatus(kp.toInt());
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

  // custom properties
  readCustomProperties(parent, incidenceBase);
}

void ICalFormatImpl::readCustomProperties(icalcomponent *parent,CustomProperties *properties)
{
  QMap<QCString, QString> customProperties;

  icalproperty *p = icalcomponent_get_first_property(parent,ICAL_X_PROPERTY);

  while (p) {

    QString value = QString::fromUtf8(icalproperty_get_x(p));
    customProperties[icalproperty_get_name(p)] = value;

    p = icalcomponent_get_next_property(parent,ICAL_X_PROPERTY);
  }

  properties->setCustomProperties(customProperties);
}

void ICalFormatImpl::readRecurrenceRule(icalproperty *rrule,Incidence *incidence)
{
//  kdDebug(5800) << "Read recurrence for " << incidence->summary() << endl;

  Recurrence *recur = incidence->recurrence();
  recur->setCompatVersion(mCalendarVersion);
  recur->unsetRecurs();

  struct icalrecurrencetype r = icalproperty_get_rrule(rrule);

  dumpIcalRecurrence(r);

  readRecurrence( r, recur );
}

void ICalFormatImpl::readRecurrence( const struct icalrecurrencetype &r, Recurrence* recur )
{
  int wkst;
  int index = 0;
  short day = 0;
  QBitArray qba(7);

  switch (r.freq) {
    case ICAL_MINUTELY_RECURRENCE:
      if (!icaltime_is_null_time(r.until)) {
        recur->setMinutely(r.interval,readICalDateTime(r.until));
      } else {
        if (r.count == 0)
          recur->setMinutely(r.interval,-1);
        else
          recur->setMinutely(r.interval,r.count);
      }
      break;
    case ICAL_HOURLY_RECURRENCE:
      if (!icaltime_is_null_time(r.until)) {
        recur->setHourly(r.interval,readICalDateTime(r.until));
      } else {
        if (r.count == 0)
          recur->setHourly(r.interval,-1);
        else
          recur->setHourly(r.interval,r.count);
      }
      break;
    case ICAL_DAILY_RECURRENCE:
      if (!icaltime_is_null_time(r.until)) {
        recur->setDaily(r.interval,readICalDate(r.until));
      } else {
        if (r.count == 0)
          recur->setDaily(r.interval,-1);
        else
          recur->setDaily(r.interval,r.count);
      }
      break;
    case ICAL_WEEKLY_RECURRENCE:
//      kdDebug(5800) << "WEEKLY_RECURRENCE" << endl;
      wkst = (r.week_start + 5)%7 + 1;
      if (!icaltime_is_null_time(r.until)) {
        recur->setWeekly(r.interval,qba,readICalDate(r.until),wkst);
      } else {
        if (r.count == 0)
          recur->setWeekly(r.interval,qba,-1,wkst);
        else
          recur->setWeekly(r.interval,qba,r.count,wkst);
      }
      while((day = r.by_day[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
//        kdDebug(5800) << " " << day << endl;
        qba.setBit((day+5)%7);    // convert from Sunday=1 to Monday=0
      }
      break;
    case ICAL_MONTHLY_RECURRENCE:
      if (r.by_day[0] != ICAL_RECURRENCE_ARRAY_MAX) {
        if (!icaltime_is_null_time(r.until)) {
          recur->setMonthly(Recurrence::rMonthlyPos,r.interval,
                            readICalDate(r.until));
        } else {
          if (r.count == 0)
            recur->setMonthly(Recurrence::rMonthlyPos,r.interval,-1);
          else
            recur->setMonthly(Recurrence::rMonthlyPos,r.interval,r.count);
        }
        bool useSetPos = false;
        short pos = 0;
        while((day = r.by_day[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
//          kdDebug(5800) << "----a " << index << ": " << day << endl;
          pos = icalrecurrencetype_day_position(day);
          if (pos) {
            day = icalrecurrencetype_day_day_of_week(day);
            QBitArray ba(7);          // don't wipe qba
            ba.setBit((day+5)%7);     // convert from Sunday=1 to Monday=0
            recur->addMonthlyPos(pos,ba);
          } else {
            qba.setBit((day+5)%7);    // convert from Sunday=1 to Monday=0
            useSetPos = true;
          }
        }
        if (useSetPos) {
          if (r.by_set_pos[0] != ICAL_RECURRENCE_ARRAY_MAX) {
            recur->addMonthlyPos(r.by_set_pos[0],qba);
          }
        }
      } else if (r.by_month_day[0] != ICAL_RECURRENCE_ARRAY_MAX) {
        if (!icaltime_is_null_time(r.until)) {
          recur->setMonthly(Recurrence::rMonthlyDay,r.interval,
                            readICalDate(r.until));
        } else {
          if (r.count == 0)
            recur->setMonthly(Recurrence::rMonthlyDay,r.interval,-1);
          else
            recur->setMonthly(Recurrence::rMonthlyDay,r.interval,r.count);
        }
        while((day = r.by_month_day[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
//          kdDebug(5800) << "----b " << day << endl;
          recur->addMonthlyDay(day);
        }
      }
      break;
    case ICAL_YEARLY_RECURRENCE:
      if (r.by_year_day[0] != ICAL_RECURRENCE_ARRAY_MAX) {
        if (!icaltime_is_null_time(r.until)) {
          recur->setYearly(Recurrence::rYearlyDay,r.interval,
                            readICalDate(r.until));
        } else {
          if (r.count == 0)
            recur->setYearly(Recurrence::rYearlyDay,r.interval,-1);
          else
            recur->setYearly(Recurrence::rYearlyDay,r.interval,r.count);
        }
        while((day = r.by_year_day[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
          recur->addYearlyNum(day);
        }
      } if (r.by_month[0] != ICAL_RECURRENCE_ARRAY_MAX) {
        if (r.by_day[0] != ICAL_RECURRENCE_ARRAY_MAX) {
          if (!icaltime_is_null_time(r.until)) {
            recur->setYearly(Recurrence::rYearlyPos,r.interval,
                              readICalDate(r.until));
          } else {
            if (r.count == 0)
              recur->setYearly(Recurrence::rYearlyPos,r.interval,-1);
            else
              recur->setYearly(Recurrence::rYearlyPos,r.interval,r.count);
          }
          bool useSetPos = false;
          short pos = 0;
          while((day = r.by_day[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
//            kdDebug(5800) << "----a " << index << ": " << day << endl;
            pos = icalrecurrencetype_day_position(day);
            if (pos) {
              day = icalrecurrencetype_day_day_of_week(day);
              QBitArray ba(7);          // don't wipe qba
              ba.setBit((day+5)%7);     // convert from Sunday=1 to Monday=0
              recur->addYearlyMonthPos(pos,ba);
            } else {
              qba.setBit((day+5)%7);    // convert from Sunday=1 to Monday=0
              useSetPos = true;
            }
          }
          if (useSetPos) {
            if (r.by_set_pos[0] != ICAL_RECURRENCE_ARRAY_MAX) {
              recur->addYearlyMonthPos(r.by_set_pos[0],qba);
            }
          }
        } else {
          if (!icaltime_is_null_time(r.until)) {
            recur->setYearly(Recurrence::rYearlyMonth,r.interval,
                              readICalDate(r.until));
          } else {
            if (r.count == 0)
              recur->setYearly(Recurrence::rYearlyMonth,r.interval,-1);
            else
              recur->setYearly(Recurrence::rYearlyMonth,r.interval,r.count);
          }
          while((day = r.by_month_day[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
            recur->addMonthlyDay(day);
          }
        }
        index = 0;
        while((day = r.by_month[index++]) != ICAL_RECURRENCE_ARRAY_MAX) {
          recur->addYearlyNum(day);
        }
      }
      break;
    default:
      kdDebug(5800) << "Unknown type of recurrence: " << r.freq << endl;
      break;
  }
}

void ICalFormatImpl::readAlarm(icalcomponent *alarm,Incidence *incidence)
{
  //kdDebug(5800) << "Read alarm for " << incidence->summary() << endl;

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

icaltimetype ICalFormatImpl::writeICalDate(const QDate &date)
{
  icaltimetype t;

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
  icaltimetype t;

  t.year = datetime.date().year();
  t.month = datetime.date().month();
  t.day = datetime.date().day();

  t.hour = datetime.time().hour();
  t.minute = datetime.time().minute();
  t.second = datetime.time().second();

  t.is_date = 0;
  t.zone = 0;
  t.is_utc = 0;

  if ( mParent->utc() ) {
    if (mParent->timeZoneId().isEmpty())
      t = icaltime_as_utc(t, 0);
    else
      t = icaltime_as_utc(t,mParent->timeZoneId().utf8());
  }

  return t;
}

QDateTime ICalFormatImpl::readICalDateTime(icaltimetype t)
{
/*
  kdDebug(5800) << "ICalFormatImpl::readICalDateTime()" << endl;
  kdDebug(5800) << "--- Y: " << t.year << " M: " << t.month << " D: " << t.day
            << endl;
  kdDebug(5800) << "--- H: " << t.hour << " M: " << t.minute << " S: " << t.second
            << endl;
  kdDebug(5800) << "--- isDate: " << t.is_date << endl;
  kdDebug(5800) << "--- isUtc: " << t.is_utc << endl;
  kdDebug(5800) << "--- zoneId: " << t.zone << endl;
*/

  // First convert the time into UTC if required.
  if ( !t.is_utc && t.zone ) {
    Timezone *timezone;

    // Always lookup with quotes.
    if (t.zone[0] != '"') {
      timezone = mTimezones.find(QString("\"") + t.zone + '"');
    } else {
      timezone = mTimezones.find(t.zone);
    }
    if (timezone) {
      // Apply the offset, and mark the structure as UTC!
      t.second -= timezone->offset(t);
      t = icaltime_normalize(t);
      t.is_utc = 1;
    } else {
      kdError(5800) << "ICalFormatImpl::readICalDateTime() cannot find timezone "
            << t.zone << endl;
    }
  }

  if ( t.is_utc && mCompat && mCompat->useTimeZoneShift() ) {
//    kdDebug(5800) << "--- Converting time to zone '" << cal->timeZoneId() << "'." << endl;
    if (mParent->timeZoneId().isEmpty())
      t = icaltime_as_zone(t, 0);
    else
      t = icaltime_as_zone(t,mParent->timeZoneId().utf8());
  }
  QDateTime result(QDate(t.year,t.month,t.day),
                   QTime(t.hour,t.minute,t.second));

  return result;
}

QDate ICalFormatImpl::readICalDate(icaltimetype t)
{
  return QDate(t.year,t.month,t.day);
}

icaldurationtype ICalFormatImpl::writeICalDuration(int seconds)
{
  icaldurationtype d;

  d.weeks    = seconds   % gSecondsPerWeek;
  seconds   -= d.weeks   * gSecondsPerWeek;
  d.days     = seconds   % gSecondsPerDay;
  seconds   -= d.days    * gSecondsPerDay;
  d.hours    = seconds   % gSecondsPerHour;
  seconds   -= d.hours   * gSecondsPerHour;
  d.minutes  = seconds   % gSecondsPerMinute;
  seconds   -= d.minutes * gSecondsPerMinute;
  d.seconds  = seconds;
  d.is_neg = 0;

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
#if 0
  if ((curVO = isAPropertyOf(vcal, ICMethodProp)) != 0) {
    char *methodType = 0;
    methodType = fakeCString(vObjectUStringZValue(curVO));
    if (mEnableDialogs)
      KMessageBox::information(mTopWidget,
                               i18n("This calendar is an iTIP transaction of type \"%1\".")
                               .arg(methodType),
                               i18n("%1: iTIP Transaction").arg(CalFormat::application()));
    delete methodType;
  }
#endif

  icalproperty *p;

  p = icalcomponent_get_first_property(calendar,ICAL_PRODID_PROPERTY);
  if (!p) {
    kdDebug(5800) << "No PRODID property found" << endl;
// TODO: does no PRODID really matter?
//    mParent->setException(new ErrorFormat(ErrorFormat::CalVersionUnknown));
//    return false;
    mLoadedProductId = "";
    mCalendarVersion = 0;
  } else {
    mLoadedProductId = QString::fromUtf8(icalproperty_get_prodid(p));
    mCalendarVersion = CalFormat::calendarVersion(mLoadedProductId.latin1());
//    kdDebug(5800) << "VCALENDAR prodid: '" << mLoadedProductId << "'" << endl;

    delete mCompat;
    mCompat = CompatFactory::createCompat( mLoadedProductId );
  }

// TODO: check for unknown PRODID
#if 0
  if (!mCalendarVersion
  &&  CalFormat::productId() != mLoadedProductId) {
    // warn the user that we might have trouble reading non-known calendar.
    if (mEnableDialogs)
      KMessageBox::information(mTopWidget,
                             i18n("This vCalendar file was not created by KOrganizer "
                                     "or any other product we support. Loading anyway..."),
                             i18n("%1: Unknown vCalendar Vendor").arg(CalFormat::application()));
  }
#endif

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


// TODO: check for calendar format version
#if 0
  // warn the user we might have trouble reading this unknown version.
  if ((curVO = isAPropertyOf(vcal, VCVersionProp)) != 0) {
    char *s = fakeCString(vObjectUStringZValue(curVO));
    if (strcmp(_VCAL_VERSION, s) != 0)
      if (mEnableDialogs)
        KMessageBox::sorry(mTopWidget,
                             i18n("This vCalendar file has version %1.\n"
                                  "We only support %2.")
                             .arg(s).arg(_VCAL_VERSION),
                             i18n("%1: Unknown vCalendar Version").arg(CalFormat::application()));
    deleteStr(s);
  }
#endif

  // custom properties
  readCustomProperties(calendar, cal);

// TODO: set time zone
#if 0
  // set the time zone
  if ((curVO = isAPropertyOf(vcal, VCTimeZoneProp)) != 0) {
    char *s = fakeCString(vObjectUStringZValue(curVO));
    cal->setTimeZone(s);
    deleteStr(s);
  }
#endif

  // Store all events with a relatedTo property in a list for post-processing
  mEventsRelate.clear();
  mTodosRelate.clear();
  // TODO: make sure that only actually added ecvens go to this lists.

  icalcomponent *c;

  // Iterate through all timezones before we do anything else. That way, the
  // information needed to interpret times in actually useful objects is
  // available below.
  c = icalcomponent_get_first_component(calendar,ICAL_VTIMEZONE_COMPONENT);
  while (c) {
//    kdDebug(5800) << "----Timezone found" << endl;
    readTimezone(c);
    c = icalcomponent_get_next_component(calendar,ICAL_VTIMEZONE_COMPONENT);
  }

  // Iterate through all todos
  c = icalcomponent_get_first_component(calendar,ICAL_VTODO_COMPONENT);
  while (c) {
//    kdDebug(5800) << "----Todo found" << endl;
    Todo *todo = readTodo(c);
    if (todo && !cal->todo(todo->uid())) cal->addTodo(todo);
    c = icalcomponent_get_next_component(calendar,ICAL_VTODO_COMPONENT);
  }

  // Iterate through all events
  c = icalcomponent_get_first_component(calendar,ICAL_VEVENT_COMPONENT);
  while (c) {
//    kdDebug(5800) << "----Event found" << endl;
    Event *event = readEvent(c);
    if (event && !cal->event(event->uid())) cal->addEvent(event);
    c = icalcomponent_get_next_component(calendar,ICAL_VEVENT_COMPONENT);
  }

  // Iterate through all journals
  c = icalcomponent_get_first_component(calendar,ICAL_VJOURNAL_COMPONENT);
  while (c) {
//    kdDebug(5800) << "----Journal found" << endl;
    Journal *journal = readJournal(c);
    if (journal && !cal->journal(journal->uid())) cal->addJournal(journal);
    c = icalcomponent_get_next_component(calendar,ICAL_VJOURNAL_COMPONENT);
  }

#if 0
  initPropIterator(&i, vcal);

  // go through all the vobjects in the vcal
  while (moreIteration(&i)) {
    curVO = nextVObject(&i);

    /************************************************************************/

    // now, check to see that the object is an event or todo.
    if (strcmp(vObjectName(curVO), VCEventProp) == 0) {

      if ((curVOProp = isAPropertyOf(curVO, KPilotStatusProp)) != 0) {
        char *s;
        s = fakeCString(vObjectUStringZValue(curVOProp));
        // check to see if event was deleted by the kpilot conduit
        if (atoi(s) == Event::SYNCDEL) {
          deleteStr(s);
          kdDebug(5800) << "skipping pilot-deleted event" << endl;
          goto SKIP;
        }
        deleteStr(s);
      }

      // this code checks to see if we are trying to read in an event
      // that we already find to be in the calendar.  If we find this
      // to be the case, we skip the event.
      if ((curVOProp = isAPropertyOf(curVO, VCUniqueStringProp)) != 0) {
        char *s = fakeCString(vObjectUStringZValue(curVOProp));
        QString tmpStr(s);
        deleteStr(s);

        if (cal->event(tmpStr)) {
          goto SKIP;
        }
        if (cal->todo(tmpStr)) {
          goto SKIP;
        }
      }

      if ((!(curVOProp = isAPropertyOf(curVO, VCDTstartProp))) &&
          (!(curVOProp = isAPropertyOf(curVO, VCDTendProp)))) {
        kdDebug(5800) << "found a VEvent with no DTSTART and no DTEND! Skipping..." << endl;
        goto SKIP;
      }

      anEvent = VEventToEvent(curVO);
      // we now use addEvent instead of insertEvent so that the
      // signal/slot get connected.
      if (anEvent)
        cal->addEvent(anEvent);
      else {
        // some sort of error must have occurred while in translation.
        goto SKIP;
      }
    } else if (strcmp(vObjectName(curVO), VCTodoProp) == 0) {
      anEvent = VTodoToEvent(curVO);
      cal->addTodo(anEvent);
    } else if ((strcmp(vObjectName(curVO), VCVersionProp) == 0) ||
               (strcmp(vObjectName(curVO), VCProdIdProp) == 0) ||
               (strcmp(vObjectName(curVO), VCTimeZoneProp) == 0)) {
      // do nothing, we know these properties and we want to skip them.
      // we have either already processed them or are ignoring them.
      ;
    } else {
      kdDebug(5800) << "Ignoring unknown vObject \"" << vObjectName(curVO) << "\"" << endl;
    }
  SKIP:
    ;
  } // while
#endif

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
  kdDebug(5800) << " Until: " << icaltime_as_ctime(r.until) << endl;
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

// This function reads any TZID setting for an icaltime. TBD: incorporate
// this into icalproperty_get_datetime() so it is picked up everywhere as
// needed?
void ICalFormatImpl::readTzidParameter( icalcomponent *p,
                                        icaltimetype &icaltime )
{
  icalproperty *tzp = icalproperty_get_first_parameter( p,
                                                        ICAL_TZID_PARAMETER );
  if ( tzp ) {
    icaltime.zone = icalparameter_get_tzid( tzp );
  }
}

