/* todo-conduit.cc			KPilot
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
**
** This file is part of the todo conduit, a conduit for KPilot that
** synchronises the Pilot's todo application with the outside world,
** which currently means KOrganizer.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
// I have noticed that this is full of memory leaks, but since it is
// short lived, it shouldn't matter so much. -- PGB

#include "options.h"

#include <sys/types.h>
#include <signal.h>
#include <iostream.h>
#include <stdlib.h>
#include <time.h>

#ifndef QBITARRAY_H
#include <qbitarray.h>
#endif

#ifndef QDIR_H
#include <qdir.h>
#endif

#ifndef QDATETM_H
#include <qdatetm.h>
#endif

#ifndef QSTRING_H
#include <qstring.h>
#endif

#ifndef QMSGBOX_H
#include <qmsgbox.h>
#endif


#ifndef _KAPP_H
#include <kapp.h>
#endif

#ifndef _KMESSAGEBOX_H
#include <kmessagebox.h>
#endif

#ifndef _KCONFIG_H
#include <kconfig.h>
#endif

#ifndef _KDEBUG_H
#include <kdebug.h>
#endif


#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif

#ifndef _KPILOT_PILOTDATABASE_H
#include "pilotDatabase.h"
#endif

#ifndef _KPILOT_PILOTRECORD_H
#include "pilotRecord.h"
#endif

#ifndef _KPILOT_PILOTTODOENTRY_H
#include "pilotTodoEntry.h"
#endif


#include "vcc.h"

#ifndef _KPILOT_TODO_CONDUIT_H
#include "todo-conduit.h"
#endif

#ifndef _KPILOT_TODO_SETUP_H
#include "todo-setup.h"
#endif

#ifndef _KPILOT_CONDUITAPP_H
#include "conduitApp.h"
#endif

static const char *todo_conduit_id = "$Id$";


// globals
bool first = TRUE;

int main(int argc, char* argv[])
{
  ConduitApp a(argc, argv, "todo_conduit",
  	"ToDo-list conduit",
	KPILOT_VERSION);
  a.addAuthor("Preston Brown",I18N_NOOP("Organizer author"));
  TodoConduit conduit(a.getMode());
  a.setConduit(&conduit);
  return a.exec();

	/* NOTREACHED */
	(void) todo_conduit_id;
}


TodoConduit::TodoConduit(eConduitMode mode)
  : VCalBaseConduit(mode)
{
	FUNCTIONSETUP;
}


TodoConduit::~TodoConduit()
{
}


/* static */ const char *TodoConduit::version()
{
	return "ToDo Conduit " KPILOT_VERSION;
}


void TodoConduit::doBackup()
{
   PilotRecord* rec;
   int index = 0;

	getCalendar(TodoSetup::TodoGroup);

   rec = readRecordByIndex(index++);

  // Get ALL entries from Pilot
   while(rec) {
     if(rec->getAttrib() & dlpRecAttrDeleted) { // tagged for deletion
       deleteVObject(rec);
     } else {
       updateVObject(rec);
     }    
     delete rec;
     rec = readRecordByIndex(index++);
   }
   // save the todoendar
   saveVCal();
}

void TodoConduit::doSync()
{
	FUNCTIONSETUP;
   PilotRecord* rec;

	getCalendar(TodoSetup::TodoGroup);
   rec = readNextModifiedRecord();

   // get only MODIFIED entries from Pilot, compared with the above (doBackup),
   // which gets ALL entries
   while (rec) {
     if(rec->getAttrib() & dlpRecAttrDeleted)
       //	 recordDeleted(rec);
       deleteVObject(rec);
     else {
       bool pilotRecModified = (rec->getAttrib() & dlpRecAttrDirty);
       if (pilotRecModified)
	{
	 updateVObject(rec);
	}
       else {
		DEBUGCONDUIT << fname
			<< ": Asked for a modified record and got "
			   "an unmodified one."
			<< endl;
       }
     }
	 
     delete rec;
     rec = readNextModifiedRecord();
   }

   // now, all the stuff that was modified/new on the pilot should be
   // added to the todoendar.  We now need to add stuff to the pilot
   // that is modified/new in the todoendar (the opposite).	  
   doLocalSync();

   // now we save the todoendar.
   saveVCal();
}


/*****************************************************************************/

/*
 * Given a pilot record, check to see what needs to be done to the
 * analogous vobject to bring things into sync.
 */
