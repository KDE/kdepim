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

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qregexp.h>
#include <qclipboard.h>
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

#define _ICAL_VERSION "2.0"

using namespace KCal;

ICalFormat::ICalFormat(Calendar *cal) :
  CalFormat(cal)
{
  mImpl = new ICalFormatImpl(this,cal);
}

ICalFormat::~ICalFormat()
{
  delete mImpl;
}

bool ICalFormat::load(const QString &fileName)
{
  kdDebug(5800) << "ICalFormat::load() " << fileName << endl;

  clearException();

  QFile file( fileName );
  if (!file.open( IO_ReadOnly ) ) {
    kdDebug(5800) << "ICalFormat::load() load error" << endl;
    setException(new ErrorFormat(ErrorFormat::LoadError));
    return false;
  }
  QTextStream ts( &file );
  ts.setEncoding(QTextStream::UnicodeUTF8);
  QString text = ts.read();
  file.close();

  // Get first VCALENDAR component.
  // TODO: Handle more than one VCALENDAR or non-VCALENDAR top components
  icalcomponent *calendar;

  calendar = icalcomponent_new_from_string( text.local8Bit().data());
  //  kdDebug(5800) << "Error: " << icalerror_perror() << endl;
  if (!calendar) {
    kdDebug(5800) << "ICalFormat::load() parse error" << endl;
    setException(new ErrorFormat(ErrorFormat::ParseErrorIcal));
    return false;
  }

  if (icalcomponent_isa(calendar) != ICAL_VCALENDAR_COMPONENT) {
    kdDebug(5800) << "ICalFormat::load(): No VCALENDAR component found" << endl;
    setException(new ErrorFormat(ErrorFormat::NoCalendar));
    return false;
  }

  // put all objects into their proper places
  if (!mImpl->populate(calendar)) {
    kdDebug(5800) << "ICalFormat::load(): Could not populate calendar" << endl;
    setException(new ErrorFormat(ErrorFormat::ParseErrorKcal));
    return false;
  }

  return true;
}


bool ICalFormat::save(const QString &fileName)
{
  kdDebug(5800) << "ICalFormat::save(): " << fileName << endl;

  clearException();

  icalcomponent *calendar = mImpl->createCalendarComponent();
  
  icalcomponent *component;

  // todos
  QPtrList<Todo> todoList = mCalendar->getTodoList();
  QPtrListIterator<Todo> qlt(todoList);
  for (; qlt.current(); ++qlt) {
    component = mImpl->writeTodo(qlt.current());
    icalcomponent_add_component(calendar,component);
  }

  // events
  QPtrList<Event> events = mCalendar->getAllEvents();
  Event *ev;
  for(ev=events.first();ev;ev=events.next()) {
    component = mImpl->writeEvent(ev);
    icalcomponent_add_component(calendar,component);
  }

  // journals
  QPtrList<Journal> journals = mCalendar->journalList();
  Journal *j;
  for(j=journals.first();j;j=journals.next()) {
    component = mImpl->writeJournal(j);
    icalcomponent_add_component(calendar,component);
  }

  // TODO: write backup file

  QFile file( fileName );
  if (!file.open( IO_WriteOnly ) ) {
    setException(new ErrorFormat(ErrorFormat::SaveError,
                 i18n("Could not open file �%1�").arg(fileName)));
    return false;    
  }
  QTextStream ts( &file );
  ts.setEncoding(QTextStream::UnicodeUTF8);
  const char *text = icalcomponent_as_ical_string( calendar );
  if (!text) {
    setException(new ErrorFormat(ErrorFormat::SaveError,
                 i18n("libical error")));
    file.close();
    return false;
  }
  ts << QString::fromLocal8Bit(text);
  file.close();

  return true;
}

