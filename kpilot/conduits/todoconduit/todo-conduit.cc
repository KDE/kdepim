// Conduit for KPilot <--> KOrganizer for Todo Items
// (c) 1998, 1999, 2000 Preston Brown

// I have noticed that this is full of memory leaks, but since it is
// short lived, it shouldn't matter so much. -- PGB

#include <sys/types.h>
#include <signal.h>
#include <iostream.h>
#include <stdlib.h>
#include <time.h>
#include <qbitarray.h>
#include <qdir.h>
#include <qdatetm.h>
#include <qstring.h>
#include <kapp.h>
#include <qmsgbox.h>
#include <kconfig.h>

#include "kpilotlink.h"
#include "pilotDatabase.h"
#include "pilotRecord.h"
#include "pilotTodoEntry.h"

#include "todo-conduit.h"
#include "todo-setup.h"
#include "conduitApp.h"

#include "options.h"

// globals
bool first = TRUE;

FILE *logfile;

int main(int argc, char* argv[])
{
  QString fileName;
  fileName += tmpnam(0L);
  fileName += "-todoconduit.log";
  logfile = fopen(fileName.latin1(), "w+");
  fprintf(logfile, "todoconduit log file opened for writing\n");
  fflush(logfile);
  ConduitApp a(argc, argv, "todo_conduit",
  	"\t\ttodo_conduit -- A conduit for KPilot\n");
  TodoConduit conduit(a.getMode());
  a.setConduit(&conduit);
  return a.exec();
}


TodoConduit::TodoConduit(eConduitMode mode)
  : BaseConduit(mode)
{
  fCalendar = 0L;
  KConfig* config = KPilotLink::getConfig(TodoSetup::TodoGroup);

  QString calName = config->readEntry("CalFile");
  first = config->readBoolEntry("FirstTime", TRUE);

  if ((fMode == BaseConduit::HotSync) || (fMode == BaseConduit::Backup)) {
    fCalendar = Parse_MIME_FromFileName((char*)calName.latin1());

    if (fCalendar == 0L) {
      QString message;
      message.sprintf("The TodoConduit could not open the file %s.\n "
		      "Please configure the conduit with the correct "
		      "filename and try again",calName.latin1());
      QMessageBox::critical(0, "KPilot Todo Conduit Fatal Error",
			    message.latin1());
      exit(-1);
    }
  }
}

TodoConduit::~TodoConduit()
{
  if (fCalendar) {
    cleanVObject(fCalendar);
    cleanStrTbl();
  }
  fprintf(logfile,"---------------------------\n");
  fflush(logfile);
  fclose(logfile);

}


/* static */ const char *TodoConduit::version()
{
	return "ToDo Conduit v2.0";
}

void TodoConduit::doBackup()
{
   PilotRecord* rec;
   int index = 0;

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
   fprintf(logfile,"saving the calendar, doBackup() finished\n");
   fflush(logfile);
   saveTodo();
}

void TodoConduit::doSync()
{
   PilotRecord* rec;

   rec = readNextModifiedRecord();

   // get only MODIFIED entries from Pilot, compared with the above (doBackup),
   // which gets ALL entries
   while (rec) {
     fprintf(logfile, "got another record in doSync\n");
     fflush(logfile);
     if(rec->getAttrib() & dlpRecAttrDeleted)
       //	 recordDeleted(rec);
       deleteVObject(rec);
     else {
       bool pilotRecModified = (rec->getAttrib() & dlpRecAttrDirty);
       if (pilotRecModified)
	 updateVObject(rec);
       else {
	 fprintf(logfile,"weird! we asked for a modified record and got one that wasn't.\n");
	 fflush(logfile);
       }
     }
	 
     delete rec;
     rec = readNextModifiedRecord();
   }

   // now, all the stuff that was modified/new on the pilot should be
   // added to the todoendar.  We now need to add stuff to the pilot
   // that is modified/new in the todoendar (the opposite).	  
   fprintf(logfile,"starting local sync\n");
   fflush(logfile);
   doLocalSync();
   fprintf(logfile,"local sync finished\n");
   fflush(logfile);

   // now we save the todoendar.
   fprintf(logfile,"saving the calendar, doSync() finished\n");
   fflush(logfile);
   saveTodo();
}


