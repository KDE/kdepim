/* todo-conduit.cc  Todo-Conduit for syncing KPilot and KOrganizer
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
** Copyright (C) 2001 Cornelius Schumacher
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

#include "options.h"

#if QT_VERSION < 300
#include <qmsgbox.h>
#else
#include <qmessagebox.h>
#endif

#include <kconfig.h>

#if KDE_VERSION < 300
#include <libkcal/todo.h>
#else
#include <todo.h>
#endif

#include "pilotRecord.h"
#include "pilotSerialDatabase.h"
#include "pilotTodoEntry.h"

#include "todo-factory.h"
#include "todo-conduit.h"

using namespace KCal;

static const char *todo_conduit_id = "$Id$";



TodoConduit::TodoConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &l) :
	VCalBaseConduit(d,n,l)
{
	FUNCTIONSETUP;
}


TodoConduit::~TodoConduit()
{
}


void TodoConduit::doBackup()
{
	FUNCTIONSETUP;

	if (!getCalendar(ToDoConduitFactory::group))
	{
		noCalendarError(i18n("ToDo Conduit"));
		return;
	}

	int index = 0;
	PilotRecord *rec = fDatabase->readRecordByIndex(index++);

   // Get ALL entries from Pilot
   while(rec) {
     if(rec->isDeleted()) { // tagged for deletion
       deleteRecord(rec);
//       deleteVObject(rec, VCTodoProp);
     } else {
       updateTodo(rec);
//       updateVObject(rec);
     }
     delete rec;
     rec = fDatabase->readRecordByIndex(index++);
   }

	// save the todos
	saveVCal();

	// clear the "first time" flag
	setFirstTime(fConfig, false);
}

void TodoConduit::doSync()
{
  FUNCTIONSETUP;
  PilotRecord* rec;

	if (!getCalendar(ToDoConduitFactory::group))
	{
		noCalendarError(i18n("ToDo Conduit"));
		return;
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Pilot -> Desktop" << endl;
#endif

	rec = fDatabase->readNextModifiedRec();

	// get only MODIFIED entries from Pilot, compared with the above (doBackup),
	// which gets ALL entries
	while (rec)
	{
		if(rec->isDeleted()) deleteRecord(rec);
		else
		{
			bool pilotRecModified = (rec->getAttrib() & dlpRecAttrDirty);
			if (pilotRecModified)
			{
				updateTodo(rec);
			}
			else
			{
#ifdef DEBUG
				DEBUGCONDUIT << fname
					<< ": Asked for a modified record and got an unmodified one."
					<< endl;
#endif
			}
		}

		delete rec;
		rec = fDatabase->readNextModifiedRec();
	}

  // now, all the stuff that was modified/new on the pilot should be
  // added to the todoendar.  We now need to add stuff to the pilot
  // that is modified/new in the todoendar (the opposite).
  DEBUGCONDUIT << fname << ": Desktop -> Pilot" << endl;
  doLocalSync();

  // now we save the todoendar.
  saveVCal();

  // clear the "first time" flag
  setFirstTime(fConfig, false);
}


/*
 * Given a pilot record, check to see what needs to be done to the
 * analogous vobject to bring things into sync.
 */
void TodoConduit::updateTodo(PilotRecord *rec)
{
  PilotTodoEntry todoEntry(rec);

  Todo *vtodo=findTodo(rec->getID());
  if (!vtodo) {
    // no event was found, so we need to add one with some initial info
    vtodo = new Todo;
    calendar()->addTodo(vtodo);

    vtodo->setPilotId(todoEntry.getID());
    vtodo->setSyncStatus(Incidence::SYNCNONE);
  }

  // we don't want to modify the vobject with pilot info, because it has
  // already been  modified on the desktop.  The VObject's modified state
  // overrides the PilotRec's modified state.

  if (vtodo->syncStatus() != Incidence::SYNCNONE) return;

  // otherwise, the vObject hasn't been touched.  Updated it with the
  // info from the PilotRec.

  if (todoEntry.getIndefinite()) {
    vtodo->setHasDueDate(false);
  } else {
    vtodo->setDtDue(readTm(todoEntry.getDueDate()));
  }

  // PRIORITY //
  vtodo->setPriority(todoEntry.getPriority());

  // COMPLETED? //
  vtodo->setCompleted(todoEntry.getComplete());

  setSummary(vtodo, todoEntry.getDescription());
  setNote(vtodo, todoEntry.getNote());
  
  vtodo->setSyncStatus(Incidence::SYNCNONE);
}