void TodoConduit::updateVObject(PilotRecord *rec)
{
	FUNCTIONSETUP;

  VObject *vtodo;
  VObject *vo;
  QDateTime todaysDate = QDateTime::currentDateTime();
  QString tmpStr;
  QString numStr;
  PilotTodoEntry todoEntry(rec);
  
  vtodo=findEntryInCalendar(rec->getID());
  if (!vtodo) {
    // no event was found, so we need to add one with some initial info
    vtodo = addProp(calendar(), VCTodoProp);

	addDateProperty(vtodo, VCDCreatedProp, todaysDate);

    numStr.sprintf("KPilot - %d",rec->getID());
    addPropValue(vtodo, VCUniqueStringProp, numStr.latin1());
    addPropValue(vtodo, VCSequenceProp, "1");
	addDateProperty(vtodo, VCLastModifiedProp, todaysDate);
    addPropValue(vtodo, VCPriorityProp, "0");
    addPropValue(vtodo, KPilotIdProp, numStr.setNum(todoEntry.getID()).latin1());
    addPropValue(vtodo, KPilotStatusProp, "0");

  }

  // determine whether the vobject has been modified since the last sync
  vo = isAPropertyOf(vtodo, KPilotStatusProp);
  bool todoRecModified = 0;
  if (vo)
    todoRecModified = (atol(fakeCString(vObjectUStringZValue(vo))) == 1);
  
  if (todoRecModified) {
    // we don't want to modify the vobject with pilot info, because it has
    // already been  modified on the desktop.  The VObject's modified state
    // overrides the PilotRec's modified state.
    return;
  }
  // otherwise, the vObject hasn't been touched.  Updated it with the
  // info from the PilotRec.
  
  // END TIME //
  vo = isAPropertyOf(vtodo, VCDTendProp);
	if (todoEntry.getIndefinite()) 
	{ // there is no end date
		if (vo)
		{
		      addProp(vo, KPilotSkipProp);
		}

		DEBUGCONDUIT << fname
			<< ": Todo-item with no end date."
			<< endl;
	}
	else 
	{
		if (vo)
		{
			setDateProperty(vo,todoEntry.getDueDate_p());
		}
		else
		{
			addDateProperty(vtodo, VCDTendProp,
				todoEntry.getDueDate_p());
		}
	}
  
  // PRIORITY //
  vo = isAPropertyOf(vtodo, VCPriorityProp);
  int priority = todoEntry.getPriority();
  tmpStr.setNum(priority);
  if (vo)
    setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.latin1(), 0));
  else
    addPropValue(vtodo, VCPriorityProp, tmpStr.latin1());

  // COMPLETED? //
  vo = isAPropertyOf(vtodo, VCStatusProp);
  tmpStr = (todoEntry.getComplete() ? "COMPLETED" : "X-ACTION");
  if (vo)
	{
    setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.latin1(), 0));
	}
  else
	{
    addPropValue(vtodo, VCStatusProp, tmpStr.latin1());
	}

	setSummary(vtodo,todoEntry.getDescription());
	setNote(vtodo,todoEntry.getNote());
	setStatus(vtodo,0);
}

/*****************************************************************************/