// Disabled until iCalendar drag and drop is implemented
VCalDrag *ICalFormat::createDrag(Event */*selectedEv*/, QWidget */*owner*/)
{
  return 0;
#if 0
  VObject *vcal, *vevent;
  QString tmpStr;
  
  vcal = newVObject(VCCalProp);
  
  addPropValue(vcal,VCProdIdProp, productId());
  tmpStr = mCalendar->getTimeZoneStr();
  addPropValue(vcal,VCTimeZoneProp, tmpStr.local8Bit());
  addPropValue(vcal,VCVersionProp, _VCAL_VERSION);
  
  vevent = eventToVEvent(selectedEv);
  
  addVObjectProp(vcal, vevent);

  VCalDrag *vcd = new VCalDrag(vcal, owner);
  // free memory associated with vCalendar stuff
  cleanVObject(vcal);  
  vcd->setPixmap(BarIcon("appointment"));

  return vcd;
#endif
}

VCalDrag *ICalFormat::createDragTodo(Todo */*selectedEv*/, QWidget */*owner*/)
{
  return 0;
#if 0
  VObject *vcal, *vevent;
  QString tmpStr;
  
  vcal = newVObject(VCCalProp);
  
  addPropValue(vcal,VCProdIdProp, productId());
  tmpStr = mCalendar->getTimeZoneStr();
  addPropValue(vcal,VCTimeZoneProp, tmpStr.local8Bit());
  addPropValue(vcal,VCVersionProp, _VCAL_VERSION);
  
  vevent = eventToVTodo(selectedEv);
  
  addVObjectProp(vcal, vevent);

  VCalDrag *vcd = new VCalDrag(vcal, owner);
  // free memory associated with vCalendar stuff
  cleanVObject(vcal);  
  vcd->setPixmap(BarIcon("todo"));

  return vcd;
#endif
}

Event *ICalFormat::createDrop(QDropEvent */*de*/)
{
  return 0;
#if 0
  VObject *vcal;
  Event *event = 0;

  if (VCalDrag::decode(de, &vcal)) {
    de->accept();
    VObjectIterator i;
    VObject *curvo;
    initPropIterator(&i, vcal);
    
    // we only take the first object.
    do  {
      curvo = nextVObject(&i);
    } while (strcmp(vObjectName(curvo), VCEventProp) &&
             strcmp(vObjectName(curvo), VCTodoProp));

    if (strcmp(vObjectName(curvo), VCTodoProp) == 0) {
      kdDebug(5800) << "ICalFormat::createDrop(): Got todo instead of event." << endl;
    } else if (strcmp(vObjectName(curvo), VCEventProp) == 0) {
      event = VEventToEvent(curvo);
    } else {
      kdDebug(5800) << "ICalFormat::createDropTodo(): Unknown event type in drop." << endl;
    }
    // get rid of temporary VObject
    deleteVObject(vcal);
  }
  
  return event;
#endif
}

Todo *ICalFormat::createDropTodo(QDropEvent */*de*/)
{
  return 0;
#if 0
  VObject *vcal;
  Event *event = 0;

  if (VCalDrag::decode(de, &vcal)) {
    de->accept();
    VObjectIterator i;
    VObject *curvo;
    initPropIterator(&i, vcal);
    
    // we only take the first object.
    do  {
      curvo = nextVObject(&i);
    } while (strcmp(vObjectName(curvo), VCEventProp) &&
             strcmp(vObjectName(curvo), VCTodoProp));

    if (strcmp(vObjectName(curvo), VCEventProp) == 0) {
      kdDebug(5800) << "ICalFormat::createDropTodo(): Got event instead of todo." << endl;
    } else if (strcmp(vObjectName(curvo), VCTodoProp) == 0) {
      event = VTodoToEvent(curvo);
    } else {
      kdDebug(5800) << "ICalFormat::createDropTodo(): Unknown event type in drop." << endl;
    }
    // get rid of temporary VObject
    deleteVObject(vcal);
  }
  
  return event;
#endif
}