/*****************************************************************************/

/*
 * Given a pilot record, check to see what needs to be done to the
 * analogous vobject to bring things into sync.
 */
void TodoConduit::updateVObject(PilotRecord *rec)
{
  fprintf(logfile,"updating / creating a VObject\n");
  fflush(logfile);
  VObject *vtodo;
  VObject *vo;
  QDateTime todaysDate = QDateTime::currentDateTime();
  QString dateString, tmpStr;
  QString numStr;
  PilotTodoEntry todoEntry(rec);
  
  vtodo=findEntryInCalendar(rec->getID());
  if (!vtodo) {
    fprintf(logfile,"it isn't in the todo list, it's a new pilot event.\n");
    fflush(logfile);
    // no event was found, so we need to add one with some initial info
    vtodo = addProp(fCalendar, VCTodoProp);
    fprintf(logfile,"got here 1\n");
    fflush(logfile);

    dateString.sprintf("%.2d%.2d%.2dT%.2d%.2d%.2d",
			todaysDate.date().year(), todaysDate.date().month(),
		       todaysDate.date().day(), todaysDate.time().hour(),
		       todaysDate.time().minute(), todaysDate.time().second());

    addPropValue(vtodo, VCDCreatedProp, dateString.latin1());
    numStr.sprintf("KPilot - %d",rec->getID());
    addPropValue(vtodo, VCUniqueStringProp, numStr.latin1());
    addPropValue(vtodo, VCSequenceProp, "1");
    addPropValue(vtodo, VCLastModifiedProp, dateString.latin1());
    addPropValue(vtodo, VCPriorityProp, "0");
    addPropValue(vtodo, KPilotIdProp, numStr.setNum(todoEntry.getID()).latin1());
    addPropValue(vtodo, KPilotStatusProp, "0");

    fprintf(logfile,"created initial VObject\n");    
    fflush(logfile);
  } else {
    fprintf(logfile,"it is an existing VObject in the todo list.\n");
    fflush(logfile);
  }

  // determine whether the vobject has been modified since the last sync
  vo = isAPropertyOf(vtodo, KPilotStatusProp);
  bool todoRecModified = 0;
  if (vo)
    todoRecModified = (atol(fakeCString(vObjectUStringZValue(vo))) == 1);
  
  fprintf(logfile,"todoRecModified = %d\n",todoRecModified);
  fflush(logfile);
  if (todoRecModified) {
    fprintf(logfile,"damn! the vobject has been modified on both the pilot and desktop\n");
    fprintf(logfile,"skipping pilot update, desktop overrides...\n");
    fflush(logfile);
    // we don't want to modify the vobject with pilot info, because it has
    // already been  modified on the desktop.  The VObject's modified state
    // overrides the PilotRec's modified state.
    return;
  }
  // otherwise, the vObject hasn't been touched.  Updated it with the
  // info from the PilotRec.
  
  // END TIME //
  vo = isAPropertyOf(vtodo, VCDTendProp);
  if (todoEntry.getIndefinite()) { // there is no end date
    if (vo)
      addProp(vo, KPilotSkipProp);
  } else {
    dateString.sprintf("%.4d%.2d%.2dT%.2d%.2d%.2d",
		       1900 + todoEntry.getDueDate().tm_year,
		       todoEntry.getDueDate().tm_mon + 1,
		       todoEntry.getDueDate().tm_mday,
		       todoEntry.getDueDate().tm_hour,
		       todoEntry.getDueDate().tm_min,
		       todoEntry.getDueDate().tm_sec);
    if (vo)
      setVObjectUStringZValue_(vo, fakeUnicode(dateString.latin1(), 0));
    else
      addPropValue(vtodo, VCDTendProp, dateString.latin1());
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

  // SUMMARY //
  vo = isAPropertyOf(vtodo, VCSummaryProp);
  tmpStr = todoEntry.getDescription();
  // the following should take care of the multi-line summary bug.
  tmpStr = tmpStr.simplifyWhiteSpace();

  // the vCalendar parser really hates empty summaries, avoid them.
  if (!tmpStr.isEmpty()) {
    if (vo)
      setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.latin1(), 0));
    else
      addPropValue(vtodo, VCSummaryProp, tmpStr.latin1());
  }

  // DESCRIPTION (NOTE) //
  vo = isAPropertyOf(vtodo, VCDescriptionProp);
  if (todoEntry.getNote() != 0L && strlen(todoEntry.getNote()) != 0) {
    if (vo)
      setVObjectUStringZValue_(vo, fakeUnicode(todoEntry.getNote(), 0));
    else
      vo = addPropValue(vtodo, VCDescriptionProp, todoEntry.getNote());
    // if the description takes up more than one line, we need
    // to add the Quoted-Printable property.
    if (strchr(todoEntry.getNote(), '\n') &&
	!isAPropertyOf(vo, VCQuotedPrintableProp))
      addProp(vo, VCQuotedPrintableProp);
  } else {
    if (vo)
      addProp(vo, KPilotSkipProp);
  }

  // PILOT STATUS //
  vo = isAPropertyOf(vtodo, KPilotStatusProp);
  // TURN OFF MODIFIED
  if (vo) {
    int voStatus = atol(fakeCString(vObjectUStringZValue(vo)));
    if (voStatus != 0)
      fprintf(logfile,"in updateVObject there was a vobject with status %d!\n",
	      voStatus);
    setVObjectUStringZValue_(vo, fakeUnicode("0", 0));
  } else
    addPropValue(vtodo, KPilotStatusProp, "0");
  
  fprintf(logfile,"finished updateVObject\n");
  fflush(logfile);

}