void TodoConduit::doLocalSync()
{
	FUNCTIONSETUP;

  VObjectIterator i;
  VObject *vtodo = 0L;
  VObject *vo;
  char *s;
  int status;
  recordid_t id;
  PilotRecord *pRec;
  PilotTodoEntry *todoEntry;
  fTimeZone = getTimeZone();
  
  initPropIterator(&i, calendar());
  
  // go through the whole todo list.  If the event has the dirty (modified)
  // flag set, make a new pilot record and add it.
  // we only take events that have KPilotStatusProp as a property.  If
  // this property isn't present, ignore the event.
  while (moreIteration(&i)) {
    vtodo = nextVObject(&i);
    vo = isAPropertyOf(vtodo, KPilotStatusProp);
    
    if (vo && (strcmp(vObjectName(vtodo), VCTodoProp) == 0)) {
      status = 0;
      status = atoi(s = fakeCString(vObjectUStringZValue(vo)));
      deleteStr(s);
      
      if (status == 1) {
	// the event has been modified, need to write it to the pilot
	// After using the writeRecord method, be sure and put the returned id
	// back into the todo entry!
	
	// we read the pilotID.
	
	vo = isAPropertyOf(vtodo, KPilotIdProp);
	id = atoi(s = fakeCString(vObjectUStringZValue(vo)));
	deleteStr(s);
	
	// if id != 0, this is a modified event,
	// otherwise it is new.
	if (id != 0) {
	  pRec = readRecordById(id);
	  // if this fails, somehow the record got deleted from the pilot
	  // but we were never informed! bad pilot. naughty pilot.
	  ASSERT(pRec);
	  
	  todoEntry = new PilotTodoEntry(pRec);
	} else {
	  todoEntry = new PilotTodoEntry();
	}
	
	// update it from the vObject.
	
	// END TIME (DUE DATE) //
	if ((vo = isAPropertyOf(vtodo, VCDTendProp)) != 0L) {
	  char *s = fakeCString(vObjectUStringZValue(vo));
	  struct tm due = ISOToTm(QString(s));
	  deleteStr(s);
	  todoEntry->setDueDate(due);
	  todoEntry->setIndefinite(0);
	} else {
	  // indefinite event
	  todoEntry->setIndefinite(1);
	}

	// PRIORITY //
	if ((vo = isAPropertyOf(vtodo, VCPriorityProp)) != 0L) {
	  int priority = atoi(fakeCString(vObjectUStringZValue(vo)));
	  if (priority == 0)
	    priority = 1; // no 'undefined' priorities on the pilot
	  todoEntry->setPriority(priority);
	} else {
	  todoEntry->setPriority(1); // todo needs a priority.
	}

	// COMPLETE? //
	if ((vo = isAPropertyOf(vtodo, VCStatusProp)) != 0L) {
	  char *s = fakeCString(vObjectUStringZValue(vo));
	  if (strcmp(s, "COMPLETED") == 0)
	    todoEntry->setComplete(1);
	  else
	    todoEntry->setComplete(0);
	  deleteStr(s);
	} else {
	  // needs complete status even if none given
	  todoEntry->setComplete(0);
	}

	// SUMMARY //
	char *s2=0, *s3=0;
	// what we call summary pilot calls description.
	if ((vo = isAPropertyOf(vtodo, VCSummaryProp)) != 0L) {
	  s2 = fakeCString(vObjectUStringZValue(vo));
	  todoEntry->setDescription(s2);
	} else {
	  s2 = (char *) malloc(2);
	  s2[0] = 0;
	  todoEntry->setDescription(s2);
	}
	
	// DESCRIPTION //
	// what we call description pilot puts as a separate note
	if ((vo = isAPropertyOf(vtodo, VCDescriptionProp)) != 0L) {
	  s3 = fakeCString(vObjectUStringZValue(vo));
	  todoEntry->setNote(s3);
	} 

	// put the pilotRec in the database...
	pRec=todoEntry->pack();
	pRec->setAttrib(todoEntry->getAttrib() & ~dlpRecAttrDirty);
	id = writeRecord(pRec);
	delete(todoEntry);
	delete(pRec);
	deleteStr(s2);
	if (s3)
	  deleteStr(s3);

	// write the id we got from writeRecord back to the vObject
	if (id > 0) {
	  QString tmpStr;
	  tmpStr.setNum(id);
	  vo = isAPropertyOf(vtodo, KPilotIdProp);
	  // give it an id.
	  setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.latin1(), 0));
	  vo = isAPropertyOf(vtodo, KPilotStatusProp);
	  tmpStr = "0"; // no longer a modified event.
	  setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.latin1(), 0));
	} else {
		kdDebug() << fname
			<< "error! writeRecord returned a pilotID <= 0!"
			<< endl;
	}
       }
     }
   }
   // anything that is left on the pilot but that is not found in the
   // todo list has to be deleted.  We know this because we have added
   // everything to the todo list from the pilot that had the modified
   // flag set, and we have added everything to the pilot from the todo list
   // that had the modified flag set.
   //
   // insertall flag will be set in the future in the config file 
   // so that this behaviour can be overridden.
   PilotRecord *rec;
   int index=0, response, insertall=0;
   QList<int> deletedList;
   deletedList.setAutoDelete(TRUE);

   rec = readRecordByIndex(index++);
   //  Get all entries from Pilot
   while (rec) {
     todoEntry = new PilotTodoEntry(rec); // convert to date structure
     vtodo = findEntryInCalendar(rec->getID());
     
     if (vtodo == 0L) {
       if (first == FALSE) {
	 deletedList.append(new int(rec->getID()));
       } else { 
	 if (insertall == 0) {
	   QString text;
	   text = "This is the first time than you have done a HotSync\n"
	     "with the To Do conduit. There is an To Do entry\n"
	     "in the PalmPilot which is not in the vCalendar (KOrganizer).\n\n";
	   text += "To Do: ";
	   text += todoEntry->getDescription();
	   text += "\n\nWhat must be done with this appointment?";

	   response = QMessageBox::information(0, "KPilot Todo List Conduit",
					       text.latin1(), "&Insert","&Delete",
					       "Insert &All",0);
	   switch(response) {
	   case 1:
	     deletedList.append(new int(rec->getID()));
	     break;
	   case 2:
	     insertall = 1;
	     updateVObject(rec);
	     break;
	   case 0:
	   default:
	     updateVObject(rec);
	     break;
	   }
	 } else {
	   // all records are to be inserted.
	   updateVObject(rec);
	 }
       }
     }
     delete rec;
     rec = readRecordByIndex(index++);
   }

   for (int *j = deletedList.first(); j; j = deletedList.next()) {
     rec = readRecordById(*j);
     rec->setAttrib(~dlpRecAttrDeleted);
     writeRecord(rec);
     delete rec;
   }
   deletedList.clear();

	KConfig& config = KPilotConfig::getConfig(TodoSetup::TodoGroup);
	setFirstTime(config,false);
}


/* put up the about / setup dialog. */
QWidget* TodoConduit::aboutAndSetup()
{
  return new TodoSetup();
}

// $Log$
// Revision 1.18  2001/04/01 17:32:05  adridg
// Fiddling around with date properties
//
// Revision 1.17  2001/03/24 16:11:06  adridg
// Fixup some date-to-vcs functions
//
// Revision 1.16  2001/03/10 18:26:04  adridg
// Refactored vcal conduit and todo conduit
//
// Revision 1.15  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.14  2001/03/05 23:57:53  adridg
// Added KPILOT_VERSION
//
// Revision 1.13  2001/03/04 13:46:49  adridg
// struct tm woes
//
// Revision 1.12  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.11  2001/02/07 15:46:32  adridg
// Updated copyright headers for source release. Added CVS log. No code change.
//
