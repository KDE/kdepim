/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brwon
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

#include <qapplication.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qregexp.h>
#include <qclipboard.h>
#include <qdialog.h>
#include <qfile.h>

#include <kdebug.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <klocale.h>

#include "vcc.h"
#include "vobject.h"
extern "C" {
#include "icaltime.h"
}
#include "vcaldrag.h"
#include "calendar.h"

#include "vcalformat.h"

using namespace KCal;

VCalFormat::VCalFormat()
{
}

VCalFormat::~VCalFormat()
{
}

bool VCalFormat::load(Calendar *calendar, const QString &fileName)
{
  mCalendar = calendar;

  clearException();

  kdDebug(5800) << "VCalFormat::load() " << fileName << endl;

  VObject *vcal = 0;

  // this is not necessarily only 1 vcal.  Could be many vcals, or include
  // a vcard...
  vcal = Parse_MIME_FromFileName(const_cast<char *>(QFile::encodeName(fileName).data()));

  if (!vcal) {
    setException(new ErrorFormat(ErrorFormat::CalVersionUnknown));
    return FALSE;
  }

  // any other top-level calendar stuff should be added/initialized here

  // put all vobjects into their proper places
  populate(vcal);

  // clean up from vcal API stuff
  cleanVObjects(vcal);
  cleanStrTbl();

  return true;
}


bool VCalFormat::save(Calendar *calendar, const QString &fileName)
{
  mCalendar = calendar;

  QString tmpStr;
  VObject *vcal, *vo;

  kdDebug(5800) << "VCalFormat::save(): " << fileName << endl;

  vcal = newVObject(VCCalProp);

  //  addPropValue(vcal,VCLocationProp, "0.0");
  addPropValue(vcal,VCProdIdProp, productId().latin1());
  addPropValue(vcal,VCVersionProp, _VCAL_VERSION);

  // TODO STUFF
  Todo::List todoList = mCalendar->rawTodos();
  Todo::List::ConstIterator it;
  for ( it = todoList.begin(); it != todoList.end(); ++it ) {
    vo = eventToVTodo( *it );
    addVObjectProp( vcal, vo );
  }

  // EVENT STUFF
  Event::List events = mCalendar->rawEvents();
  Event::List::ConstIterator it2;
  for( it2 = events.begin(); it2 != events.end(); ++it2 ) {
    vo = eventToVEvent( *it2 );
    addVObjectProp( vcal, vo );
  }

  writeVObjectToFile(QFile::encodeName(fileName).data() ,vcal);
  cleanVObjects(vcal);
  cleanStrTbl();

  if (QFile::exists(fileName)) {
    kdDebug(5800) << "No error" << endl;
    return true;
  } else  {
    kdDebug(5800) << "Error" << endl;
    return false; // error
  }

  return false;
}

bool VCalFormat::fromString( Calendar *calendar, const QString &text )
{
  // TODO: Factor out VCalFormat::fromString()
  mCalendar = calendar;

  QCString data = text.utf8();

  if ( !data.size() ) return false;

  VObject *vcal = Parse_MIME( data.data(), data.size());
  if ( !vcal ) return false;

  VObjectIterator i;
  VObject *curvo;
  initPropIterator( &i, vcal );

  // we only take the first object. TODO: parse all incidences.
  do  {
    curvo = nextVObject( &i );
  } while ( strcmp( vObjectName( curvo ), VCEventProp ) &&
            strcmp( vObjectName( curvo ), VCTodoProp ) );

  if ( strcmp( vObjectName( curvo ), VCEventProp ) == 0 ) {
    Event *event = VEventToEvent( curvo );
    calendar->addEvent( event );
  } else {
    kdDebug(5800) << "VCalFormat::fromString(): Unknown object type." << endl;
    deleteVObject( vcal );
    return false;
  }

  deleteVObject( vcal );

  return true;
}

QString VCalFormat::toString( Calendar *calendar )
{
  // TODO: Factor out VCalFormat::asString()
  mCalendar = calendar;

  VObject *vcal = newVObject(VCCalProp);

  addPropValue( vcal, VCProdIdProp, CalFormat::productId().latin1() );
  addPropValue( vcal, VCVersionProp, _VCAL_VERSION );

  // TODO: Use all data.
  Event::List events = calendar->events();
  Event *event = events.first();
  if ( !event ) return QString::null;

  VObject *vevent = eventToVEvent( event );

  addVObjectProp( vcal, vevent );

  char *buf = writeMemVObject( 0, 0, vcal );

  QString result( buf );

  cleanVObject( vcal );

  return result;
}