/*
 * The pilot record specified was deleted on the pilot.  Remove
 * the corresponding vobject from the todoendar.
 */
void TodoConduit::deleteVObject(PilotRecord *rec)
{
  VObject *delvo;
  
  fprintf(logfile,"actually Deleting a record.\n");
  fflush(logfile);
  
  delvo = findEntryInCalendar(rec->getID());
  // if the entry was found, it is still in the todo list.  We need to
  // set the Status flag to Deleted, so that KOrganizer will not load
  // it next time the todo list is read in.  If it is not found, the
  // user has also deleted it already in the todo list, and we can
  // safely do nothing.
  if (delvo) {
    // we now use the additional 'KPilotSkip' property, instead of a special
    // value for KPilotStatusProp.
    addProp(delvo, KPilotSkipProp);
  }  
}

/*****************************************************************************/

void TodoConduit::saveTodo()
{
	FUNCTIONSETUP;

	KConfig* config = KPilotLink::getConfig(TodoSetup::TodoGroup);
	QString calName = config->readEntry("CalFile");

	if (fCalendar)
	{
		writeVObjectToFile((char*)calName.latin1(), fCalendar);  
	}
}

void TodoConduit::doLocalSync()
{
  VObjectIterator i;
  VObject *vtodo = 0L;
  VObject *vo;
  char *s;
  int status;
  recordid_t id;
  PilotRecord *pRec;
  PilotTodoEntry *todoEntry;
  timeZone = 0;
  
  vo = isAPropertyOf(fCalendar, VCTimeZoneProp);
  
  // deal with time zone offset
  if (vo) {
    bool neg = FALSE;
    int hours, minutes;
    QString tmpStr(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
    
    if (tmpStr.left(1) == "-")
      neg = TRUE;
    if (tmpStr.left(1) == "-" || tmpStr.left(1) == "+")
      tmpStr.remove(0, 1);
    hours = tmpStr.left(2).toInt();
    if (tmpStr.length() > 2)
      minutes = tmpStr.right(2).toInt();
    else
      minutes = 0;
    timeZone = (60*hours+minutes);
    if (neg)
      timeZone = -timeZone;
  }
  
  initPropIterator(&i, fCalendar);
  
  // go through the whole todo list.  If the event has the dirty (modified)
  // flag set, make a new pilot record and add it.
  // we only take events that have KPilotStatusProp as a property.  If
  // this property isn't present, ignore the event.
  while (moreIteration(&i)) {
    vtodo = nextVObject(&i);
    vo = isAPropertyOf(vtodo, KPilotStatusProp);
    
    if (vo && (strcmp(vObjectName(vtodo), VCTodoProp) == 0)) {
      fprintf(logfile, "doLocalSync() working on a new vTodo\n");
      fflush(logfile);
      
      status = 0;
      status = atoi(s = fakeCString(vObjectUStringZValue(vo)));
      deleteStr(s);
      
      if (status == 1) {
	fprintf(logfile,"found an event with status == 1, updating...\n");
	fflush(logfile);
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
	  fprintf(logfile,"the event is not new to pilot, found in pilot as ptr %p\n",
		  pRec);
	  fflush(logfile);
	  
	  todoEntry = new PilotTodoEntry(pRec);
	} else {
	  fprintf(logfile,"the event is new.\n");
	  fflush(logfile);
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
	fprintf(logfile,"wrote it to database, id is %ld\n",id);
	fflush(logfile);
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
	  fprintf(logfile,"error! writeRecord returned a pilotID <= 0!\n");
	  fflush(logfile);
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
	   case 0:
	     updateVObject(rec);
	     break;
	   case 1:
	     deletedList.append(new int(rec->getID()));
	     break;
	   case 2:
	     insertall = 1;
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
     fprintf(logfile,"deleting record %i\n",*j);
     fflush(logfile);
     rec = readRecordById(*j);
     rec->setAttrib(~dlpRecAttrDeleted);
     writeRecord(rec);
     delete rec;
   }
   deletedList.clear();
}

/*
 * Given an pilot id, search the todo list for a matching vobject, and return
 * the pointer to that object.  If not found, return NULL.
 */
VObject* TodoConduit::findEntryInCalendar(unsigned int id)
{
  VObjectIterator i;
  VObject* entry = 0L;
  VObject* objectID;
  
  initPropIterator(&i, fCalendar);
  
  // go through all the vobjects in the todo
  while (moreIteration(&i)) {
     entry = nextVObject(&i);
     objectID = isAPropertyOf(entry, KPilotIdProp);

     if (objectID && (strcmp(vObjectName(entry), VCTodoProp) == 0)) {
       if(strtoul(fakeCString(vObjectUStringZValue(objectID)), 0L, 0) == id) {
  	 return entry;
       }
     }
  }
  return 0L;
}

/* put up teh about / setup dialog. */
QWidget* TodoConduit::aboutAndSetup()
{
  return new TodoSetup();
}

QString TodoConduit::TmToISO(struct tm tm)
{
  QString dStr;
  
  dStr.sprintf("%.4d%.2d%.2dT%.2d%.2d%.2d",
	       1900 + tm.tm_year,
	       tm.tm_mon + 1,
	       tm.tm_mday,
	       tm.tm_hour,
	       tm.tm_min,
	       tm.tm_sec);
  
  return dStr;
}

struct tm TodoConduit::ISOToTm(const QString &tStr)
{
  struct tm tm;
  
  tm.tm_wday = 0; // unimplemented
  tm.tm_yday = 0; // unimplemented
  tm.tm_isdst = 0; // unimplemented
  
  sscanf(tStr.latin1(),"%04d%02d%02dT%02d%02d%02d",
	 &tm.tm_year, &tm.tm_mon,
	 &tm.tm_mday, &tm.tm_hour,
	 &tm.tm_min, &tm.tm_sec);

  // possibly correct for timeZone
  if (timeZone && (tStr.right(1) == "Z")) {
    QDateTime tmpDT;
    tmpDT.setDate(QDate(tm.tm_year, tm.tm_mon, tm.tm_mday));
    tmpDT.setTime(QTime(tm.tm_hour, tm.tm_min, tm.tm_sec));
    tmpDT = tmpDT.addSecs(60*timeZone); // correct from GMT
    tm.tm_year = tmpDT.date().year();
    tm.tm_mon = tmpDT.date().month();
    tm.tm_mday = tmpDT.date().day();
    tm.tm_hour = tmpDT.time().hour();
    tm.tm_min = tmpDT.time().minute();
    tm.tm_sec = tmpDT.time().second();
  }

  // tm_year is only since 1900
  tm.tm_year -= 1900; 
  // pilot month is 0-based.
  tm.tm_mon -= 1;

  return tm;
}

int TodoConduit::numFromDay(const QString &day)
{
  if (day == "SU ") return 0;
  if (day == "MO ") return 1;
  if (day == "TU ") return 2;
  if (day == "WE ") return 3;
  if (day == "TH ") return 4;
  if (day == "FR ") return 5;
  if (day == "SA ") return 6;
  
  return -1; // something bad happened. :)
} 