bool ICalFormat::copyEvent(Event */*selectedEv*/)
{
  return false;
#if 0
  QClipboard *cb = QApplication::clipboard();
  VObject *vcal, *vevent;
  QString tmpStr;

  vcal = newVObject(VCCalProp);

  //  addPropValue(vcal,VCLocationProp, "0.0");
  addPropValue(vcal,VCProdIdProp, productId());
  tmpStr = mCalendar->getTimeZoneStr();
  addPropValue(vcal,VCTimeZoneProp, tmpStr.local8Bit());
  addPropValue(vcal,VCVersionProp, _VCAL_VERSION);

  vevent = eventToVEvent(selectedEv);

  addVObjectProp(vcal, vevent);

  // paste to clipboard
  cb->setData(new VCalDrag(vcal));
  
  // free memory associated with vCalendar stuff
  cleanVObject(vcal);
  
  return TRUE;
#endif
}

Event *ICalFormat::pasteEvent(const QDate &/*newDate*/,const QTime */*newTime*/)
{
  return 0;
#if 0
  VObject *vcal, *curVO, *curVOProp;
  VObjectIterator i;
  int daysOffset;

  Event *anEvent = 0L;

  QClipboard *cb = QApplication::clipboard();
  int bufsize;
  const char * buf;
  buf = cb->text().local8Bit();
  bufsize = strlen(buf);

  if (!VCalDrag::decode(cb->data(),&vcal)) {
    if (mEnableDialogs) {
      KMessageBox::sorry(mTopWidget, 
                            i18n("An error has occurred parsing the "
                                 "contents of the clipboard.\nYou can "
                                 "only paste a valid vCalendar into "
                                 "%1.\n").arg(application()),
                            i18n("%1: Paste Calendar").arg(application()));
      return 0;
    }
  }  

  initPropIterator(&i, vcal);
  
  // we only take the first object.
  do  {
    curVO = nextVObject(&i);
  } while (strcmp(vObjectName(curVO), VCEventProp));
  
  // now, check to see that the object is BOTH an event, and if so,
  // that it has a starting date
  if (strcmp(vObjectName(curVO), VCEventProp) == 0) {
    if ((curVOProp = isAPropertyOf(curVO, VCDTstartProp)) ||
	(curVOProp = isAPropertyOf(curVO, VCDTendProp))) {
      
      // we found an event with a start time, put it in the dict
      anEvent = VEventToEvent(curVO);
      // if we pasted an event that was the result of a copy in our
      // own calendar, now we have duplicate UID strings.  Need to generate
      // a new one for this new event.
      QString uidStr = createUniqueId();
      if (mCalendar->getEvent(anEvent->VUID()))
	anEvent->setVUID(uidStr);

      daysOffset = anEvent->dtEnd().date().dayOfYear() - 
	anEvent->dtStart().date().dayOfYear();
      
      if (newTime)
	anEvent->setDtStart(QDateTime(*newDate, *newTime));
      else
	anEvent->setDtStart(QDateTime(*newDate, anEvent->dtStart().time()));
      
      anEvent->setDtEnd(QDateTime(newDate->addDays(daysOffset),
				  anEvent->dtEnd().time()));
      mCalendar->addEvent(anEvent);
    } else {
      kdDebug(5800) << "found a VEvent with no DTSTART/DTEND! Skipping" << endl;
    }
  } else if (strcmp(vObjectName(curVO), VCTodoProp) == 0) {
    anEvent = VTodoToEvent(curVO);
    mCalendar->addTodo(anEvent);
  } else {
    kdDebug(5800) << "unknown event type in paste!!!" << endl;
  }
  // get rid of temporary VObject
  deleteVObject(vcal);
  return anEvent;
#endif
}


QString ICalFormat::createScheduleMessage(Incidence *incidence,
                                          Scheduler::Method method)
{
  icalcomponent *message = mImpl->createScheduleComponent(incidence,method);

  QString messageText = icalcomponent_as_ical_string(message);

#if 0
  kdDebug(5800) << "ICalFormat::createScheduleMessage: message START\n"
            << messageText
            << "ICalFormat::createScheduleMessage: message END" << endl;
#endif

  return messageText;
}