VObject *VCalFormat::eventToVTodo(const Todo *anEvent)
{
  VObject *vtodo;
  QString tmpStr;
  QStringList tmpStrList;

  vtodo = newVObject(VCTodoProp);

  // due date
  if (anEvent->hasDueDate()) {
    tmpStr = qDateTimeToISO(anEvent->dtDue(),
                            !anEvent->doesFloat());
    addPropValue(vtodo, VCDueProp, tmpStr.local8Bit());
  }

  // start date
  if (anEvent->hasStartDate()) {
    tmpStr = qDateTimeToISO(anEvent->dtStart(),
                            !anEvent->doesFloat());
    addPropValue(vtodo, VCDTstartProp, tmpStr.local8Bit());
  }

  // creation date
  tmpStr = qDateTimeToISO(anEvent->created());
  addPropValue(vtodo, VCDCreatedProp, tmpStr.local8Bit());

  // unique id
  addPropValue(vtodo, VCUniqueStringProp,
               anEvent->uid().local8Bit());

  // revision
  tmpStr.sprintf("%i", anEvent->revision());
  addPropValue(vtodo, VCSequenceProp, tmpStr.local8Bit());

  // last modification date
  tmpStr = qDateTimeToISO(anEvent->lastModified());
  addPropValue(vtodo, VCLastModifiedProp, tmpStr.local8Bit());

  // organizer stuff
  // @TODO: How about the common name?
  tmpStr = "MAILTO:" + anEvent->organizer().email();
  addPropValue(vtodo, ICOrganizerProp, tmpStr.local8Bit());

  // attendees
  if ( anEvent->attendeeCount() > 0 ) {
    Attendee::List::ConstIterator it;
    Attendee *curAttendee;
    for ( it = anEvent->attendees().begin(); it != anEvent->attendees().end();
          ++it ) {
      curAttendee = *it;
      if (!curAttendee->email().isEmpty() &&
          !curAttendee->name().isEmpty())
        tmpStr = "MAILTO:" + curAttendee->name() + " <" +
                 curAttendee->email() + ">";
      else if (curAttendee->name().isEmpty())
        tmpStr = "MAILTO: " + curAttendee->email();
      else if (curAttendee->email().isEmpty())
        tmpStr = "MAILTO: " + curAttendee->name();
      else if (curAttendee->name().isEmpty() &&
               curAttendee->email().isEmpty())
        kdDebug(5800) << "warning! this Event has an attendee w/o name or email!" << endl;
      VObject *aProp = addPropValue(vtodo, VCAttendeeProp, tmpStr.local8Bit());
      addPropValue(aProp, VCRSVPProp, curAttendee->RSVP() ? "TRUE" : "FALSE");
      addPropValue(aProp, VCStatusProp, writeStatus(curAttendee->status()));
    }
  }

  // description BL:
  if (!anEvent->description().isEmpty()) {
    VObject *d = addPropValue(vtodo, VCDescriptionProp,
                              anEvent->description().local8Bit());
    if (anEvent->description().find('\n') != -1)
      addPropValue(d, VCEncodingProp, VCQuotedPrintableProp);
  }

  // summary
  if (!anEvent->summary().isEmpty())
    addPropValue(vtodo, VCSummaryProp, anEvent->summary().local8Bit());

  // location
  if (!anEvent->location().isEmpty())
    addPropValue(vtodo, VCLocationProp, anEvent->location().local8Bit());

  // completed
  // status
  // backward compatibility, KOrganizer used to interpret only these two values
  addPropValue(vtodo, VCStatusProp, anEvent->isCompleted() ? "COMPLETED" :
                                                             "NEEDS_ACTION");
  // completion date
  if (anEvent->hasCompletedDate()) {
    tmpStr = qDateTimeToISO(anEvent->completed());
    addPropValue(vtodo, VCCompletedProp, tmpStr.local8Bit());
  }

  // priority
  tmpStr.sprintf("%i",anEvent->priority());
  addPropValue(vtodo, VCPriorityProp, tmpStr.local8Bit());

  // related event
  if (anEvent->relatedTo()) {
    addPropValue(vtodo, VCRelatedToProp,
                 anEvent->relatedTo()->uid().local8Bit());
  }

  // categories
  tmpStrList = anEvent->categories();
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
    addPropValue(vtodo, VCCategoriesProp, tmpStr.local8Bit());
  }

  // alarm stuff
  kdDebug(5800) << "vcalformat::eventToVTodo was called" << endl;
  Alarm::List::ConstIterator it;
  for ( it = anEvent->alarms().begin(); it != anEvent->alarms().end(); ++it ) {
    Alarm *alarm = *it;
    if (alarm->enabled()) {
      VObject *a = addProp(vtodo, VCDAlarmProp);
      tmpStr = qDateTimeToISO(alarm->time());
      addPropValue(a, VCRunTimeProp, tmpStr.local8Bit());
      addPropValue(a, VCRepeatCountProp, "1");
      addPropValue(a, VCDisplayStringProp, "beep!");
      if (alarm->type() == Alarm::Audio) {
        a = addProp(vtodo, VCAAlarmProp);
        addPropValue(a, VCRunTimeProp, tmpStr.local8Bit());
        addPropValue(a, VCRepeatCountProp, "1");
        addPropValue(a, VCAudioContentProp, QFile::encodeName(alarm->audioFile()));
      }
      else if (alarm->type() == Alarm::Procedure) {
        a = addProp(vtodo, VCPAlarmProp);
        addPropValue(a, VCRunTimeProp, tmpStr.local8Bit());
        addPropValue(a, VCRepeatCountProp, "1");
        addPropValue(a, VCProcedureNameProp, QFile::encodeName(alarm->programFile()));
      }
    }
  }

  if (anEvent->pilotId()) {
    // pilot sync stuff
    tmpStr.sprintf("%lu",anEvent->pilotId());
    addPropValue(vtodo, KPilotIdProp, tmpStr.local8Bit());
    tmpStr.sprintf("%i",anEvent->syncStatus());
    addPropValue(vtodo, KPilotStatusProp, tmpStr.local8Bit());
  }

  return vtodo;
}

