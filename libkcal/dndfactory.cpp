/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brwon
    Copyright (c) 2001,2002 Cornelius Schumacher <schumacher@kde.org>

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

#include "vcc.h"
#include "vobject.h"

#include <qapplication.h>
#include <qclipboard.h>

#include <kiconloader.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "vcaldrag.h"
#include "calendar.h"
#include "vcalformat.h"

#include "dndfactory.h"

using namespace KCal;

DndFactory::DndFactory( Calendar *cal ) :
  mCalendar( cal )
{
}

VCalDrag *DndFactory::createDrag(Event *selectedEv, QWidget *owner)
{
  VObject *vcal, *vevent;
  QString tmpStr;

  vcal = newVObject(VCCalProp);

  addPropValue(vcal,VCProdIdProp, CalFormat::productId());
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
}

VCalDrag *DndFactory::createDragTodo(Todo *selectedEv, QWidget *owner)
{
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
}

Event *DndFactory::createDrop(QDropEvent *de)
{
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
      kdDebug(5800) << "VCalFormat::createDrop(): Got todo instead of event." << endl;
    } else if (strcmp(vObjectName(curvo), VCEventProp) == 0) {
      event = VEventToEvent(curvo);
    } else {
      kdDebug(5800) << "VCalFormat::createDropTodo(): Unknown event type in drop." << endl;
    }
    // get rid of temporary VObject
    deleteVObject(vcal);
  }

  return event;
}

Todo *DndFactory::createDropTodo(QDropEvent *de)
{
  kdDebug(5800) << "VCalFormat::createDropTodo()" << endl;

  VObject *vcal;
  Todo *event = 0;

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
      kdDebug(5800) << "VCalFormat::createDropTodo(): Got event instead of todo." << endl;
    } else if (strcmp(vObjectName(curvo), VCTodoProp) == 0) {
      event = VTodoToEvent(curvo);
    } else {
      kdDebug(5800) << "VCalFormat::createDropTodo(): Unknown event type in drop." << endl;
    }
    // get rid of temporary VObject
    deleteVObject(vcal);
  }

  return event;
}


void DndFactory::cutEvent(Event *selectedEv)
{
  if (copyEvent(selectedEv)) {
    mCalendar->deleteEvent(selectedEv);
  }
}

bool DndFactory::copyEvent(Event *selectedEv)
{
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
}

Event *DndFactory::pasteEvent(const QDate &newDate, const QTime *newTime)
{
  VObject *vcal, *curVO, *curVOProp;
  VObjectIterator i;
  int daysOffset;

  Event *anEvent = 0L;

  QClipboard *cb = QApplication::clipboard();
  int bufsize;
  const char * buf;
  buf = cb->text().local8Bit();  // vCalendar object is in latin1
				//lukas: yeah, so why not screw it :(
  bufsize = strlen(buf);

  if (!VCalDrag::decode(cb->data(),&vcal)) {
    kdDebug() << "An error has occurred parsing the contents of the clipboard."
                 "You can only paste a valid vCalendar into " << application()
              << endl;
    return 0;
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
      if (mCalendar->getEvent(anEvent->uid()))
	anEvent->setUid(uidStr);

      daysOffset = anEvent->dtEnd().date().dayOfYear() -
	anEvent->dtStart().date().dayOfYear();

      if (newTime)
	anEvent->setDtStart(QDateTime(newDate, *newTime));
      else
	anEvent->setDtStart(QDateTime(newDate, anEvent->dtStart().time()));

      anEvent->setDtEnd(QDateTime(newDate.addDays(daysOffset),
				  anEvent->dtEnd().time()));
      mCalendar->addEvent(anEvent);
    } else {
      kdDebug(5800) << "found a VEvent with no DTSTART/DTEND! Skipping" << endl;
    }
  } else if (strcmp(vObjectName(curVO), VCTodoProp) == 0) {
    kdDebug(5800) << "Trying to paste a Todo." << endl;
// TODO: check, if todos can be pasted
//    Todo *aTodo = VTodoToEvent(curVO);
//    mCalendar->addTodo(aTodo);
  } else {
    kdDebug(5800) << "unknown event type in paste!!!" << endl;
  }
  // get rid of temporary VObject
  deleteVObject(vcal);
  return anEvent;
}