void TodoConduit::doLocalSync()
{
  FUNCTIONSETUP;

  QList<Todo> todos = calendar()->getTodoList();
  
  /* go through the whole todo list.  If the event has the dirty
     (modified) flag set, make a new pilot record and add it. */
  for(Todo *todo = todos.first();todo;todo = todos.next()) {
    recordid_t id;
    PilotTodoEntry *todoEntry;
    PilotRecord *pRec;

    if (todo->syncStatus() != Incidence::SYNCNONE) {
      // The event has been modified, need to write it to the pilot.
      // After using the writeRecord method, be sure and put the returned id
      // back into the todo entry! 
      
      // we read the pilotID.

      id = todo->pilotId();

      // if id != 0, this is a modified event, otherwise it is new.
      if (id != 0) {
	  pRec = fDatabase->readRecordById(id);
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

      // update it from the iCalendar Todo.

      if (todo->hasDueDate()) {
        struct tm t = writeTm(todo->dtDue());
        todoEntry->setDueDate(t);
        todoEntry->setIndefinite(0);
      } else {
        todoEntry->setIndefinite(1);
      }

      todoEntry->setPriority(todo->priority());

      todoEntry->setComplete(todo->isCompleted());

      // what we call summary pilot calls description.
      todoEntry->setDescription(todo->summary());
	
      // what we call description pilot puts as a separate note
      todoEntry->setNote(todo->description());

      // put the pilotRec in the database...
      pRec=todoEntry->pack();
      pRec->setAttrib(todoEntry->getAttrib() & ~dlpRecAttrDirty);
      id = fDatabase->writeRecord(pRec);
      delete(todoEntry);
      delete(pRec);

      // write the id we got from writeRecord back to the vObject
      if (id > 0) {
        todo->setPilotId(id);
        todo->setSyncStatus(Incidence::SYNCNONE);
      } else {
        kdDebug() << fname
                  << "error! writeRecord returned a pilotID <= 0!"
                  << endl;
      }
    }
  }

	bool deleteOnPilot = false;
	if (fConfig)
	{
		deleteOnPilot = fConfig->readBoolEntry(VCalBaseConduit::deleteOnPilot,false);
	}


  if (isFirstTime()) firstSyncCopy(deleteOnPilot);

  if (deleteOnPilot) deleteFromPilot(VCalBaseConduit::TypeTodo);
}


void TodoConduit::firstSyncCopy(bool DeleteOnPilot)
{
	FUNCTIONSETUP;

  bool insertall = false, skipall = false;

  // Get all entries from Pilot
  PilotRecord *rec;
  int index = 0;
  while ((rec = fDatabase->readRecordByIndex(index++)) != 0) {
    PilotTodoEntry *todoEntry = new PilotTodoEntry(rec);

    if (!todoEntry) {
      kdError(CONDUIT_AREA) << k_funcinfo
			    << ": Conversion to PilotTodoEntry failed"
			    << endl;
      continue;
    }

    Todo *todo = findTodo(rec->getID());
    if (!todo) {
#ifdef DEBUG
		DEBUGCONDUIT << fname
			<< ": Entry found on pilot but not in vcalendar."
			<< endl;
#endif

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
			( DeleteOnPilot ? i18n("&Delete") : i18n("&Skip") ),
			i18n("Insert &All"),
			false);

#ifdef DEBUG
	DEBUGCONDUIT << fname
		     << ": Event disposition "
		     << response
		     << endl;
#endif

	switch (response) {
          case 0:
          default: 
	    // Default is to insert this single entry and ask again later
            updateTodo(rec);
            break;
          case 1:
            // Just skip this, it will be deleted by deleteFromPilot().
            break;
          case 2:
            insertall = true;
            skipall = false;
            updateTodo(rec);
            break;
	}
      } else if (insertall) {
        // all records are to be inserted.
        updateTodo(rec);
      }
    }
    delete rec;
  }
}

/* virtual */ void TodoConduit::exec()
{
	FUNCTIONSETUP;

	if (!fConfig) return;

	KConfigGroupSaver cfgs(fConfig,ToDoConduitFactory::group);
	fDatabase = new PilotSerialDatabase(pilotSocket(),
		"ToDoDB",
		this,
		"ToDoDB");
	if (!fDatabase || !fDatabase->isDBOpen())
	{
		kdWarning() << k_funcinfo
			<< ": Couldn't open database."
			<< endl;
		KPILOT_DELETE(fDatabase);
		emit syncDone(this);
		return;
	}

	if (isTest())
	{
		// Nothing: the todo conduit doesn't have a test mode.
	}
	else if (isBackup())
	{
		doBackup();
		fDatabase->resetSyncFlags();
	}
	else
	{
		doSync();
		fDatabase->resetSyncFlags();
	}

	KPILOT_DELETE(fDatabase);
	emit syncDone(this);
	return;
}

// $Log$
// Revision 1.6  2001/06/18 19:51:40  cschumac
// Fixed todo and datebook conduits to cope with KOrganizers iCalendar format.
// They use libkcal now.
//
// Revision 1.5  2001/06/05 22:58:40  adridg
// General rewrite, cleanup thx. Philipp Hullmann
//
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
