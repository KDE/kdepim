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

const int gSecondsPerMinute = 60;
const int gSecondsPerHour   = gSecondsPerMinute * 60;
const int gSecondsPerDay    = gSecondsPerHour   * 24;
const int gSecondsPerWeek   = gSecondsPerDay    * 7;

ICalFormatImpl::ICalFormatImpl( ICalFormat *parent ) :
  mParent( parent ), mCalendarVersion( 0 )
{
  mCompat = new Compat;
}

ICalFormatImpl::~ICalFormatImpl()
{
  delete mCompat;
}

class ToStringVisitor : public Incidence::Visitor
{
  public:
    ToStringVisitor( ICalFormatImpl *impl ) : mImpl( impl ), mComponent( 0 ) {}

    bool visit( Event *e ) { mComponent = mImpl->writeEvent( e ); return true; }
    bool visit( Todo *e ) { mComponent = mImpl->writeTodo( e ); return true; }
    bool visit( Journal *e ) { mComponent = mImpl->writeJournal( e ); return true; }

    icalcomponent *component() { return mComponent; }

  private:
    ICalFormatImpl *mImpl;
    icalcomponent *mComponent;
};

icalcomponent *ICalFormatImpl::writeIncidence(Incidence *incidence)
{
  ToStringVisitor v( this );
  incidence->accept(v);
  return v.component();
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
      due = writeICalDate(todo->dtDue().date());
    } else {
      due = writeICalDateTime(todo->dtDue());
    }
    icalcomponent_add_property(vtodo,icalproperty_new_due(due));
  }

  // start time
  if (todo->hasStartDate()) {
    icaltimetype start;
    if (todo->doesFloat()) {
//      kdDebug(5800) << "§§ Incidence " << todo->summary() << " floats." << endl;
      start = writeICalDate(todo->dtStart().date());
    } else {
//      kdDebug(5800) << "§§ incidence " << todo->summary() << " has time." << endl;
      start = writeICalDateTime(todo->dtStart());
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

  return vtodo;
}

icalcomponent *ICalFormatImpl::writeEvent(Event *event)
{
  kdDebug(5800) << "Write Event '" << event->summary() << "' (" << event->uid()
              << ")" << endl;

  QString tmpStr;
  QStringList tmpStrList;

  icalcomponent *vevent = icalcomponent_new(ICAL_VEVENT_COMPONENT);

  writeIncidence(vevent,event);

  // start time
  icaltimetype start;
  if (event->doesFloat()) {
//    kdDebug(5800) << "§§ Incidence " << event->summary() << " floats." << endl;
    start = writeICalDate(event->dtStart().date());
  } else {
//    kdDebug(5800) << "§§ incidence " << event->summary() << " has time." << endl;
    start = writeICalDateTime(event->dtStart());
  }
  icalcomponent_add_property(vevent,icalproperty_new_dtstart(start));

  // end time
  icaltimetype end;
  if (event->doesEndFloat()) {
//    kdDebug(5800) << "§§ Event " << event->summary() << " floats." << endl;
    // +1 day because end date is non-inclusive.
    end = writeICalDate( event->dtEnd().date().addDays( 1 ) );
  } else {
//    kdDebug(5800) << "§§ Event " << event->summary() << " has time." << endl;
    end = writeICalDateTime(event->dtEnd());
  }
  icalcomponent_add_property(vevent,icalproperty_new_dtend(end));

// TODO: attachments, resources
#if 0
  // attachments
  tmpStrList = anEvent->attachments();
  for ( QStringList::Iterator it = tmpStrList.begin();
        it != tmpStrList.end();
        ++it )
    addPropValue(vevent, VCAttachProp, (*it).utf8());

  // resources
  tmpStrList = anEvent->resources();
  tmpStr = tmpStrList.join(";");
  if (!tmpStr.isEmpty())
    addPropValue(vevent, VCResourcesProp, tmpStr.utf8());

#endif

// TODO: transparency
  // transparency
//  tmpStr.sprintf("%i",anEvent->getTransparency());
//  addPropValue(vevent, VCTranspProp, tmpStr.utf8());

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
    period.end = writeICalDateTime((*it).end());
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
//      kdDebug(5800) << "§§ Incidence " << event->summary() << " floats." << endl;
      start = writeICalDate(journal->dtStart().date());
    } else {
//      kdDebug(5800) << "§§ incidence " << event->summary() << " has time." << endl;
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

// TODO:
  // status
//  addPropValue(parent, VCStatusProp, incidence->getStatusStr().utf8());

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
  Recurrence *recur = incidence->recurrence();
  if (recur->doesRecur()) {
    kdDebug(5800) << "Write recurrence for '" << incidence->summary() << "' (" << incidence->uid()
              << ")" << endl;
    icalcomponent_add_property(parent,writeRecurrenceRule(recur));
  }

  // recurrence excpetion dates
  DateList dateList = incidence->exDates();
  DateList::ConstIterator exIt;
  for(exIt = dateList.begin(); exIt != dateList.end(); ++exIt) {
    icalcomponent_add_property(parent,icalproperty_new_exdate(
        writeICalDate(*exIt)));
  }
  
  // attachments
  QPtrList<Attachment> attachments = incidence->attachments();
  for (Attachment *at = attachments.first(); at; at = attachments.next())
    icalcomponent_add_property(parent,writeAttachment(at));

  // alarms
  QPtrList<Alarm> alarms = incidence->alarms();
  Alarm* alarm;
  for (alarm = alarms.first(); alarm; alarm = alarms.next()) {
    if (alarm->enabled()) {
      kdDebug(5800) << "Write alarm for " << incidence->summary() << endl;
      icalcomponent_add_component(parent,writeAlarm(alarm));
    }
  }

  // duration

// turned off as it always is set to PTS0 (and must not occur together with DTEND

//  if (incidence->hasDuration()) {
//    icaldurationtype duration;
//    duration = writeICalDuration(incidence->duration());
//    icalcomponent_add_property(parent,icalproperty_new_duration(duration));
//  }
}

void ICalFormatImpl::writeIncidenceBase(icalcomponent *parent,IncidenceBase *incidenceBase)
{
  icalcomponent_add_property(parent,icalproperty_new_dtstamp(
      writeICalDateTime(QDateTime::currentDateTime())));

  // organizer stuff
  icalcomponent_add_property(parent,icalproperty_new_organizer(
      ("MAILTO:" + incidenceBase->organizer()).utf8()));

  // attendees
  if (incidenceBase->attendeeCount() != 0) {
    QPtrList<Attendee> al = incidenceBase->attendees();
    QPtrListIterator<Attendee> ai(al);
    for (; ai.current(); ++ai) {
      icalcomponent_add_property(parent,writeAttendee(ai.current()));
    }
  }

  // custom properties
  writeCustomProperties(parent, incidenceBase);
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
  icalattachtype* attach = icalattachtype_new();
  if (att->isURI())
    icalattachtype_set_url(attach, att->uri().utf8().data());
  else
    icalattachtype_set_base64(attach, att->data(), 0);

  icalproperty *p = icalproperty_new_attach(attach);

  if (!att->mimeType().isEmpty())
    icalproperty_add_parameter(p,icalparameter_new_fmttype(att->mimeType().utf8().data()));

  if (att->isBinary()) {
    icalproperty_add_parameter(p,icalparameter_new_value(ICAL_VALUE_BINARY));
    icalproperty_add_parameter(p,icalparameter_new_encoding(ICAL_ENCODING_BASE64));
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
        r.by_month_day[index++] = icalrecurrencetype_day_position(*tmpDay*8);//*tmpDay);
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
      icalcomponent_add_property(a,icalproperty_new_description(alarm->text().utf8()));
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
        if (icaltime.is_date) {
          todo->setDtDue(QDateTime(readICalDate(icaltime),QTime(0,0,0)));
          todo->setFloats(true);

        } else {
          todo->setDtDue(readICalDateTime(icaltime));
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

      case ICAL_DTSTART_PROPERTY:
        // Flag that todo has start date. Value is read in by readIncidence().
        todo->setHasStartDate(true);
        break;

      default:
//        kdDebug(5800) << "ICALFormat::readTodo(): Unknown property: " << kind
//                  << endl;
        break;
    }

    p = icalcomponent_get_next_property(vtodo,ICAL_ANY_PROPERTY);
  }

  return todo;
}

Event *ICalFormatImpl::readEvent(icalcomponent *vevent)
{
  Event *event = new Event;
  event->setFloats(false);

  readIncidence(vevent,event);

  icalproperty *p = icalcomponent_get_first_property(vevent,ICAL_ANY_PROPERTY);

//  int intvalue;
  icaltimetype icaltime;

  QStringList categories;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_DTEND_PROPERTY:  // start date and time
        icaltime = icalproperty_get_dtend(p);
        if (icaltime.is_date) {
          event->setFloats( true );
          // End date is non-inclusive
          QDate endDate = readICalDate( icaltime ).addDays( -1 );
          mCompat->fixFloatingEnd( endDate );
          if ( endDate < event->dtStart().date() ) {
            endDate = event->dtStart().date();
          }
          event->setDtEnd( QDateTime( endDate, QTime( 0, 0, 0 ) ) );
        } else {
          event->setDtEnd(readICalDateTime(icaltime));
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

// TODO: exdates
#if 0
  // recurrence exceptions
  if ((vo = isAPropertyOf(vevent, VCExDateProp)) != 0) {
    anEvent->setExDates(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }
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

// TODO: transparency
#if 0
  // transparency
  if ((vo = isAPropertyOf(vevent, VCTranspProp)) != 0) {
    anEvent->setTransparency(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
#endif

      case ICAL_RELATEDTO_PROPERTY:  // releated event (parent)
        event->setRelatedToUid(QString::fromUtf8(icalproperty_get_relatedto(p)));
        mEventsRelate.append(event);
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

  // some stupid vCal exporters ignore the standard and use Description
  // instead of Summary for the default field.  Correct for this.
  if (event->summary().isEmpty() &&
      !(event->description().isEmpty())) {
    QString tmpStr = event->description().simplifyWhiteSpace();
    event->setDescription("");
    event->setSummary(tmpStr);
  }

  return event;
}

FreeBusy *ICalFormatImpl::readFreeBusy(icalcomponent *vfreebusy)
{
  FreeBusy *freebusy = new FreeBusy;

  readIncidenceBase(vfreebusy,freebusy);

  icalproperty *p = icalcomponent_get_first_property(vfreebusy,ICAL_ANY_PROPERTY);

  icaltimetype icaltime;
  icalperiodtype icalperiod;
  QDateTime period_start, period_end;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_DTSTART_PROPERTY:  // start date and time
        icaltime = icalproperty_get_dtstart(p);
        freebusy->setDtStart(readICalDateTime(icaltime));
        break;

      case ICAL_DTEND_PROPERTY:  // start End Date and Time
        icaltime = icalproperty_get_dtend(p);
        freebusy->setDtEnd(readICalDateTime(icaltime));
        break;

      case ICAL_FREEBUSY_PROPERTY:  //Any FreeBusy Times
        icalperiod = icalproperty_get_freebusy(p);
        period_start = readICalDateTime(icalperiod.start);
        period_end = readICalDateTime(icalperiod.end);
        freebusy->addPeriod(period_start, period_end);
        break;

      default:
        kdDebug(5800) << "ICALFormat::readIncidence(): Unknown property: " << kind
                  << endl;
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

  if (v == ICAL_VALUE_BINARY && e == ICAL_ENCODING_BASE64)
    attachment = new Attachment(icalattachtype_get_base64(a));
  else if ((v == ICAL_VALUE_NONE || v == ICAL_VALUE_URI) && (e == ICAL_ENCODING_NONE || e == ICAL_ENCODING_8BIT)) {
    attachment = new Attachment(QString(icalattachtype_get_url(a)));
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
        incidence->setCreated(readICalDateTime(icaltime));
        break;

      case ICAL_SEQUENCE_PROPERTY:  // sequence
        intvalue = icalproperty_get_sequence(p);
        incidence->setRevision(intvalue);
        break;

      case ICAL_LASTMODIFIED_PROPERTY:  // last modification date
        icaltime = icalproperty_get_lastmodified(p);
        incidence->setLastModified(readICalDateTime(icaltime));
        break;

      case ICAL_DTSTART_PROPERTY:  // start date and time
        icaltime = icalproperty_get_dtstart(p);
        if (icaltime.is_date) {
          incidence->setDtStart(QDateTime(readICalDate(icaltime),QTime(0,0,0)));
          incidence->setFloats(true);
        } else {
          incidence->setDtStart(readICalDateTime(icaltime));
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

#if 0
  // status
  if ((vo = isAPropertyOf(vincidence, VCStatusProp)) != 0) {
    incidence->setStatus(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }
  else
    incidence->setStatus("NEEDS ACTION");
#endif

      case ICAL_PRIORITY_PROPERTY:  // priority
        intvalue = icalproperty_get_priority(p);
        incidence->setPriority(intvalue);
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
        incidence->addExDate(readICalDate(icaltime));
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

  // Cancel backwards compatibility mode for subsequent changes by the application
  incidence->recurrence()->setCompatVersion();

  // add categories
  incidence->setCategories(categories);

  // iterate through all alarms
  for (icalcomponent *alarm = icalcomponent_get_first_component(parent,ICAL_VALARM_COMPONENT);
       alarm;
       alarm = icalcomponent_get_next_component(parent,ICAL_VALARM_COMPONENT)) {
    readAlarm(alarm,incidence);
  }
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
        incidenceBase->setOrganizer(QString::fromUtf8(icalproperty_get_organizer(p)));
        break;

      case ICAL_ATTENDEE_PROPERTY:  // attendee
        incidenceBase->addAttendee(readAttendee(p));
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
  if ( !p ) {
    kdDebug(5800) << "Unknown type of alarm" << endl;
    return;
  }

  icalproperty_action action = icalproperty_get_action(p);
  Alarm::Type type = Alarm::Display;
  switch ( action ) {
    case ICAL_ACTION_DISPLAY:   type = Alarm::Display;  break;
    case ICAL_ACTION_AUDIO:     type = Alarm::Audio;  break;
    case ICAL_ACTION_PROCEDURE: type = Alarm::Procedure;  break;
    case ICAL_ACTION_EMAIL:     type = Alarm::Email;  break;
    default:
      kdDebug(5800) << "Unknown type of alarm" << endl;
      return;
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
        icalattachtype *attach = icalproperty_get_attach(p);
        QString url = QFile::decodeName(icalattachtype_get_url(attach));
        switch ( action ) {
          case ICAL_ACTION_AUDIO:
            ialarm->setAudioFile( url );
            break;
          case ICAL_ACTION_PROCEDURE:
            ialarm->setProgramFile( url );
            break;
          case ICAL_ACTION_EMAIL:
            ialarm->addMailAttachment( url );
            break;
          default:
            break;
        }
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
      t = icaltime_as_utc(t,mParent->timeZoneId().local8Bit());
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
*/

  if (t.is_utc) {
//    kdDebug(5800) << "--- Converting time to zone '" << cal->timeZoneId() << "'." << endl;
    if (mParent->timeZoneId().isEmpty())
      t = icaltime_as_zone(t, 0);
    else
      t = icaltime_as_zone(t,mParent->timeZoneId().local8Bit());
  }

  return QDateTime(QDate(t.year,t.month,t.day),
                   QTime(t.hour,t.minute,t.second));
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
    mCalendarVersion = CalFormat::calendarVersion(mLoadedProductId);
    kdDebug(5800) << "VCALENDAR prodid: '" << mLoadedProductId << "'" << endl;

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
    kdDebug(5800) << "VCALENDAR version: '" << version << "'" << endl;

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

  // Iterate through all todos
  c = icalcomponent_get_first_component(calendar,ICAL_VTODO_COMPONENT);
  while (c) {
//    kdDebug(5800) << "----Todo found" << endl;
    Todo *todo = readTodo(c);
    if (!cal->todo(todo->uid())) cal->addTodo(todo);
    c = icalcomponent_get_next_component(calendar,ICAL_VTODO_COMPONENT);
  }

  // Iterate through all events
  c = icalcomponent_get_first_component(calendar,ICAL_VEVENT_COMPONENT);
  while (c) {
//    kdDebug(5800) << "----Event found" << endl;
    Event *event = readEvent(c);
    if (!cal->event(event->uid())) cal->addEvent(event);
    c = icalcomponent_get_next_component(calendar,ICAL_VEVENT_COMPONENT);
  }

  // Iterate through all journals
  c = icalcomponent_get_first_component(calendar,ICAL_VJOURNAL_COMPONENT);
  while (c) {
//    kdDebug(5800) << "----Journal found" << endl;
    Journal *journal = readJournal(c);
    if (!cal->journal(journal->uid())) cal->addJournal(journal);
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
  Event *ev;
  for ( ev=mEventsRelate.first(); ev != 0; ev=mEventsRelate.next() ) {
    ev->setRelatedTo(cal->event(ev->relatedToUid()));
  }
  Todo *todo;
  for ( todo=mTodosRelate.first(); todo != 0; todo=mTodosRelate.next() ) {
    todo->setRelatedTo(cal->todo(todo->relatedToUid()));
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

  // TODO: check, if dynamic cast is required
  if(incidence->type() == "Todo") {
    Todo *todo = static_cast<Todo *>(incidence);
    icalcomponent_add_component(message,writeTodo(todo));
  }
  if(incidence->type() == "Event") {
    Event *event = static_cast<Event *>(incidence);
    icalcomponent_add_component(message,writeEvent(event));
  }
  if(incidence->type() == "FreeBusy") {
    FreeBusy *freebusy = static_cast<FreeBusy *>(incidence);
    icalcomponent_add_component(message,writeFreeBusy(freebusy, method));
  }

  return message;
}