VObject* VCalFormat::eventToVEvent(const Event *anEvent)
{
  VObject *vevent;
  QString tmpStr;
  QStringList tmpStrList;

  vevent = newVObject(VCEventProp);

  // start and end time
  tmpStr = qDateTimeToISO(anEvent->dtStart(),
                          !anEvent->doesFloat());
  addPropValue(vevent, VCDTstartProp, tmpStr.local8Bit());

  // events that have time associated but take up no time should
  // not have both DTSTART and DTEND.
  if (anEvent->dtStart() != anEvent->dtEnd()) {
    tmpStr = qDateTimeToISO(anEvent->dtEnd(),
                            !anEvent->doesFloat());
    addPropValue(vevent, VCDTendProp, tmpStr.local8Bit());
  }

  // creation date
  tmpStr = qDateTimeToISO(anEvent->created());
  addPropValue(vevent, VCDCreatedProp, tmpStr.local8Bit());

  // unique id
  addPropValue(vevent, VCUniqueStringProp,
               anEvent->uid().local8Bit());

  // revision
  tmpStr.sprintf("%i", anEvent->revision());
  addPropValue(vevent, VCSequenceProp, tmpStr.local8Bit());

  // last modification date
  tmpStr = qDateTimeToISO(anEvent->lastModified());
  addPropValue(vevent, VCLastModifiedProp, tmpStr.local8Bit());

  // attendee and organizer stuff
  // TODO: What to do with the common name?
  tmpStr = "MAILTO:" + anEvent->organizer().email();
  addPropValue(vevent, ICOrganizerProp, tmpStr.local8Bit());

  // TODO: Put this functionality into Attendee class
  if ( anEvent->attendeeCount() > 0 ) {
    Attendee::List::ConstIterator it;
    for ( it = anEvent->attendees().begin(); it != anEvent->attendees().end();
          ++it ) {
      Attendee *curAttendee = *it;
      if (!curAttendee->email().isEmpty() &&
          !curAttendee->name().isEmpty())
        tmpStr = "MAILTO:" + curAttendee->name() + " <" +
                 curAttendee->email() + ">";
      else if (curAttendee->name().isEmpty())
        tmpStr = "MAILTO: " + curAttendee->email();
      else if (curAttendee->email().isEmpty())
        tmpStr = "MAILTO: " + curAttendee->name();
      else if (curAttendee->name().isEmpty() &&
               curAttendee->email().isEmpty())
        kdDebug(5800) << "warning! this Event has an attendee w/o name or email!" << endl;
      VObject *aProp = addPropValue(vevent, VCAttendeeProp, tmpStr.local8Bit());
      addPropValue(aProp, VCRSVPProp, curAttendee->RSVP() ? "TRUE" : "FALSE");
      addPropValue(aProp, VCStatusProp, writeStatus(curAttendee->status()));
    }
  }

  // recurrence rule stuff
  if (anEvent->recurrence()->doesRecur()) {
    // some more variables
    QPtrList<Recurrence::rMonthPos> tmpPositions;
    QPtrList<int> tmpDays;
    int *tmpDay;
    Recurrence::rMonthPos *tmpPos;
    QString tmpStr2;
    int i;

    switch(anEvent->recurrence()->doesRecur()) {
    case Recurrence::rDaily:
      tmpStr.sprintf("D%i ",anEvent->recurrence()->frequency());
//      if (anEvent->rDuration > 0)
//        tmpStr += "#";
      break;
    case Recurrence::rWeekly:
      tmpStr.sprintf("W%i ",anEvent->recurrence()->frequency());
      for (i = 0; i < 7; i++) {
        if (anEvent->recurrence()->days().testBit(i))
          tmpStr += dayFromNum(i);
      }
      break;
    case Recurrence::rMonthlyPos:
      tmpStr.sprintf("MP%i ", anEvent->recurrence()->frequency());
      // write out all rMonthPos's
      tmpPositions = anEvent->recurrence()->monthPositions();
      for (tmpPos = tmpPositions.first();
           tmpPos;
           tmpPos = tmpPositions.next()) {

        tmpStr2.sprintf("%i", tmpPos->rPos);
        if (tmpPos->negative)
          tmpStr2 += "- ";
        else
          tmpStr2 += "+ ";
        tmpStr += tmpStr2;
        for (i = 0; i < 7; i++) {
          if (tmpPos->rDays.testBit(i))
            tmpStr += dayFromNum(i);
        }
      } // loop for all rMonthPos's
      break;
    case Recurrence::rMonthlyDay:
      tmpStr.sprintf("MD%i ", anEvent->recurrence()->frequency());
      // write out all rMonthDays;
      tmpDays = anEvent->recurrence()->monthDays();
      for (tmpDay = tmpDays.first();
           tmpDay;
           tmpDay = tmpDays.next()) {
        tmpStr2.sprintf("%i ", *tmpDay);
        tmpStr += tmpStr2;
      }
      break;
    case Recurrence::rYearlyMonth:
      tmpStr.sprintf("YM%i ", anEvent->recurrence()->frequency());
      // write out all the rYearNums;
      tmpDays = anEvent->recurrence()->yearNums();
      for (tmpDay = tmpDays.first();
           tmpDay;
           tmpDay = tmpDays.next()) {
        tmpStr2.sprintf("%i ", *tmpDay);
        tmpStr += tmpStr2;
      }
      break;
    case Recurrence::rYearlyDay:
      tmpStr.sprintf("YD%i ", anEvent->recurrence()->frequency());
      // write out all the rYearNums;
      tmpDays = anEvent->recurrence()->yearNums();
      for (tmpDay = tmpDays.first();
           tmpDay;
           tmpDay = tmpDays.next()) {
        tmpStr2.sprintf("%i ", *tmpDay);
        tmpStr += tmpStr2;
      }
      break;
    default:
      kdDebug(5800) << "ERROR, it should never get here in eventToVEvent!" << endl;
      break;
    } // switch

    if (anEvent->recurrence()->duration() > 0) {
      tmpStr2.sprintf("#%i",anEvent->recurrence()->duration());
      tmpStr += tmpStr2;
    } else if (anEvent->recurrence()->duration() == -1) {
      tmpStr += "#0"; // defined as repeat forever
    } else {
      tmpStr += qDateTimeToISO(anEvent->recurrence()->endDate(), FALSE);
    }
    addPropValue(vevent,VCRRuleProp, tmpStr.local8Bit());

  } // event repeats

  // exceptions to recurrence
  DateList dateList = anEvent->exDates();
  DateList::ConstIterator it;
  QString tmpStr2;

  for (it = dateList.begin(); it != dateList.end(); ++it) {
    tmpStr = qDateToISO(*it) + ";";
    tmpStr2 += tmpStr;
  }
  if (!tmpStr2.isEmpty()) {
    tmpStr2.truncate(tmpStr2.length()-1);
    addPropValue(vevent, VCExDateProp, tmpStr2.local8Bit());
  }

  // description
  if (!anEvent->description().isEmpty()) {
    VObject *d = addPropValue(vevent, VCDescriptionProp,
                              anEvent->description().local8Bit());
    if (anEvent->description().find('\n') != -1)
      addPropValue(d, VCEncodingProp, VCQuotedPrintableProp);
  }

  // summary
  if (!anEvent->summary().isEmpty())
    addPropValue(vevent, VCSummaryProp, anEvent->summary().local8Bit());

  // location
  if (!anEvent->location().isEmpty())
    addPropValue(vevent, VCLocationProp, anEvent->location().local8Bit());

  // status
// TODO: define Event status
//  addPropValue(vevent, VCStatusProp, anEvent->statusStr().local8Bit());

  // secrecy
  const char *text = 0;
  switch (anEvent->secrecy()) {
    case Incidence::SecrecyPublic:
      text = "PUBLIC";
      break;
    case Incidence::SecrecyPrivate:
      text = "PRIVATE";
      break;
    case Incidence::SecrecyConfidential:
      text = "CONFIDENTIAL";
      break;
  }
  if (text) {
    addPropValue(vevent, VCClassProp, text);
  }

  // categories
  tmpStrList = anEvent->categories();
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
    addPropValue(vevent, VCCategoriesProp, tmpStr.local8Bit());
  }

  // attachments
  // TODO: handle binary attachments!
  Attachment::List attachments = anEvent->attachments();
  Attachment::List::ConstIterator atIt;
  for ( atIt = attachments.begin(); atIt != attachments.end(); ++atIt )
    addPropValue( vevent, VCAttachProp, (*atIt)->uri().local8Bit() );

  // resources
  tmpStrList = anEvent->resources();
  tmpStr = tmpStrList.join(";");
  if (!tmpStr.isEmpty())
    addPropValue(vevent, VCResourcesProp, tmpStr.local8Bit());

  // alarm stuff
  Alarm::List::ConstIterator it2;
  for ( it2 = anEvent->alarms().begin(); it2 != anEvent->alarms().end(); ++it2 ) {
    Alarm *alarm = *it2;
    if (alarm->enabled()) {
      VObject *a = addProp(vevent, VCDAlarmProp);
      tmpStr = qDateTimeToISO(alarm->time());
      addPropValue(a, VCRunTimeProp, tmpStr.local8Bit());
      addPropValue(a, VCRepeatCountProp, "1");
      addPropValue(a, VCDisplayStringProp, "beep!");
      if (alarm->type() == Alarm::Audio) {
        a = addProp(vevent, VCAAlarmProp);
        addPropValue(a, VCRunTimeProp, tmpStr.local8Bit());
        addPropValue(a, VCRepeatCountProp, "1");
        addPropValue(a, VCAudioContentProp, QFile::encodeName(alarm->audioFile()));
      }
      if (alarm->type() == Alarm::Procedure) {
        a = addProp(vevent, VCPAlarmProp);
        addPropValue(a, VCRunTimeProp, tmpStr.local8Bit());
        addPropValue(a, VCRepeatCountProp, "1");
        addPropValue(a, VCProcedureNameProp, QFile::encodeName(alarm->programFile()));
      }
    }
  }

  // priority
  tmpStr.sprintf("%i",anEvent->priority());
  addPropValue(vevent, VCPriorityProp, tmpStr.local8Bit());

  // transparency
  tmpStr.sprintf("%i",anEvent->transparency());
  addPropValue(vevent, VCTranspProp, tmpStr.local8Bit());

  // related event
  if (anEvent->relatedTo()) {
    addPropValue(vevent, VCRelatedToProp,
                 anEvent->relatedTo()->uid().local8Bit());
  }

  if (anEvent->pilotId()) {
    // pilot sync stuff
    tmpStr.sprintf("%lu",anEvent->pilotId());
    addPropValue(vevent, KPilotIdProp, tmpStr.local8Bit());
    tmpStr.sprintf("%i",anEvent->syncStatus());
    addPropValue(vevent, KPilotStatusProp, tmpStr.local8Bit());
  }

  return vevent;
}

