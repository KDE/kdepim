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

#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

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


#ifndef __VCC_H__
#include "vcc.h"
#endif

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


int main(int argc, char* argv[])
{
  ConduitApp a(argc, argv, "todo_conduit",
  	"ToDo-list conduit",
	KPILOT_VERSION);
  a.addAuthor("Preston Brown",I18N_NOOP("Organizer author"));
	a.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"adridg@cs.kun.nl");
	a.addAuthor("Philipp Hullmann",
		I18N_NOOP("Bugfixer"));
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

   if (!getCalendar(TodoSetup::TodoGroup)) {
     noCalendarError(i18n("ToDo Conduit"));
     exit(ConduitMisconfigured);
   }

   rec = readRecordByIndex(index++);

  // Get ALL entries from Pilot
   while(rec) {
     if(rec->getAttrib() & dlpRecAttrDeleted) { // tagged for deletion
       deleteVObject(rec, VCTodoProp);
     } else {
       updateVObject(rec);
     }    
     delete rec;
     rec = readRecordByIndex(index++);
   }
   // save the todoendar
   saveVCal();

   // clear the "first time" flag
  KConfig& config = KPilotConfig::getConfig(TodoSetup::TodoGroup);
  setFirstTime(config, false);
}

void TodoConduit::doSync()
{
	FUNCTIONSETUP;
   PilotRecord* rec;

   if (!getCalendar(TodoSetup::TodoGroup)) {
     noCalendarError(i18n("ToDo Conduit"));
     exit(ConduitMisconfigured);
   }

   DEBUGCONDUIT << fname << ": Pilot -> Desktop" << endl;

   rec = readNextModifiedRecord();

   // get only MODIFIED entries from Pilot, compared with the above (doBackup),
   // which gets ALL entries
   while (rec) {
     if(rec->getAttrib() & dlpRecAttrDeleted)
       //	 recordDeleted(rec);
       deleteVObject(rec, VCTodoProp);
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
   DEBUGCONDUIT << fname << ": Desktop -> Pilot" << endl;
   doLocalSync();

   // now we save the todoendar.
   saveVCal();

   // clear the "first time" flag
  KConfig& config = KPilotConfig::getConfig(TodoSetup::TodoGroup);
  setFirstTime(config, false);
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
  PilotTodoEntry todoEntry(rec);
  
  vtodo=findEntryInCalendar(rec->getID(), VCTodoProp);
  if (!vtodo) {
    // no event was found, so we need to add one with some initial info
    QString numStr;
    vtodo = addProp(calendar(), VCTodoProp);
    addDateProperty(vtodo, VCDCreatedProp, todaysDate);
    numStr.sprintf("KPilot - %ld",rec->getID());
    addPropValue(vtodo, VCUniqueStringProp, numStr.latin1());
    addPropValue(vtodo, VCSequenceProp, "1");
    addDateProperty(vtodo, VCLastModifiedProp, todaysDate);
    addPropValue(vtodo, VCPriorityProp, "0");
    addPropValue(vtodo, KPilotIdProp,
		 numStr.setNum(todoEntry.getID()).latin1()); 
    addPropValue(vtodo, KPilotStatusProp, "0");
  }

  if (getStatus(vtodo)) {
    // we don't want to modify the vobject with pilot info, because it has
    // already been  modified on the desktop.  The VObject's modified state
    // overrides the PilotRec's modified state.
    return;
  }

  // otherwise, the vObject hasn't been touched.  Updated it with the
  // info from the PilotRec.
  
	// DUE DATE //
	vo = isAPropertyOf(vtodo, VCDueProp);
	if (todoEntry.getIndefinite()) 
	{ 
		// there is no due date, remove it if already present.
		//
		//
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
			setDateProperty(vo, todoEntry.getDueDate_p());
		}
		else
		{
			addDateProperty(vtodo, VCDueProp,
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
    setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.latin1(), 0));
  else
    addPropValue(vtodo, VCStatusProp, tmpStr.latin1());

  setSummary(vtodo, todoEntry.getDescription());
  setNote(vtodo, todoEntry.getNote());
  setStatus(vtodo, 0);
}

/*****************************************************************************/


void TodoConduit::doLocalSync()
{
	FUNCTIONSETUP;

  VObjectIterator i;
  
  initPropIterator(&i, calendar());
  
  /* go through the whole todo list.  If the event has the dirty
     (modified) flag set, make a new pilot record and add it. */
  while (moreIteration(&i)) {
    recordid_t id;
    PilotTodoEntry *todoEntry;
    PilotRecord *pRec;

    VObject *vtodo = nextVObject(&i);
    if (strcmp(vObjectName(vtodo), VCTodoProp) == 0 &&
	getStatus(vtodo)) {
      /* the event has been modified, need to write it to the pilot
	 After using the writeRecord method, be sure and put the returned id
	 back into the todo entry! */
      
      // we read the pilotID.

      id = getRecordID(vtodo);

	// if id != 0, this is a modified event, otherwise it is new.
	if (id != 0) {
	  pRec = readRecordById(id);
	  /* If this fails, somehow the record got deleted from the
	     pilot but we were never informed! bad pilot. naughty
	     pilot.  Just crashing ain't right, though. The Right
	     Thing is to re-create the record. */

	  if (pRec)
	    todoEntry = new PilotTodoEntry(pRec);
	  else {
	    todoEntry = new PilotTodoEntry();
	    id = 0;
	  }
	} else {
	  todoEntry = new PilotTodoEntry();
	}
	
	// update it from the vObject.
	
	// END TIME (DUE DATE) //
	VObject *vo;
	if ((vo = isAPropertyOf(vtodo, VCDueProp)) != 0L) {
	  char *s = fakeCString(vObjectUStringZValue(vo));
	  struct tm due = ISOToTm(QString(s));
	  DEBUGCONDUIT << fname <<  ": Due Date: " << s << endl;
	  deleteStr(s);
	  todoEntry->setDueDate(due);
	  todoEntry->setIndefinite(0);
	} else {
	  // indefinite event
	  DEBUGCONDUIT << fname << ": Indefinite event.\n";
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
	// what we call summary pilot calls description.
	todoEntry->setDescription(getSummary(vtodo));
	
	// DESCRIPTION //
	// what we call description pilot puts as a separate note
	todoEntry->setNote(getDescription(vtodo));

	// put the pilotRec in the database...
	pRec=todoEntry->pack();
	pRec->setAttrib(todoEntry->getAttrib() & ~dlpRecAttrDirty);
	id = writeRecord(pRec);
	delete(todoEntry);
	delete(pRec);

	// write the id we got from writeRecord back to the vObject
	if (id > 0) {
	  setNumProperty(vtodo, KPilotIdProp, id);
	  setNumProperty(vtodo, KPilotStatusProp, 0);
	} else {
		kdDebug() << fname
			<< "error! writeRecord returned a pilotID <= 0!"
			<< endl;
	}
      }
   }

  KConfig& config = KPilotConfig::getConfig(TodoSetup::TodoGroup);
  bool DeleteOnPilot = config.readBoolEntry("DeleteOnPilot", true);

  if (firstTime())
    firstSyncCopy(DeleteOnPilot);

  if (DeleteOnPilot)
    deleteFromPilot(VCTodoProp);
}


//////////////////////////////////////////////////////////////////////////

void TodoConduit::firstSyncCopy(bool DeleteOnPilot) {
  FUNCTIONSETUP;

  bool insertall = false, skipall = false;

  // Get all entries from Pilot
  PilotRecord *rec;
  int index = 0;
  while ((rec = readRecordByIndex(index++)) != 0) {
    PilotTodoEntry *todoEntry = new PilotTodoEntry(rec);
      
    if (!todoEntry) {
      kdError(CONDUIT_AREA) << __FUNCTION__
			    << ": Conversion to PilotTodoEntry failed"
			    << endl;
      continue;
    }
      
    VObject *vevent = findEntryInCalendar(rec->getID(),
					  VCTodoProp);
    if (vevent == 0L) {
      DEBUGCONDUIT << __FUNCTION__
		   << ": Entry found on pilot but not in vcalendar."
		   << endl;
	
      // First hot-sync, ask user how to treat this event.
      if (!insertall && !skipall) {
	DEBUGCONDUIT << __FUNCTION__
		     << ": Questioning event disposition."
		     << endl;
	    
	QString text = i18n("This is the first time that "
			    "you have done a HotSync\n"
			    "with the vCalendar conduit. "
			    "There is a to-do item\n"
			    "in the PalmPilot which is not "
			    "in the vCalendar (KOrganizer).\n\n");
	text += i18n("Item: %1.\n\n"
		     "What must be done with this item?")
	  .arg(todoEntry->getDescription());
	    
	int response =
	  QMessageBox::information(0, 
				   i18n("KPilot To-Do Conduit"), 
				   text, 
				   i18n("&Insert"), 
				   DeleteOnPilot ? i18n("&Delete") 
				   : i18n("&Skip"),
				   i18n("Insert &All"),
				   false);
	    
	DEBUGCONDUIT << __FUNCTION__ 
		     << ": Event disposition "
		     << response
		     << endl;
	    
	switch (response) {
	case 0:
	default: 
	  /* Default is to insert this single entry and ask again
	     later. */
	  updateVObject(rec);
	  break;
	case 1:
	  // Just skip this, it will be deleted by deleteFromPilot().
	  break;
	case 2:
	  insertall = true;
	  skipall = false;
	  updateVObject(rec);
	  break;
	} // switch (response)
      } else if (insertall) {
	// all records are to be inserted.
	updateVObject(rec);
      }
    } // if (!vevent)
    delete rec;
  } // while ((rec = readRecordByIndex(index++)) != 0)
} // void TodoConduit::firstSyncCopy()


/* put up the about / setup dialog. */
QWidget* TodoConduit::aboutAndSetup()
{
  return new TodoSetup();
}

// $Log$
// Revision 1.4  2001/05/07 20:09:32  adridg
// Phillipp's due-date patches
//
// Revision 1.3  2001/04/23 21:26:02  adridg
// Some testing and i18n() fixups, 8-bit char fixes
//
// Revision 1.2  2001/04/23 06:29:30  adridg
// Patches for bug #23385 and probably #23289
//
// Revision 1.1  2001/04/16 13:36:20  adridg
// Moved todoconduit
//
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