ScheduleMessage *ICalFormat::parseScheduleMessage(const QString &messageText)
{
  clearException();

  if (messageText.isEmpty()) return 0;

  icalcomponent *message;
  message = icalparser_parse_string(messageText.local8Bit());
  
  if (!message) return 0;
  
  icalproperty *m = icalcomponent_get_first_property(message,
                                                     ICAL_METHOD_PROPERTY);
                                                          
  if (!m) return 0;
  
  icalcomponent *c;
  
  Incidence *incidence = 0;
  c = icalcomponent_get_first_component(message,ICAL_VEVENT_COMPONENT);
  if (c) {
    incidence = mImpl->readEvent(c);
  } else {
    c = icalcomponent_get_first_component(message,ICAL_VTODO_COMPONENT);
    if (c) {
      incidence = mImpl->readTodo(c);
    }
  }

  if (!incidence) return 0;

  kdDebug(5800) << "ICalFormat::parseScheduleMessage() getting method..." << endl;

  icalproperty_method icalmethod = icalproperty_get_method(m);
  Scheduler::Method method;
  
  switch (icalmethod) {
    case ICAL_METHOD_PUBLISH:
      method = Scheduler::Publish;
      break;
    case ICAL_METHOD_REQUEST:
      method = Scheduler::Request;
      break;
    case ICAL_METHOD_REFRESH:
      method = Scheduler::Refresh;
      break;
    case ICAL_METHOD_CANCEL:
      method = Scheduler::Cancel;
      break;
    case ICAL_METHOD_ADD:
      method = Scheduler::Add;
      break;
    case ICAL_METHOD_REPLY:
      method = Scheduler::Reply;
      break;
    case ICAL_METHOD_COUNTER:
      method = Scheduler::Counter;
      break;
    case ICAL_METHOD_DECLINECOUNTER:
      method = Scheduler::Declinecounter;
      break;
    default:
      method = Scheduler::NoMethod;
      kdDebug(5800) << "ICalFormat::parseScheduleMessage(): Unknow method" << endl;
      break;
  }

  kdDebug(5800) << "ICalFormat::parseScheduleMessage() restriction..." << endl;

  if (!icalrestriction_check(message)) {
    setException(new ErrorFormat(ErrorFormat::Restriction,
                                   Scheduler::methodName(method) + ": " +
                                   mImpl->extractErrorProperty(c)));
    return 0;
  }
  
  icalcomponent *calendarComponent = mImpl->createCalendarComponent();

  Incidence *existingIncidence = mCalendar->getEvent(incidence->VUID());
  if (existingIncidence) {
    // TODO: check, if dynamic cast is required
    Todo *todo = dynamic_cast<Todo *>(existingIncidence);
    if (todo) {
      icalcomponent_add_component(calendarComponent,
                                  mImpl->writeTodo(todo));
    }
    Event *event = dynamic_cast<Event *>(existingIncidence);
    if (event) {
      icalcomponent_add_component(calendarComponent,
                                  mImpl->writeEvent(event));
    }
  } else {
    calendarComponent = 0;
  }

  kdDebug(5800) << "ICalFormat::parseScheduleMessage() classify..." << endl;

  icalclass result = icalclassify(message,calendarComponent,(char *)"");
  
  kdDebug(5800) << "ICalFormat::parseScheduleMessage() returning..." << endl;

  ScheduleMessage::Status status;

  switch (result) {
    case ICAL_PUBLISH_NEW_CLASS:
      status = ScheduleMessage::PublishNew;
      break;
    case ICAL_OBSOLETE_CLASS:
      status = ScheduleMessage::Obsolete;
      break;
    case ICAL_REQUEST_NEW_CLASS:
      status = ScheduleMessage::RequestNew;
      break;
    case ICAL_REQUEST_UPDATE_CLASS:
      status = ScheduleMessage::RequestUpdate;
      break;
    case ICAL_UNKNOWN_CLASS:
    default:
      status = ScheduleMessage::Unknown;
      break;
  }
  
  return new ScheduleMessage(incidence,method,status);
}