Todo *VCalFormat::VTodoToEvent(VObject *vtodo)
{
  VObject *vo;
  VObjectIterator voi;
  char *s;

  Todo *anEvent = new Todo;

  // creation date
  if ((vo = isAPropertyOf(vtodo, VCDCreatedProp)) != 0) {
      anEvent->setCreated(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
      deleteStr(s);
  }

  // unique id
  vo = isAPropertyOf(vtodo, VCUniqueStringProp);
  // while the UID property is preferred, it is not required.  We'll use the
  // default Event UID if none is given.
  if (vo) {
    anEvent->setUid(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }

  // last modification date
  if ((vo = isAPropertyOf(vtodo, VCLastModifiedProp)) != 0) {
    anEvent->setLastModified(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setLastModified(QDateTime(QDate::currentDate(),
                                       QTime::currentTime()));

  // organizer
  // if our extension property for the event's ORGANIZER exists, add it.
  if ((vo = isAPropertyOf(vtodo, ICOrganizerProp)) != 0) {
    anEvent->setOrganizer( s = fakeCString(vObjectUStringZValue(vo) ) );
    deleteStr(s);
  } else {
    anEvent->setOrganizer( mCalendar->getOwner() );
  }

  // attendees.
  initPropIterator(&voi, vtodo);
  while (moreIteration(&voi)) {
    vo = nextVObject(&voi);
    if (strcmp(vObjectName(vo), VCAttendeeProp) == 0) {
      Attendee *a;
      VObject *vp;
      s = fakeCString(vObjectUStringZValue(vo));
      QString tmpStr = QString::fromLocal8Bit(s);
      deleteStr(s);
      tmpStr = tmpStr.simplifyWhiteSpace();
      int emailPos1, emailPos2;
      if ((emailPos1 = tmpStr.find('<')) > 0) {
        // both email address and name
        emailPos2 = tmpStr.findRev('>');
        a = new Attendee(tmpStr.left(emailPos1 - 1),
                         tmpStr.mid(emailPos1 + 1,
                                    emailPos2 - (emailPos1 + 1)));
      } else if (tmpStr.find('@') > 0) {
        // just an email address
        a = new Attendee(0, tmpStr);
      } else {
        // just a name
        QString email = tmpStr.replace( QRegExp(" "), "." );
        a = new Attendee(tmpStr,email);
      }

      // is there an RSVP property?
      if ((vp = isAPropertyOf(vo, VCRSVPProp)) != 0)
        a->setRSVP(vObjectStringZValue(vp));
      // is there a status property?
      if ((vp = isAPropertyOf(vo, VCStatusProp)) != 0)
        a->setStatus(readStatus(vObjectStringZValue(vp)));
      // add the attendee
      anEvent->addAttendee(a);
    }
  }

  // description for todo
  if ((vo = isAPropertyOf(vtodo, VCDescriptionProp)) != 0) {
    s = fakeCString(vObjectUStringZValue(vo));
    anEvent->setDescription(QString::fromLocal8Bit(s));
    deleteStr(s);
  }

  // summary
  if ((vo = isAPropertyOf(vtodo, VCSummaryProp))) {
    s = fakeCString(vObjectUStringZValue(vo));
    anEvent->setSummary(QString::fromLocal8Bit(s));
    deleteStr(s);
  }


  // location
  if ((vo = isAPropertyOf(vtodo, VCLocationProp)) != 0) {
    s = fakeCString(vObjectUStringZValue(vo));
    anEvent->setLocation( QString::fromLocal8Bit(s) );
    deleteStr(s);
  }
  // completed
  // was: status
  if ((vo = isAPropertyOf(vtodo, VCStatusProp)) != 0) {
    s = fakeCString(vObjectUStringZValue(vo));
    if (strcmp(s,"COMPLETED") == 0) {
      anEvent->setCompleted(true);
    } else {
      anEvent->setCompleted(false);
    }
    deleteStr(s);
  }
  else
    anEvent->setCompleted(false);

  // completion date
  if ((vo = isAPropertyOf(vtodo, VCCompletedProp)) != 0) {
    anEvent->setCompleted(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }

  // priority
  if ((vo = isAPropertyOf(vtodo, VCPriorityProp))) {
    anEvent->setPriority(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }

  // due date
  if ((vo = isAPropertyOf(vtodo, VCDueProp)) != 0) {
    anEvent->setDtDue(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
    anEvent->setHasDueDate(true);
  } else {
    anEvent->setHasDueDate(false);
  }

  // start time
  if ((vo = isAPropertyOf(vtodo, VCDTstartProp)) != 0) {
    anEvent->setDtStart(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
    //    kdDebug(5800) << "s is " << //          s << ", ISO is " << ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))).toString() << endl;
    deleteStr(s);
    anEvent->setHasStartDate(true);
  } else {
    anEvent->setHasStartDate(false);
  }

  /* alarm stuff */
  //kdDebug(5800) << "vcalformat::VTodoToEvent called" << endl;
  if ((vo = isAPropertyOf(vtodo, VCDAlarmProp))) {
    Alarm* alarm = anEvent->newAlarm();
    VObject *a;
    if ((a = isAPropertyOf(vo, VCRunTimeProp))) {
      alarm->setTime(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(a))));
      deleteStr(s);
    }
    alarm->setEnabled(true);
    if ((vo = isAPropertyOf(vtodo, VCPAlarmProp))) {
      if ((a = isAPropertyOf(vo, VCProcedureNameProp))) {
        s = fakeCString(vObjectUStringZValue(a));
        alarm->setProcedureAlarm(QFile::decodeName(s));
        deleteStr(s);
      }
    }
    if ((vo = isAPropertyOf(vtodo, VCAAlarmProp))) {
      if ((a = isAPropertyOf(vo, VCAudioContentProp))) {
        s = fakeCString(vObjectUStringZValue(a));
        alarm->setAudioAlarm(QFile::decodeName(s));
        deleteStr(s);
      }
    }
  }

  // related todo
  if ((vo = isAPropertyOf(vtodo, VCRelatedToProp)) != 0) {
    anEvent->setRelatedToUid(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
    mTodosRelate.append(anEvent);
  }

  // categories
  QStringList tmpStrList;
  int index1 = 0;
  int index2 = 0;
  if ((vo = isAPropertyOf(vtodo, VCCategoriesProp)) != 0) {
    s = fakeCString(vObjectUStringZValue(vo));
    QString categories = QString::fromLocal8Bit(s);
    deleteStr(s);
    //const char* category;
    QString category;
    while ((index2 = categories.find(',', index1)) != -1) {
        //category = (const char *) categories.mid(index1, (index2 - index1));
      category = categories.mid(index1, (index2 - index1));
      tmpStrList.append(category);
      index1 = index2+1;
    }
    // get last category
    category = categories.mid(index1, (categories.length()-index1));
    tmpStrList.append(category);
    anEvent->setCategories(tmpStrList);
  }

  /* PILOT SYNC STUFF */
  if ((vo = isAPropertyOf(vtodo, KPilotIdProp))) {
    anEvent->setPilotId(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setPilotId(0);

  if ((vo = isAPropertyOf(vtodo, KPilotStatusProp))) {
    anEvent->setSyncStatus(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setSyncStatus(Event::SYNCMOD);

  return anEvent;
}

Event* VCalFormat::VEventToEvent(VObject *vevent)
{
  VObject *vo;
  VObjectIterator voi;
  char *s;

  Event *anEvent = new Event;

  // creation date
  if ((vo = isAPropertyOf(vevent, VCDCreatedProp)) != 0) {
      anEvent->setCreated(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
      deleteStr(s);
  }

  // unique id
  vo = isAPropertyOf(vevent, VCUniqueStringProp);
  // while the UID property is preferred, it is not required.  We'll use the
  // default Event UID if none is given.
  if (vo) {
    anEvent->setUid(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }

  // revision
  // again NSCAL doesn't give us much to work with, so we improvise...
  if ((vo = isAPropertyOf(vevent, VCSequenceProp)) != 0) {
    anEvent->setRevision(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setRevision(0);

  // last modification date
  if ((vo = isAPropertyOf(vevent, VCLastModifiedProp)) != 0) {
    anEvent->setLastModified(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setLastModified(QDateTime(QDate::currentDate(),
                                       QTime::currentTime()));

  // organizer
  // if our extension property for the event's ORGANIZER exists, add it.
  if ((vo = isAPropertyOf(vevent, ICOrganizerProp)) != 0) {
    // FIXME:  Also use the full name, not just the email address
    anEvent->setOrganizer( s = fakeCString(vObjectUStringZValue(vo) ) );
    deleteStr(s);
  } else {
    anEvent->setOrganizer( mCalendar->getOwner() );
  }

  // deal with attendees.
  initPropIterator(&voi, vevent);
  while (moreIteration(&voi)) {
    vo = nextVObject(&voi);
    if (strcmp(vObjectName(vo), VCAttendeeProp) == 0) {
      Attendee *a;
      VObject *vp;
      s = fakeCString(vObjectUStringZValue(vo));
      QString tmpStr = QString::fromLocal8Bit(s);
      deleteStr(s);
      tmpStr = tmpStr.simplifyWhiteSpace();
      int emailPos1, emailPos2;
      if ((emailPos1 = tmpStr.find('<')) > 0) {
        // both email address and name
        emailPos2 = tmpStr.findRev('>');
        a = new Attendee(tmpStr.left(emailPos1 - 1),
                         tmpStr.mid(emailPos1 + 1,
                                    emailPos2 - (emailPos1 + 1)));
      } else if (tmpStr.find('@') > 0) {
        // just an email address
        a = new Attendee(0, tmpStr);
      } else {
        // just a name
        QString email = tmpStr.replace( QRegExp(" "), "." );
        a = new Attendee(tmpStr,email);
      }

      // is there an RSVP property?
      if ((vp = isAPropertyOf(vo, VCRSVPProp)) != 0)
        a->setRSVP(vObjectStringZValue(vp));
      // is there a status property?
      if ((vp = isAPropertyOf(vo, VCStatusProp)) != 0)
        a->setStatus(readStatus(vObjectStringZValue(vp)));
      // add the attendee
      anEvent->addAttendee(a);
    }
  }

  // This isn't strictly true.  An event that doesn't have a start time
  // or an end time doesn't "float", it has an anchor in time but it doesn't
  // "take up" any time.
  /*if ((isAPropertyOf(vevent, VCDTstartProp) == 0) ||
      (isAPropertyOf(vevent, VCDTendProp) == 0)) {
    anEvent->setFloats(TRUE);
    } else {
    }*/

  anEvent->setFloats(FALSE);

  // start time
  if ((vo = isAPropertyOf(vevent, VCDTstartProp)) != 0) {
    anEvent->setDtStart(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
    //    kdDebug(5800) << "s is " << //          s << ", ISO is " << ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))).toString() << endl;
    deleteStr(s);
    if (anEvent->dtStart().time().isNull())
      anEvent->setFloats(TRUE);
  }

  // stop time
  if ((vo = isAPropertyOf(vevent, VCDTendProp)) != 0) {
    anEvent->setDtEnd(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
      deleteStr(s);
      if (anEvent->dtEnd().time().isNull())
        anEvent->setFloats(TRUE);
  }

  // at this point, there should be at least a start or end time.
  // fix up for events that take up no time but have a time associated
  if (!(vo = isAPropertyOf(vevent, VCDTstartProp)))
    anEvent->setDtStart(anEvent->dtEnd());
  if (!(vo = isAPropertyOf(vevent, VCDTendProp)))
    anEvent->setDtEnd(anEvent->dtStart());

  ///////////////////////////////////////////////////////////////////////////

  // repeat stuff
  if ((vo = isAPropertyOf(vevent, VCRRuleProp)) != 0) {
    QString tmpStr = (s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
    tmpStr.simplifyWhiteSpace();
    tmpStr = tmpStr.upper();

    /********************************* DAILY ******************************/
    if (tmpStr.left(1) == "D") {
      int index = tmpStr.find(' ');
      int rFreq = tmpStr.mid(1, (index-1)).toInt();
      index = tmpStr.findRev(' ') + 1; // advance to last field
      if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
        QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index))).date();
        anEvent->recurrence()->setDaily(rFreq, rEndDate);
      } else {
        int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
        if (rDuration == 0) // VEvents set this to 0 forever, we use -1
          anEvent->recurrence()->setDaily(rFreq, -1);
        else
          anEvent->recurrence()->setDaily(rFreq, rDuration);
      }
    }
    /********************************* WEEKLY ******************************/
    else if (tmpStr.left(1) == "W") {
      int index = tmpStr.find(' ');
      int last = tmpStr.findRev(' ') + 1;
      int rFreq = tmpStr.mid(1, (index-1)).toInt();
      index += 1; // advance to beginning of stuff after freq
      QBitArray qba(7);
      QString dayStr;
      if( index == last ) {
        // e.g. W1 #0
        qba.setBit(anEvent->dtStart().date().dayOfWeek() - 1);
      }
      else {
        // e.g. W1 SU #0
        while (index < last) {
          dayStr = tmpStr.mid(index, 3);
          int dayNum = numFromDay(dayStr);
          qba.setBit(dayNum);
          index += 3; // advance to next day, or possibly "#"
        }
      }
      index = last; if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
        QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index))).date();
        anEvent->recurrence()->setWeekly(rFreq, qba, rEndDate);
      } else {
        int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
        if (rDuration == 0)
          anEvent->recurrence()->setWeekly(rFreq, qba, -1);
        else
          anEvent->recurrence()->setWeekly(rFreq, qba, rDuration);
      }
    }
    /**************************** MONTHLY-BY-POS ***************************/
    else if (tmpStr.left(2) == "MP") {
      int index = tmpStr.find(' ');
      int last = tmpStr.findRev(' ') + 1;
      int rFreq = tmpStr.mid(2, (index-1)).toInt();
      index += 1; // advance to beginning of stuff after freq
      QBitArray qba(7);
      short tmpPos;
      if( index == last ) {
        // e.g. MP1 #0
        tmpPos = anEvent->dtStart().date().day()/7 + 1;
        if( tmpPos == 5 )
          tmpPos = -1;
        qba.setBit(anEvent->dtStart().date().dayOfWeek() - 1);
        anEvent->recurrence()->addMonthlyPos(tmpPos, qba);
      }
      else {
        // e.g. MP1 1+ SU #0
        while (index < last) {
          tmpPos = tmpStr.mid(index,1).toShort();
          index += 1;
          if (tmpStr.mid(index,1) == "-")
            // convert tmpPos to negative
            tmpPos = 0 - tmpPos;
          index += 2; // advance to day(s)
          while (numFromDay(tmpStr.mid(index,3)) >= 0) {
            int dayNum = numFromDay(tmpStr.mid(index,3));
            qba.setBit(dayNum);
            index += 3; // advance to next day, or possibly pos or "#"
          }
          anEvent->recurrence()->addMonthlyPos(tmpPos, qba);
          qba.detach();
          qba.fill(FALSE); // clear out
        } // while != "#"
      }
      index = last; if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
        QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length() -
                                                    index))).date();
        anEvent->recurrence()->setMonthly(Recurrence::rMonthlyPos, rFreq, rEndDate);
      } else {
        int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
        if (rDuration == 0)
          anEvent->recurrence()->setMonthly(Recurrence::rMonthlyPos, rFreq, -1);
        else
          anEvent->recurrence()->setMonthly(Recurrence::rMonthlyPos, rFreq, rDuration);
      }
    }

    /**************************** MONTHLY-BY-DAY ***************************/
    else if (tmpStr.left(2) == "MD") {
      int index = tmpStr.find(' ');
      int last = tmpStr.findRev(' ') + 1;
      int rFreq = tmpStr.mid(2, (index-1)).toInt();
      index += 1;
      short tmpDay;
      // We have to set monthly by day now (using dummy values), because the
      // addMonthlyDay calls check for that type of recurrence, and if the
      // recurrence isn't yet set to monthly, addMonthlyDay doesn't do anything
      anEvent->recurrence()->setMonthly( Recurrence::rMonthlyDay, rFreq, -1 );
      if( index == last ) {
        // e.g. MD1 #0
        tmpDay = anEvent->dtStart().date().day();
        anEvent->recurrence()->addMonthlyDay(tmpDay);
      }
      else {
        // e.g. MD1 3 #0
        while (index < last) {
          int index2 = tmpStr.find(' ', index);
          tmpDay = tmpStr.mid(index, (index2-index)).toShort();
          index = index2-1;
          if (tmpStr.mid(index, 1) == "-")
            tmpDay = 0 - tmpDay;
          index += 2; // advance the index;
          anEvent->recurrence()->addMonthlyDay(tmpDay);
        } // while != #
      }
      index = last; if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
        QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index))).date();
        anEvent->recurrence()->setMonthly(Recurrence::rMonthlyDay, rFreq, rEndDate);
      } else {
        int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
        if (rDuration == 0)
          anEvent->recurrence()->setMonthly(Recurrence::rMonthlyDay, rFreq, -1);
        else
          anEvent->recurrence()->setMonthly(Recurrence::rMonthlyDay, rFreq, rDuration);
      }
    }

    /*********************** YEARLY-BY-MONTH *******************************/
    else if (tmpStr.left(2) == "YM") {
      int index = tmpStr.find(' ');
      int last = tmpStr.findRev(' ') + 1;
      int rFreq = tmpStr.mid(2, (index-1)).toInt();
      index += 1;
      short tmpMonth;
      if( index == last ) {
        // e.g. YM1 #0
        tmpMonth = anEvent->dtStart().date().month();
        anEvent->recurrence()->addYearlyNum(tmpMonth);
      }
      else {
        // e.g. YM1 3 #0
        while (index < last) {
          int index2 = tmpStr.find(' ', index);
          tmpMonth = tmpStr.mid(index, (index2-index)).toShort();
          index = index2+1;
          anEvent->recurrence()->addYearlyNum(tmpMonth);
        } // while != #
      }
      index = last; if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
        QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index))).date();
        anEvent->recurrence()->setYearly(Recurrence::rYearlyMonth, rFreq, rEndDate);
      } else {
        int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
        if (rDuration == 0)
          anEvent->recurrence()->setYearly(Recurrence::rYearlyMonth, rFreq, -1);
        else
          anEvent->recurrence()->setYearly(Recurrence::rYearlyMonth, rFreq, rDuration);
      }
    }

    /*********************** YEARLY-BY-DAY *********************************/
    else if (tmpStr.left(2) == "YD") {
      int index = tmpStr.find(' ');
      int last = tmpStr.findRev(' ') + 1;
      int rFreq = tmpStr.mid(2, (index-1)).toInt();
      index += 1;
      short tmpDay;
      if( index == last ) {
        // e.g. YD1 #0
        tmpDay = anEvent->dtStart().date().dayOfYear();
        anEvent->recurrence()->addYearlyNum(tmpDay);
      }
      else {
        // e.g. YD1 123 #0
        while (index < last) {
          int index2 = tmpStr.find(' ', index);
          tmpDay = tmpStr.mid(index, (index2-index)).toShort();
          index = index2+1;
          anEvent->recurrence()->addYearlyNum(tmpDay);
        } // while != #
      }
      index = last; if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
        QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index))).date();
        anEvent->recurrence()->setYearly(Recurrence::rYearlyDay, rFreq, rEndDate);
      } else {
        int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
        if (rDuration == 0)
          anEvent->recurrence()->setYearly(Recurrence::rYearlyDay, rFreq, -1);
        else
          anEvent->recurrence()->setYearly(Recurrence::rYearlyDay, rFreq, rDuration);
      }
    } else {
      kdDebug(5800) << "we don't understand this type of recurrence!" << endl;
    } // if
  } // repeats


  // recurrence exceptions
  if ((vo = isAPropertyOf(vevent, VCExDateProp)) != 0) {
    s = fakeCString(vObjectUStringZValue(vo));
    QStringList exDates = QStringList::split(",",s);
    QStringList::ConstIterator it;
    for(it = exDates.begin(); it != exDates.end(); ++it ) {
      anEvent->addExDate(ISOToQDate(*it));
    }
    deleteStr(s);
  }

  // summary
  if ((vo = isAPropertyOf(vevent, VCSummaryProp))) {
    s = fakeCString(vObjectUStringZValue(vo));
    anEvent->setSummary(QString::fromLocal8Bit(s));
    deleteStr(s);
  }

  // description
  if ((vo = isAPropertyOf(vevent, VCDescriptionProp)) != 0) {
    s = fakeCString(vObjectUStringZValue(vo));
    if (!anEvent->description().isEmpty()) {
      anEvent->setDescription(anEvent->description() + "\n" +
                              QString::fromLocal8Bit(s));
    } else {
      anEvent->setDescription(QString::fromLocal8Bit(s));
    }
    deleteStr(s);
  }

  // location
  if ((vo = isAPropertyOf(vevent, VCLocationProp)) != 0) {
    s = fakeCString(vObjectUStringZValue(vo));
    anEvent->setLocation( QString::fromLocal8Bit(s) );
    deleteStr(s);
  }

  // some stupid vCal exporters ignore the standard and use Description
  // instead of Summary for the default field.  Correct for this.
  if (anEvent->summary().isEmpty() &&
      !(anEvent->description().isEmpty())) {
    QString tmpStr = anEvent->description().simplifyWhiteSpace();
    anEvent->setDescription("");
    anEvent->setSummary(tmpStr);
  }

#if 0
  // status
  if ((vo = isAPropertyOf(vevent, VCStatusProp)) != 0) {
    QString tmpStr(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
// TODO: Define Event status
//    anEvent->setStatus(tmpStr);
  }
  else
//    anEvent->setStatus("NEEDS ACTION");
#endif

  // secrecy
  int secrecy = Incidence::SecrecyPublic;
  if ((vo = isAPropertyOf(vevent, VCClassProp)) != 0) {
    s = fakeCString(vObjectUStringZValue(vo));
    if (strcmp(s,"PRIVATE") == 0) {
      secrecy = Incidence::SecrecyPrivate;
    } else if (strcmp(s,"CONFIDENTIAL") == 0) {
      secrecy = Incidence::SecrecyConfidential;
    }
    deleteStr(s);
  }
  anEvent->setSecrecy(secrecy);

  // categories
  QStringList tmpStrList;
  int index1 = 0;
  int index2 = 0;
  if ((vo = isAPropertyOf(vevent, VCCategoriesProp)) != 0) {
    s = fakeCString(vObjectUStringZValue(vo));
    QString categories = QString::fromLocal8Bit(s);
    deleteStr(s);
    //const char* category;
    QString category;
    while ((index2 = categories.find(',', index1)) != -1) {
        //category = (const char *) categories.mid(index1, (index2 - index1));
      category = categories.mid(index1, (index2 - index1));
      tmpStrList.append(category);
      index1 = index2+1;
    }
    // get last category
    category = categories.mid(index1, (categories.length()-index1));
    tmpStrList.append(category);
    anEvent->setCategories(tmpStrList);
  }

  // attachments
  tmpStrList.clear();
  initPropIterator(&voi, vevent);
  while (moreIteration(&voi)) {
    vo = nextVObject(&voi);
    if (strcmp(vObjectName(vo), VCAttachProp) == 0) {
      s = fakeCString(vObjectUStringZValue(vo));
      anEvent->addAttachment(new Attachment(QString(s)));
      deleteStr(s);
    }
  }

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

  /* alarm stuff */
  if ((vo = isAPropertyOf(vevent, VCDAlarmProp))) {
    Alarm* alarm = anEvent->newAlarm();
    VObject *a;
    if ((a = isAPropertyOf(vo, VCRunTimeProp))) {
      alarm->setTime(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(a))));
      deleteStr(s);
    }
    alarm->setEnabled(true);
    if ((vo = isAPropertyOf(vevent, VCPAlarmProp))) {
      if ((a = isAPropertyOf(vo, VCProcedureNameProp))) {
        s = fakeCString(vObjectUStringZValue(a));
        alarm->setProcedureAlarm(QFile::decodeName(s));
        deleteStr(s);
      }
    }
    if ((vo = isAPropertyOf(vevent, VCAAlarmProp))) {
      if ((a = isAPropertyOf(vo, VCAudioContentProp))) {
        s = fakeCString(vObjectUStringZValue(a));
        alarm->setAudioAlarm(QFile::decodeName(s));
        deleteStr(s);
      }
    }
  }

  // priority
  if ((vo = isAPropertyOf(vevent, VCPriorityProp))) {
    anEvent->setPriority(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }

  // transparency
  if ((vo = isAPropertyOf(vevent, VCTranspProp)) != 0) {
    int i = atoi(s = fakeCString(vObjectUStringZValue(vo)));
    anEvent->setTransparency( i == 1 ? Event::Transparent : Event::Opaque );
    deleteStr(s);
  }

  // related event
  if ((vo = isAPropertyOf(vevent, VCRelatedToProp)) != 0) {
    anEvent->setRelatedToUid(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
    mEventsRelate.append(anEvent);
  }

  /* PILOT SYNC STUFF */
  if ((vo = isAPropertyOf(vevent, KPilotIdProp))) {
    anEvent->setPilotId(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setPilotId(0);

  if ((vo = isAPropertyOf(vevent, KPilotStatusProp))) {
    anEvent->setSyncStatus(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setSyncStatus(Event::SYNCMOD);

  return anEvent;
}


QString VCalFormat::qDateToISO(const QDate &qd)
{
  QString tmpStr;

  Q_ASSERT(qd.isValid());

  tmpStr.sprintf("%.2d%.2d%.2d",
                 qd.year(), qd.month(), qd.day());
  return tmpStr;

}

/* Return the offset of the named zone as seconds. tt is a time
   indicating the date for which you want the offset */
int vcaltime_utc_offset( QDateTime ictt, QString tzid )
{
  struct icaltimetype tt = icaltime_from_timet( ictt.toTime_t(), false );
  return icaltime_utc_offset( tt, tzid.latin1() );
}

QString VCalFormat::qDateTimeToISO(const QDateTime &qdt, bool zulu)
{
  QString tmpStr;

  Q_ASSERT(qdt.date().isValid());
  Q_ASSERT(qdt.time().isValid());
  if (zulu) {
    QDateTime tmpDT(qdt);
    // correct to GMT:
    tmpDT = tmpDT.addSecs(-vcaltime_utc_offset( tmpDT, mCalendar->timeZoneId()));
    tmpStr.sprintf( "%.2d%.2d%.2dT%.2d%.2d%.2dZ",
                    tmpDT.date().year(), tmpDT.date().month(),
                    tmpDT.date().day(), tmpDT.time().hour(),
                    tmpDT.time().minute(), tmpDT.time().second());
  } else {
    tmpStr.sprintf( "%.2d%.2d%.2dT%.2d%.2d%.2d",
                    qdt.date().year(), qdt.date().month(),
                    qdt.date().day(), qdt.time().hour(),
                    qdt.time().minute(), qdt.time().second());
  }
  return tmpStr;
}

QDateTime VCalFormat::ISOToQDateTime(const QString & dtStr)
{
  QDate tmpDate;
  QTime tmpTime;
  QString tmpStr;
  int year, month, day, hour, minute, second;

  tmpStr = dtStr;
  year = tmpStr.left(4).toInt();
  month = tmpStr.mid(4,2).toInt();
  day = tmpStr.mid(6,2).toInt();
  hour = tmpStr.mid(9,2).toInt();
  minute = tmpStr.mid(11,2).toInt();
  second = tmpStr.mid(13,2).toInt();
  tmpDate.setYMD(year, month, day);
  tmpTime.setHMS(hour, minute, second);

  Q_ASSERT(tmpDate.isValid());
  Q_ASSERT(tmpTime.isValid());
  QDateTime tmpDT(tmpDate, tmpTime);
  // correct for GMT if string is in Zulu format
  if (dtStr.at(dtStr.length()-1) == 'Z') {
    tmpDT = tmpDT.addSecs(vcaltime_utc_offset( tmpDT, mCalendar->timeZoneId()));
  }
  return tmpDT;
}

QDate VCalFormat::ISOToQDate(const QString &dateStr)
{
  int year, month, day;

  year = dateStr.left(4).toInt();
  month = dateStr.mid(4,2).toInt();
  day = dateStr.mid(6,2).toInt();

  return(QDate(year, month, day));
}

// take a raw vcalendar (i.e. from a file on disk, clipboard, etc. etc.
// and break it down from it's tree-like format into the dictionary format
// that is used internally in the VCalFormat.
void VCalFormat::populate(VObject *vcal)
{
  // this function will populate the caldict dictionary and other event
  // lists. It turns vevents into Events and then inserts them.

  VObjectIterator i;
  VObject *curVO, *curVOProp;
  Event *anEvent;

  if ((curVO = isAPropertyOf(vcal, ICMethodProp)) != 0) {
    char *methodType = 0;
    methodType = fakeCString(vObjectUStringZValue(curVO));
    kdDebug(5800) << "This calendar is an iTIP transaction of type '"
              << methodType << "'" << endl;
    delete methodType;
  }

  // warn the user that we might have trouble reading non-known calendar.
  if ((curVO = isAPropertyOf(vcal, VCProdIdProp)) != 0) {
    char *s = fakeCString(vObjectUStringZValue(curVO));
    if (strcmp(productId().local8Bit(), s) != 0)
      kdDebug(5800) << "This vCalendar file was not created by KOrganizer "
                   "or any other product we support. Loading anyway..." << endl;
    mLoadedProductId = s;
    deleteStr(s);
  }

  // warn the user we might have trouble reading this unknown version.
  if ((curVO = isAPropertyOf(vcal, VCVersionProp)) != 0) {
    char *s = fakeCString(vObjectUStringZValue(curVO));
    if (strcmp(_VCAL_VERSION, s) != 0)
      kdDebug(5800) << "This vCalendar file has version " << s
                << "We only support " << _VCAL_VERSION << endl;
    deleteStr(s);
  }

#if 0
  // set the time zone (this is a property of the view, so just discard!)
  if ((curVO = isAPropertyOf(vcal, VCTimeZoneProp)) != 0) {
    char *s = fakeCString(vObjectUStringZValue(curVO));
    mCalendar->setTimeZone(s);
    deleteStr(s);
  }
#endif

  // Store all events with a relatedTo property in a list for post-processing
  mEventsRelate.clear();
  mTodosRelate.clear();

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

        if (mCalendar->incidence(tmpStr)) {
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
      if (anEvent) {
              if ( !anEvent->dtStart().isValid() || !anEvent->dtEnd().isValid() ) {
          kdDebug(5800) << "VCalFormat::populate(): Event has invalid dates."
                    << endl;
        } else {
          mCalendar->addEvent(anEvent);
              }
      } else {
        // some sort of error must have occurred while in translation.
        goto SKIP;
      }
    } else if (strcmp(vObjectName(curVO), VCTodoProp) == 0) {
      Todo *aTodo = VTodoToEvent(curVO);
      mCalendar->addTodo(aTodo);
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

  // Post-Process list of events with relations, put Event objects in relation
  Event::List::ConstIterator eIt;
  for ( eIt = mEventsRelate.begin(); eIt != mEventsRelate.end(); ++eIt ) {
    (*eIt)->setRelatedTo( mCalendar->incidence( (*eIt)->relatedToUid() ) );
  }
  Todo::List::ConstIterator tIt;
  for ( tIt = mTodosRelate.begin(); tIt != mTodosRelate.end(); ++tIt ) {
    (*tIt)->setRelatedTo( mCalendar->incidence( (*tIt)->relatedToUid() ) );
   }
}

const char *VCalFormat::dayFromNum(int day)
{
  const char *days[7] = { "MO ", "TU ", "WE ", "TH ", "FR ", "SA ", "SU " };

  return days[day];
}

int VCalFormat::numFromDay(const QString &day)
{
  if (day == "MO ") return 0;
  if (day == "TU ") return 1;
  if (day == "WE ") return 2;
  if (day == "TH ") return 3;
  if (day == "FR ") return 4;
  if (day == "SA ") return 5;
  if (day == "SU ") return 6;

  return -1; // something bad happened. :)
}

Attendee::PartStat VCalFormat::readStatus(const char *s) const
{
  QString statStr = s;
  statStr = statStr.upper();
  Attendee::PartStat status;

  if (statStr == "X-ACTION")
    status = Attendee::NeedsAction;
  else if (statStr == "NEEDS ACTION")
    status = Attendee::NeedsAction;
  else if (statStr== "ACCEPTED")
    status = Attendee::Accepted;
  else if (statStr== "SENT")
    status = Attendee::NeedsAction;
  else if (statStr== "TENTATIVE")
    status = Attendee::Tentative;
  else if (statStr== "CONFIRMED")
    status = Attendee::Accepted;
  else if (statStr== "DECLINED")
    status = Attendee::Declined;
  else if (statStr== "COMPLETED")
    status = Attendee::Completed;
  else if (statStr== "DELEGATED")
    status = Attendee::Delegated;
  else {
    kdDebug(5800) << "error setting attendee mStatus, unknown mStatus!" << endl;
    status = Attendee::NeedsAction;
  }

  return status;
}

QCString VCalFormat::writeStatus(Attendee::PartStat status) const
{
  switch(status) {
    default:
    case Attendee::NeedsAction:
      return "NEEDS ACTION";
      break;
    case Attendee::Accepted:
      return "ACCEPTED";
      break;
    case Attendee::Declined:
      return "DECLINED";
      break;
    case Attendee::Tentative:
      return "TENTATIVE";
      break;
    case Attendee::Delegated:
      return "DELEGATED";
      break;
    case Attendee::Completed:
      return "COMPLETED";
      break;
    case Attendee::InProcess:
      return "NEEDS ACTION";
      break;
  }
}
