// Conduit for KPilot <--> KOrganizer
// (c) 1998, 1999 Preston Brown, Herwin Jan Steehouwer, and Dan Pilone

// I have noticed that this is full of memory leaks, but since it is
// short lived, it shouldn't matter so much. -- PGB

// $Revision$

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

#include "kpilotlink.h"
#include "pilotDatabase.h"
#include "pilotRecord.h"
#include "pilotDateEntry.h"

#include "vcal-conduit.h"
#include "vcal-setup.h"
#include "conduitApp.h"
#include "kpilot.h"

static char *id="$Id$";


// globals
bool first = TRUE;

FILE *logfile;

int main(int argc, char* argv[])
{
#ifndef NO_DEBUG
  QString fileName;
  fileName += tmpnam(0L);
  fileName += "-vcalconduit.log";
  logfile = fopen(fileName.data(), "w+");
  fprintf(logfile, "vcalconduit log file opened for writing\n");
  fflush(logfile);
#else
  logfile = fopen("/dev/null", "w+");
#endif
  ConduitApp a(argc, argv, "vcal_conduit",
  	"\t\tvcal_conduit -- A conduit for KPilot\n");
  VCalConduit conduit(a.getMode());
  a.setConduit(&conduit);
  return a.exec();
}

VCalConduit::VCalConduit(eConduitMode mode)
  : BaseConduit(mode)
{
  fCalendar = 0L;
  KConfig* config = KPilotLink::getConfig(VCalSetup::VCalGroup);
  QString calName = config->readEntry("CalFile");
  first = config->readBoolEntry("FirstTime", TRUE);

  if ((fMode == BaseConduit::HotSync) || (fMode == BaseConduit::Backup)) {
    fCalendar = Parse_MIME_FromFileName(calName.data());

    if(fCalendar == 0L) {
      QString message(1000);
      message.sprintf("The VCalConduit could not open the file %s.\n "
		      "Please configure the conduit with the correct filename and try again",calName.data());
      QMessageBox::critical(0, "KPilot vCalendar Conduit Fatal Error",
			    message.data());                                                 
      exit(-1);
    }
  }
}

VCalConduit::~VCalConduit()
{
  if (fCalendar) {
    cleanVObject(fCalendar);
    cleanStrTbl();
  }
  fprintf(logfile,"---------------------------\n");
  fflush(logfile);
  fclose(logfile);
}

/* static */ const char *VCalConduit::version()
{
	return i18n("VCal Conduit v" VERSION);
}

void VCalConduit::doBackup()
{
  PilotRecord* rec;
  int index = 0;
  
  rec = readRecordByIndex(index++);
  
  // Get ALL entries from Pilot
  while (rec) {
    if (rec->getAttrib() & dlpRecAttrDeleted) { // tagged for deletion
      deleteVObject(rec);
    } else {
       updateVObject(rec);
    }    
    delete rec;
    rec = readRecordByIndex(index++);
  }
  // save the vCalendar
  fflush(logfile);
  saveVCal();
}

void VCalConduit::doSync()
{
   PilotRecord* rec;

   rec = readNextModifiedRecord();

   // get only MODIFIED entries from Pilot, compared with the above (doBackup),
   // which gets ALL entries
   while (rec) {
     fflush(logfile);
     if(rec->getAttrib() & dlpRecAttrDeleted) {
       fprintf(logfile,"found deleted pilot rec, marking corresponding vobject for deletion\n");
       fflush(logfile);
       deleteVObject(rec);
     } else {
       bool pilotRecModified = (rec->getAttrib() & dlpRecAttrDirty);
       if (pilotRecModified) {
	 fprintf(logfile,"found modified / dirty record on pilot, updating corresponding vobject\n");
	 fflush(logfile);
	 updateVObject(rec);
       } else {
	 fprintf(logfile,"weird! we asked for a modified record and got one that wasn't.\n");
	 fflush(logfile);
       }
     }
     
     delete rec;
     rec = readNextModifiedRecord();
   }
   
   // now, all the stuff that was modified/new on the pilot should be
   // added to the vCalendar.  We now need to add stuff to the pilot
   // that is modified/new in the vCalendar (the opposite).	  
   fprintf(logfile,"doing local sync\n");
   fflush(logfile);
   doLocalSync();
   fprintf(logfile,"local sync finished\n");
   fflush(logfile);
   
   // now we save the vCalendar.
   saveVCal();
}


/*****************************************************************************/

/*
 * Given a pilot record, check to see what needs to be done to the
 * analogous vobject to bring things into sync.
 */
void VCalConduit::updateVObject(PilotRecord *rec)
{
  fflush(logfile);
  VObject *vevent;
  VObject *vo;
  QDateTime todaysDate = QDateTime::currentDateTime();
  QString dateString, tmpStr;
  QString numStr;
  PilotDateEntry dateEntry(rec);
  
  vevent = findEntryInCalendar(rec->getID());
  if (!vevent) {
    fflush(logfile);
    // no event was found, so we need to add one with some initial info
    vevent = addProp(fCalendar, VCEventProp);
    
    dateString.sprintf("%.2d%.2d%.2dT%.2d%.2d%.2d",
			todaysDate.date().year(), todaysDate.date().month(),
		       todaysDate.date().day(), todaysDate.time().hour(),
		       todaysDate.time().minute(), todaysDate.time().second());
    addPropValue(vevent, VCDCreatedProp, dateString.data());
    numStr.sprintf("KPilot - %d",rec->getID());
    addPropValue(vevent, VCUniqueStringProp, numStr.data());
    addPropValue(vevent, VCSequenceProp, "1");
    addPropValue(vevent, VCLastModifiedProp, dateString.data());
    
    addPropValue(vevent, VCPriorityProp, "0");
    addPropValue(vevent, VCTranspProp, "0");
    addPropValue(vevent, VCRelatedToProp, "0");
    addPropValue(vevent, KPilotIdProp, numStr.setNum(dateEntry.getID()));
    addPropValue(vevent, KPilotStatusProp, "0");
    fprintf(logfile,"the pilot record didn't exist as a vobject, new vobject created\n");
    fflush(logfile);
  }
  
  // determine whether the vobject has been modified since the last sync
  vo = isAPropertyOf(vevent, KPilotStatusProp);
  bool vcalRecModified = (atol(fakeCString(vObjectUStringZValue(vo))) == 1);
  
  if (vcalRecModified) {
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
  
  // START TIME //
  vo = isAPropertyOf(vevent, VCDTstartProp);
  
  // check whether the event is an event or an appointment.  See dateEntry
  // structure for more info.
  if (!dateEntry.getEvent()) {
    // the event doesn't "float"
    dateString.sprintf("%.4d%.2d%.2dT%.2d%.2d%.2d",
		       1900 + dateEntry.getEventStart().tm_year,
		       dateEntry.getEventStart().tm_mon + 1,
		       dateEntry.getEventStart().tm_mday,
		       dateEntry.getEventStart().tm_hour,
		       dateEntry.getEventStart().tm_min,
		       dateEntry.getEventStart().tm_sec);
    if (vo)
      setVObjectUStringZValue_(vo, fakeUnicode(dateString.data(), 0));
    else 
      addPropValue(vevent, VCDTstartProp, dateString.data());
  } else {
    // the event floats
    dateString.sprintf("%.4d%.2d%.2dT000000",
		       1900 + dateEntry.getEventStart().tm_year,
		       dateEntry.getEventStart().tm_mon + 1,
		       dateEntry.getEventStart().tm_mday);
    if (vo)
      setVObjectUStringZValue_(vo, fakeUnicode(dateString.data(), 0));
    else
      addPropValue(vevent, VCDTstartProp, dateString.data());
  }
  
  // END TIME //
  vo = isAPropertyOf(vevent, VCDTendProp);
  
  bool multiDay = (dateEntry.getRepeatType() == repeatDaily);

  if (multiDay) {
    fprintf(logfile,"found a pilot event with is a single day repeater\n");
    fflush(logfile);
  }

  QDateTime endDT;
  // handle the case of a "repeating event on a daily basis" which is the
  // pilot's way of indicating a multi-day event.
  if (!multiDay) {
    endDT.setDate(QDate(1900+dateEntry.getEventEnd().tm_year,
			dateEntry.getEventEnd().tm_mon + 1,
			dateEntry.getEventEnd().tm_mday));
    endDT.setTime(QTime(dateEntry.getEventEnd().tm_hour,
			dateEntry.getEventEnd().tm_min,
			dateEntry.getEventEnd().tm_sec));
  } else {
    endDT.setDate(QDate(1900+dateEntry.getRepeatEnd().tm_year,
			dateEntry.getRepeatEnd().tm_mon + 1,
			dateEntry.getRepeatEnd().tm_mday));
    endDT.setTime(QTime(dateEntry.getRepeatEnd().tm_hour,
			dateEntry.getRepeatEnd().tm_min,
			dateEntry.getRepeatEnd().tm_sec));
  }
      
  if (!dateEntry.getEvent()) {
    // the event doesn't "float"
    dateString.sprintf("%.4d%.2d%.2dT%.2d%.2d%.2d",
		       endDT.date().year(),
		       endDT.date().month(),
		       endDT.date().day(),
		       endDT.time().hour(),
		       endDT.time().minute(),
		       endDT.time().second());
  } else {
    // the event "floats"
    dateString.sprintf("%.4d%.2d%.2dT000000",
		       endDT.date().year(),
		       endDT.date().month(),
		       endDT.date().day());
  }



  if (vo)
    setVObjectUStringZValue_(vo, fakeUnicode(dateString.data(), 0));
  else if (!dateEntry.getEvent() || multiDay)
    // we don't want to add it if it isn't there already, or if the
    // event isn't multiday/floating.
    // it is deprecated to have both DTSTART and DTEND set to 000000 for
    // their times.
    addPropValue(vevent, VCDTendProp, dateString.data());

  fprintf(logfile,"got to point after times are entered\n");
  fflush(logfile);
  
  // ALARM(s) //
  vo = isAPropertyOf(vevent, VCDAlarmProp);
  if (dateEntry.getAlarm()) {
    QDateTime alarmDT;
    alarmDT.setDate(QDate(1900+dateEntry.getEventStart().tm_year,
			  dateEntry.getEventStart().tm_mon + 1,
			  dateEntry.getEventStart().tm_mday));
    alarmDT.setTime(QTime(dateEntry.getEventStart().tm_hour,
			  dateEntry.getEventStart().tm_min,
			  dateEntry.getEventStart().tm_sec));

    int advanceUnits = dateEntry.getAdvanceUnits();
    if (advanceUnits == advMinutes)
      advanceUnits = 1;
    else if (advanceUnits == advHours)
      advanceUnits = 60;
    else if (advanceUnits == advDays)
      advanceUnits = 60*24;

    alarmDT = alarmDT.addSecs(60*advanceUnits*-(dateEntry.getAdvance()));

    dateString.sprintf("%.4d%.2d%.2dT%.2d%.2d%.2d",
		       alarmDT.date().year(),
		       alarmDT.date().month(),
		       alarmDT.date().day(),
		       alarmDT.time().hour(), 
		       alarmDT.time().minute(),
		       alarmDT.time().second());
    if (vo) {
      vo = isAPropertyOf(vo, VCRunTimeProp);
      setVObjectUStringZValue_(vo, fakeUnicode(dateString.data(), 0));
    } else {
      vo = addProp(vevent, VCDAlarmProp);
      addPropValue(vo, VCRunTimeProp, dateString.data());
      addPropValue(vo, VCRepeatCountProp, "1");
      addPropValue(vo, VCDisplayStringProp, "beep!");
    }

  } else {
    if (vo)
      addProp(vo, KPilotSkipProp);
  }
   
  fprintf(logfile,"got to point after alarm is entered\n");
  fflush(logfile);

  // RECURRENCE(S) //
  vo = isAPropertyOf(vevent, VCRRuleProp);
  // pilot entries that repeat daily are not what we consider daily
  // repeating events in vCalendar/KOrganizer.  It is actually a multi-day
  // appointment.  We need to conver it to this sort of thing (done above).
  if (dateEntry.getRepeatType() != repeatNone &&
      dateEntry.getRepeatType() != repeatDaily) {
    tmpStr = "";
    switch(dateEntry.getRepeatType()) {
    case repeatWeekly:
      tmpStr.sprintf("W%i ", dateEntry.getRepeatFrequency());
      if (dateEntry.getRepeatDays()[0])
	tmpStr.append("SU ");
      else if (dateEntry.getRepeatDays()[1])
	tmpStr.append("MO ");
      else if (dateEntry.getRepeatDays()[2])
	tmpStr.append("TU ");
      else if (dateEntry.getRepeatDays()[3])
	tmpStr.append("WE ");
      else if (dateEntry.getRepeatDays()[4])
	tmpStr.append("TH ");
      else if (dateEntry.getRepeatDays()[5])
	tmpStr.append("FR ");
      else if (dateEntry.getRepeatDays()[6])
	tmpStr.append("SA ");
      break;
    case repeatMonthlyByDay: {
      tmpStr.sprintf("MP%i %d+ ",dateEntry.getRepeatFrequency(),
		     (dateEntry.getRepeatDay() / 7) + 1);
      int day = dateEntry.getRepeatDay() % 7;
      if (day == 0)
	tmpStr.append("SU ");
      else if (day == 1)
	tmpStr.append("MO ");
      else if (day == 2)
	tmpStr.append("TU ");
      else if (day == 3)
	tmpStr.append("WE ");
      else if (day == 4)
	tmpStr.append("TH ");
      else if (day == 5)
	tmpStr.append("FR ");
      else if (day == 6)
	tmpStr.append("SA ");
      break;
    }
    case repeatMonthlyByDate:
      tmpStr.sprintf("MD%i ",dateEntry.getRepeatFrequency());
      break;
    case repeatYearly:
      tmpStr.sprintf("YD%i ",dateEntry.getRepeatFrequency());
      break;
    case repeatNone:
      fprintf(logfile,"argh! we think it repeats, but dateEntry has repeatNone!");
      break;
    default:
      break;
    }
    if (dateEntry.getRepeatForever())
      tmpStr += "#0";
    else 
      tmpStr += TmToISO(dateEntry.getRepeatEnd());

    vo = isAPropertyOf(vevent, VCRRuleProp);
    if (vo)
      setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.data(), 0));
    else
      addPropValue(vevent, VCRRuleProp,tmpStr.data());
  } else {
    if (vo)
      addProp(vo, KPilotSkipProp);
  }

  fprintf(logfile,"successfully entered recurrence rules, if any\n");
  fflush(logfile);

  // EXCEPTION(S) //
  vo = isAPropertyOf(vevent, VCExDateProp);
  tmpStr = "";
  if (dateEntry.getExceptionCount()) {
    for (int i = 0; i < dateEntry.getExceptionCount(); i++) {
      tmpStr = TmToISO(dateEntry.getExceptions()[i]);
      tmpStr += ";";
    }
    tmpStr.truncate(tmpStr.length()-1);
    if (vo)
      setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.data(), 0));
    else
      addPropValue(vevent, VCExDateProp, tmpStr.data());
  } else {
    if (vo)
      addProp(vo, KPilotSkipProp);
  }

  fprintf(logfile,"successfully entered exceptions, if any\n");
  fflush(logfile);

  // SUMMARY //
  vo = isAPropertyOf(vevent, VCSummaryProp);
  tmpStr = dateEntry.getDescription();
  // the following should take care of the multi-line summary bug.
  tmpStr = tmpStr.simplifyWhiteSpace();
  
  // the vCalendar parser doesn't handle empty summaries very well...
  if (!tmpStr.isEmpty()) {
    if (vo)
      setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.data(), 0));
    else
      addPropValue(vevent, VCSummaryProp, tmpStr.data());
  }

  fprintf(logfile,"successfully entered summary\n");
  fflush(logfile);
  
  // DESCRIPTION (NOTE) //
  vo = isAPropertyOf(vevent, VCDescriptionProp);
  if (dateEntry.getNote() != 0L && strlen(dateEntry.getNote()) != 0) {
    if (vo)
      setVObjectUStringZValue_(vo, fakeUnicode(dateEntry.getNote(), 0));
    else
      vo = addPropValue(vevent, VCDescriptionProp, dateEntry.getNote());
    // if the description takes up more than one line, we need
    // to add the Quoted-Printable property.
    if (strchr(dateEntry.getNote(), '\n') &&
	!isAPropertyOf(vo, VCQuotedPrintableProp))
      addProp(vo, VCQuotedPrintableProp);
  } else {
    if (vo)
      addProp(vo, KPilotSkipProp);
  }

  fprintf(logfile,"successfully entered attached note (description), if any\n");
  fflush(logfile);

  // CLASS (SECRECY) //
  vo = isAPropertyOf(vevent, VCClassProp);
  (rec->getAttrib() & dlpRecAttrSecret) ?
    tmpStr = "PRIVATE" : tmpStr = "PUBLIC";
  if (vo)
    setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.data(), 0));
  else
    addPropValue(vevent, VCClassProp, tmpStr.data());
  
  // PILOT STATUS //
  vo = isAPropertyOf(vevent, KPilotStatusProp);
  // TURN OFF MODIFIED
  if (vo) {
    int voStatus = atol(fakeCString(vObjectUStringZValue(vo)));
    if (voStatus != 0)
      fprintf(logfile,"in updateVObject there was a vobject with status %d!\n",
	      voStatus);
    setVObjectUStringZValue_(vo, fakeUnicode("0", 0));
  } else
    addPropValue(vevent, KPilotStatusProp, "0");
  
  fprintf(logfile,"finished updating/creating vobject\n");
  fflush(logfile);
}

/*
 * The pilot record specified was deleted on the pilot.  Remove
 * the corresponding vobject from the vCalendar.
 */
void VCalConduit::deleteVObject(PilotRecord *rec)
{
  VObject *delvo;
  
  delvo = findEntryInCalendar(rec->getID());
  // if the entry was found, it is still in the vCalendar.  We need to
  // set the Status flag to Deleted, so that KOrganizer will not load
  // it next time the vCalendar is read in.  If it is not found, the
  // user has also deleted it already in the vCalendar, and we can
  // safely do nothing.
  if (delvo) {
    fprintf(logfile,"found the pilot event %ld in vcalendar, marking w/KPilotSkipProp...\n", rec->getID());
    fflush(logfile);
    // we now use the additional 'KPilotSkip' property, instead of a special
    // value for KPilotStatusProp.
    addProp(delvo, KPilotSkipProp);
  }  
}

/*****************************************************************************/

void VCalConduit::saveVCal()
{
	FUNCTIONSETUP;

	KConfig* config = KPilotLink::getConfig(VCalSetup::VCalGroup);
	QString calName = config->readEntry("CalFile");
	if (fCalendar)
	{
		writeVObjectToFile(calName.data(), fCalendar);  
	}
}

void VCalConduit::doLocalSync()
{
	FUNCTIONSETUP;

	bool LocalOverridesPilot=false;
	VObjectIterator i;
	VObject *vevent = 0L;
	VObject *vo;
	char *s;
	int status;
	recordid_t id;
	PilotRecord *pRec;
	PilotDateEntry *dateEntry;
	timeZone = 0;

	// The first time we run the conduit
	// questions are asked about the disposition of all
	// unknown vCal entries; in later syncs we will assume
	// they should be deleted *except* if "Local Overrides
	// Pilot" is off. The first time, then, ask the questions
	// and delete the records that need deleting; afterwards
	// don't ask questions and delete according to the override
	// flag.
	//
	//
	if (first)
	{
		LocalOverridesPilot=true;
	}
	else
	{
		// We need one of KPilot's global 
		// settings.
		//
		KConfig *config=KPilotLink::getConfig();
		config->setGroup(0L);
		LocalOverridesPilot=config->readNumEntry("OverwriteRemote",0);
		delete config;
	}

  
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
  
  // go through the whole vCalendar.  If the event has the dirty (modified)
  // flag set, make a new pilot record and add it.
  // we only take events that have KPilotStatusProp as a property.  If
  // this property isn't present, ignore the event.
  while (moreIteration(&i)) {
    vevent = nextVObject(&i);
    vo = isAPropertyOf(vevent, KPilotStatusProp);
    
    if (vo && (strcmp(vObjectName(vevent), VCEventProp) == 0)) {
      status = 0;
      status = atoi(s = fakeCString(vObjectUStringZValue(vo)));
      deleteStr(s);
      
      if (status == 1) {
	// the event has been modified, need to write it to the pilot
	// we read the pilotID.
	
	vo = isAPropertyOf(vevent, KPilotIdProp);
	id = atoi(s = fakeCString(vObjectUStringZValue(vo)));
	deleteStr(s);
	
	// if id != 0, this is a modified event,
	// otherwise it is new.

	if (id != 0) {
	  pRec = readRecordById(id);
	  // if this fails, somehow the record got deleted from the pilot
	  // but we were never informed! bad pilot. naughty pilot.
	  ASSERT(pRec);
	  if (!pRec) {
	    fprintf(logfile,"record somehow got deleted off pilot, we weren't told.\n");
	    fflush(logfile);
	  }
	  
	  if (!pRec) {
	    dateEntry = new PilotDateEntry();
	    id = 0;
	    fprintf(logfile,"Created new pilot entry, and correcting vcalendar.\n");
	    fflush(logfile);
	  } else {
	    fprintf(logfile, "retrieving old pilot entry for updating...");
	    fflush(logfile);
	    dateEntry = new PilotDateEntry(pRec);
	    fprintf(logfile,"retrieved\n");
	    fflush(logfile);
	  }
	} else {
	  dateEntry = new PilotDateEntry();
	  fprintf(logfile,"Created new pilot entry\n");
	  fflush(logfile);
	}
	
	// START TIME //
	int event=0;
	struct tm end,start;
	
	if ((vo = isAPropertyOf(vevent, VCDTstartProp)) != 0L) {
	  char *s = fakeCString(vObjectUStringZValue(vo));
	  start = ISOToTm(QString(s));
	  deleteStr(s);
	  
	  if (start.tm_hour == 0 &&
	      start.tm_min == 0 &&
	      start.tm_sec == 0)
	    event = 1; // the event floats
	  
	  dateEntry->setEventStart(start);
	  
	}
	
	// TIMELESS EVENT? //
	dateEntry->setEvent(event);

	// END TIME //
	if ((vo = isAPropertyOf(vevent, VCDTendProp)) != 0L) {
	  char *s = fakeCString(vObjectUStringZValue(vo));
	  end = ISOToTm(QString(s));
	  deleteStr(s);
	  dateEntry->setEventEnd(end);
	} else {
	  // if the event has no DTend, get it from start time.
	  dateEntry->setEventEnd(start);
	}

	// ALARM(s) //
	if ((vo = isAPropertyOf(vevent, VCDAlarmProp))) {
	  VObject *a = 0L, *b = 0L;
	  if ((a = isAPropertyOf(vo, VCRunTimeProp)) &&
	      (b = isAPropertyOf(vevent, VCDTstartProp))) {
	      dateEntry->setAlarm(1);
	      QDate tmpDate;
	      QTime tmpTime;
	      int year, month, day, hour, minute, second;
	      
	      QString tmpStr = fakeCString(vObjectUStringZValue(a));
	      year = tmpStr.left(4).toInt();
	      month = tmpStr.mid(4,2).toInt();
	      day = tmpStr.mid(6,2).toInt();
	      hour = tmpStr.mid(9,2).toInt();
	      minute = tmpStr.mid(11,2).toInt();
	      second = tmpStr.mid(13,2).toInt();
	      tmpDate.setYMD(year, month, day);
	      tmpTime.setHMS(hour, minute, second);
	      
	      ASSERT(tmpDate.isValid());
	      ASSERT(tmpTime.isValid());
	      QDateTime tmpDT(tmpDate, tmpTime);
	      // correct for GMT if string is in Zulu format
	      if (tmpStr.right(1) == 'Z')
		tmpDT = tmpDT.addSecs(60*timeZone);

	      tmpStr = fakeCString(vObjectUStringZValue(b));
	      year = tmpStr.left(4).toInt();
	      month = tmpStr.mid(4,2).toInt();
	      day = tmpStr.mid(6,2).toInt();
	      hour = tmpStr.mid(9,2).toInt();
	      minute = tmpStr.mid(11,2).toInt();
	      second = tmpStr.mid(13,2).toInt();
	      tmpDate.setYMD(year, month, day);
	      tmpTime.setHMS(hour, minute, second);
	      
	      ASSERT(tmpDate.isValid());
	      ASSERT(tmpTime.isValid());
	      QDateTime tmpDT2(tmpDate, tmpTime);
	      // correct for GMT if string is in Zulu format
	      if (tmpStr.right(1) == 'Z')
		tmpDT2 = tmpDT2.addSecs(60*timeZone);

	      int diffSecs = tmpDT.secsTo(tmpDT2);
	      if (diffSecs > 60*60*24) {
		dateEntry->setAdvanceUnits(advDays);
		dateEntry->setAdvance((int) diffSecs/(60*60*24));
	      } else if (diffSecs > 60*60) {
		dateEntry->setAdvanceUnits(advHours);
		dateEntry->setAdvance((int) diffSecs/(60*60));
	      } else {
		dateEntry->setAdvanceUnits(advMinutes);
		dateEntry->setAdvance((int) diffSecs/60);
	      }
	  } else {
	    dateEntry->setAlarm(0);
	  }
	} else {
	  dateEntry->setAlarm(0);
	}

	// RECURRENCE(S) //
	// first we have a 'fake type of recurrence' when a multi-day
	// even it passed to the pilot, it is converted to an event
	// which recurs daily a number of times.
	QDateTime tmpDtStart(QDate(1900 + dateEntry->getEventStart().tm_year,
				   dateEntry->getEventStart().tm_mon + 1,
				   dateEntry->getEventStart().tm_mday),
			     QTime(dateEntry->getEventStart().tm_hour,
				   dateEntry->getEventStart().tm_min,
				   dateEntry->getEventStart().tm_sec));
	QDateTime tmpDtEnd(QDate(1900 + dateEntry->getEventEnd().tm_year,
				 dateEntry->getEventEnd().tm_mon + 1,
				 dateEntry->getEventEnd().tm_mday),
			   QTime(dateEntry->getEventEnd().tm_hour,
				 dateEntry->getEventEnd().tm_min,
				 dateEntry->getEventEnd().tm_sec));

	if (tmpDtStart.daysTo(tmpDtEnd)) {
	  // multi day event
	  dateEntry->setRepeatType(repeatDaily);
	  dateEntry->setRepeatFrequency(1);
	  dateEntry->setRepeatEnd(dateEntry->getEventEnd());
	  struct tm thing = dateEntry->getEventEnd();
	  fprintf(logfile,"set single-day recurrence: ");
	  fprintf(logfile, "year: %d, month: %d, day: %d\n",thing.tm_year + 1900, 
		  thing.tm_mon + 1, thing.tm_mday);
	  fflush(logfile);
	}

	// the following block of code is adapted from calobject.cpp
	// in korganizer. Make sure to update this section if that changes.
	if ((vo = isAPropertyOf(vevent, VCRRuleProp)) != 0L) {
	  QString tmpStr(s = fakeCString(vObjectUStringZValue(vo)));
	  deleteStr(s);
	  tmpStr.simplifyWhiteSpace();
	  tmpStr = tmpStr.upper();
	  
	  /********************************* DAILY **************************/
	  if (tmpStr.left(1) == "D") {
	    dateEntry->setRepeatType(repeatDaily);
	    int index = tmpStr.find(' ');
	    int rFreq = tmpStr.mid(1, (index-1)).toInt();
	    index = tmpStr.findRev(' ') + 1; // advance to last field
	    if (tmpStr.mid(index,1) == "#") index++;
	    if (tmpStr.find('T', index) != -1) {
	      struct tm rEndDate = (ISOToTm(tmpStr.mid(index, tmpStr.length() - index).data()));
	      dateEntry->setRepeatFrequency(rFreq);
	      dateEntry->setRepeatEnd(rEndDate);
	    } else {
	      int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	      if (rDuration == 0) { 
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever();
	      } else {
		// we could calculate an end date here, but too lazy right now.
		// pilot doesn't understand concept of repeat n times.
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever(); // modify me to fix
	      }
	    }
	  }
	  /********************************* WEEKLY **************************/
	  else if (tmpStr.left(1) == "W") {
	    dateEntry->setRepeatType(repeatWeekly);
	    int index = tmpStr.find(' ');
	    int last = tmpStr.findRev(' ') + 1;
	    int rFreq = tmpStr.mid(1, (index-1)).toInt();
	    index += 1; // advance to beginning of stuff after freq
	    QBitArray qba(7);
	    QString dayStr;
	    if (index == last) {
	      QDate tmpDate(1900 + dateEntry->getEventStart().tm_year,
			    dateEntry->getEventStart().tm_mon + 1,
			    dateEntry->getEventStart().tm_mday);
	      qba.setBit(tmpDate.dayOfWeek() - 1);
	    } else {
	      while (index < last) {
		dayStr = tmpStr.mid(index, 3);
		int dayNum = numFromDay(dayStr);
		qba.setBit(dayNum);
		index += 3; // advance to next day, or possibly "#"
	      }
	    }
	    dateEntry->setRepeatDays(qba);
	    index = last; if (tmpStr.mid(index,1) == "#") index++;
	    if (tmpStr.find('T', index) != -1) {
	      struct tm rEndDate = (ISOToTm(tmpStr.mid(index, tmpStr.length() - index).data()));
	      dateEntry->setRepeatFrequency(rFreq);
	      dateEntry->setRepeatEnd(rEndDate);
	    } else {
	      int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	      if (rDuration == 0) {
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever();
	      } else {
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever(); // again, modify to fix
	      }
	    }
	  }
	  /********************* MONTHLY-BY-POS ***************************/
	  else if (tmpStr.left(2) == "MP") {
	    dateEntry->setRepeatType(repeatMonthlyByDay);
	    int index = tmpStr.find(' ');
	    int last = tmpStr.findRev(' ') + 1;
	    int rFreq = tmpStr.mid(2, (index-1)).toInt();
	    index += 1; // advance to beginning of stuff after freq
	    short tmpPos;
	    if (index == last) {
	      QDate tmpDate(1900 + dateEntry->getEventStart().tm_year,
			    dateEntry->getEventStart().tm_mon + 1,
			    dateEntry->getEventStart().tm_mday);
	      tmpPos = tmpDate.day()/7 + 1;
	      if (tmpPos == 5)
		tmpPos = -1;
	      int rd = 7 * (tmpPos - 1) + tmpDate.dayOfWeek() - 1;
	      dateEntry->setRepeatDay((DayOfMonthType) rd);
	    } else {
	      while (index < last) {
		tmpPos = tmpStr.mid(index, 1).toShort();
		index += 1;
		if (tmpStr.mid(index,1) == "-")
		  // convert tmpPos to negative
		  tmpPos = 0 - tmpPos;
		index += 2; // advance to day(s)
		int dayNum = 0;
		while (numFromDay(tmpStr.mid(index,3)) >= 0) {
		  if (!dayNum) // pilot can only handle 1 day in month-by-pos
		    dayNum = numFromDay(tmpStr.mid(index,3));
		  index += 3; // advance to next day, or possibly pos / "#"
		}
		int rd = 7 * (tmpPos - 1) + dayNum;
		dateEntry->setRepeatDay((DayOfMonthType) rd);
	      }
	    }
	    index = last; if (tmpStr.mid(index,1) == "#") index++;
	    if (tmpStr.find('T', index) != -1) {
	      struct tm rEndDate = (ISOToTm(tmpStr.mid(index, tmpStr.length() - index).data()));
	      dateEntry->setRepeatFrequency(rFreq);
	      dateEntry->setRepeatEnd(rEndDate);
	    } else {
	      int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	      if (rDuration == 0) {
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever();
	      } else {
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever(); // again, modify me to fix
	      }
	    }
	    
	  }
	  /********************* MONTHLY-BY-DAY ***************************/
	  else if (tmpStr.left(2) == "MD") {
	    // the date to repeat on is the date of the event implicitly.
	    dateEntry->setRepeatType(repeatMonthlyByDate);
	    int index = tmpStr.find(' ');
	    int last = tmpStr.findRev(' ') + 1;
	    int rFreq = tmpStr.mid(2, (index-1)).toInt();
	    index += 1;
	    short tmpDay; // we aren't doing anything with this right now
	    if (index == last) {
	      tmpDay = dateEntry->getEventStart().tm_mday;
	    } else {
	      while (index < last) {
		int index2 = tmpStr.find(' ', index);
		tmpDay = tmpStr.mid(index, (index2-index)).toShort();
		index = index2-1;
		if (tmpStr.mid(index, 1) == "-")
		  tmpDay = 0 - tmpDay;
		index += 2; // advance the index;
	      } // while != #
	    }
	    index = last; if (tmpStr.mid(index,1) == "#") index++;
	    if (tmpStr.find('T', index) != -1) {
	      struct tm rEndDate = (ISOToTm(tmpStr.mid(index, tmpStr.length() - index).data()));
	      dateEntry->setRepeatFrequency(rFreq);
	      dateEntry->setRepeatEnd(rEndDate);
	    } else {
	      int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	      if (rDuration == 0) {
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever();
	      } else {
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever(); // again, modify me to fix
	      }
	    }
	  }
	  /*********************** YEARLY-BY-DAY ***************************/
	  else if (tmpStr.left(2) == "YD") {
	    dateEntry->setRepeatType(repeatYearly);
	    int index = tmpStr.find(' ');
	    int last = tmpStr.findRev(' ') + 1;
	    int rFreq = tmpStr.mid(2, (index-1)).toInt();
	    index += 1;
	    short tmpDay; // not currently used
	    if (index == last) {
	      // blah, not doing anything with tmpDay
	    } else {
	      while (index < last) {
		int index2 = tmpStr.find(' ', index);
		tmpDay = tmpStr.mid(index, (index2-index)).toShort();
		index = index2+1;
	      } // while != #
	    }
	    index = last; if (tmpStr.mid(index,1) == "#") index++;
	    if (tmpStr.find('T', index) != -1) {
	      struct tm rEndDate = (ISOToTm(tmpStr.mid(index, tmpStr.length() - index).data()));
	      dateEntry->setRepeatFrequency(rFreq);
	      dateEntry->setRepeatEnd(rEndDate);
	    } else {
	      int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	      if (rDuration == 0) {
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever();
	      } else {
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever(); // again, modify me to fix
	      }
	    }
	  }
	} // recurs

	// EXCEPTION(S) //
	if ((vo = isAPropertyOf(vevent, VCExDateProp)) != 0L) {
	  struct tm *tmList = 0;
	  s = fakeCString(vObjectUStringZValue(vo));
	  QString tmpStr(s);
	  deleteStr(s);
	  int index = 0, index2 = 0, count = 0;
	  struct tm extm;
	  while ((index2 = tmpStr.find(',', index)) != -1) {
	    ++count;
	    tmList = (struct tm *) realloc(tmList, sizeof(struct tm)*count);
	    extm = ISOToTm(tmpStr.mid(index, (index2-index)));
	    tmList[count-1] = extm;
	    index = index2 + 1;
	  }
	  ++count;
	  tmList = (struct tm *) realloc(tmList, sizeof(struct tm)*count);
	  extm = ISOToTm(tmpStr.mid(index, (tmpStr.length()-index)));
	  tmList[count-1] = extm;
	  dateEntry->setExceptionCount(count);
	  dateEntry->setExceptions(tmList);
	}

	// SUMMARY //
	char *s2=0, *s3=0;
	// what we call summary pilot calls description.
	if ((vo = isAPropertyOf(vevent, VCSummaryProp)) != 0L) {
	  s2 = fakeCString(vObjectUStringZValue(vo));
	  dateEntry->setDescription(s2);
	} else {
	  s2 = (char *) malloc(2);
	  s2[0] = 0;
	  dateEntry->setDescription(s2);
	}

	// DESCRIPTION //
	// what we call description pilot puts as a separate note
	if ((vo = isAPropertyOf(vevent, VCDescriptionProp)) != 0L) {
	  s3 = fakeCString(vObjectUStringZValue(vo));
	  dateEntry->setNote(s3);
	} 

	// put the pilotRec in the database...
	pRec=dateEntry->pack();
	pRec->setAttrib(dateEntry->getAttrib() & ~dlpRecAttrDirty);
	id = writeRecord(pRec);

	delete(dateEntry);
	delete(pRec);
	deleteStr(s2);
	if (s3)
	  deleteStr(s3);

	// write the id we got from writeRecord back to the vObject
	if (id > 0) {
	  QString tmpStr;
	  tmpStr.setNum(id);
	  vo = isAPropertyOf(vevent, KPilotIdProp);
	  // give it an id.
	  setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.data(), 0));
	  vo = isAPropertyOf(vevent, KPilotStatusProp);
	  tmpStr = "0"; // no longer a modified event.
	  setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.data(), 0));

	  fprintf(logfile,"wrote and updated vobject/pilot event\n");
	  fflush(logfile);

	} else {
	  fprintf(logfile,"error! writeRecord returned a pilotID <= 0!\n");
	  fflush(logfile);
	}
      }
    }
  }
  // anything that is left on the pilot but that is not found in the
  // vCalendar has to be deleted.  We know this because we have added
  // everything to the vCalendar from the pilot that had the modified
  // flag set, and we have added everything to the pilot from the vCalendar
  // that had the modified flag set.
  //
  // insertall flag will be set in the future in the config file 
  // so that this behaviour can be overridden.
  //
	  // We don't set insertall - it's based on the first and the local 
	  // override flag. Still construct the delete list no matter what,
	  // just don't do anything if local override is set to false.
	  //
	  //
  PilotRecord *rec;
  int index=0, response, insertall=0;
  QList<int> deletedList;
  deletedList.setAutoDelete(TRUE);
  
  fprintf(logfile,"dealing with deleted entries from vcalendar, if there are any\n");
  fflush(logfile);
  
  //  Get all entries from Pilot
  while ((rec = readRecordByIndex(index++)) != 0) {
    dateEntry = new PilotDateEntry(rec); // convert to date structure
    vevent = findEntryInCalendar(rec->getID());
    if (vevent == 0L) {
      fprintf(logfile,"found event in pilot which can't be found in vcalendar\n");
      fflush(logfile);

      if (first == FALSE) {
		// add event to list of pilot events to delete
		deletedList.append(new int(rec->getID()));

		// In this case we want it entered into the vcalendar
		if (!LocalOverridesPilot) {
	   	 	updateVObject(rec);
		}
      } else { 
	if (insertall == 0) {
		QString text;
		text = i18n("This is the first time that "
			"you have done a HotSync\n"
			"with the vCalendar conduit. "
			"There is an appointment\n"
			"in the PalmPilot which is not "
			"in the vCalendar (KOrganizer).\n\n");
		text += i18n("Appointment: ");
		text += dateEntry->getDescription();
		text += i18n("\n\nWhat must be done with this appointment?");

		response = QMessageBox::information(0, 
			i18n("KPilot vCalendar Conduit"), 
			text, 
			i18n("&Insert"),i18n("&Delete"), i18n("Insert &All")
			,0);

		if (debug_level & SYNC_MINOR)
		{
			cerr << fname 
				<< ": Event disposition "
				<< response
				<< endl;
		}

		switch(response) 
		{
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
    rec = 0L;
  }
  
	if (LocalOverridesPilot)
	{
		if (debug_level & SYNC_MAJOR)
		{
			cerr << fname << ": Deleting records from pilot."
				<< endl;
		}

		for (int *j = deletedList.first(); j; j = deletedList.next()) 
		{
			fprintf(logfile,"deleting record %i\n",*j);
			fflush(logfile);
			rec = readRecordById(*j);
			rec->setAttrib(~dlpRecAttrDeleted);
			writeRecord(rec);
			delete rec;
		}
	}
	else
	{
		if (debug_level & SYNC_TEDIOUS)
		{
			cerr << fname << ": Leaving records in pilot, "
				"even those not found in organizer."
				<< endl;
		}
	}

	deletedList.clear();
  
}

/*
 * Given an pilot id, search the vCalendar for a matching vobject, and return
 * the pointer to that object.  If not found, return NULL.
 */
VObject* VCalConduit::findEntryInCalendar(unsigned int id)
{
  VObjectIterator i;
  VObject* entry = 0L;
  VObject* objectID;
  
  initPropIterator(&i, fCalendar);
  
  // go through all the vobjects in the vcal
  while (moreIteration(&i)) {
     entry = nextVObject(&i);
     objectID = isAPropertyOf(entry, KPilotIdProp);

     if (objectID && (strcmp(vObjectName(entry), VCEventProp) == 0)) {
       if(strtoul(fakeCString(vObjectUStringZValue(objectID)), 0L, 0) == id) {
  	 return entry;
       }
     }
  }
  return 0L;
}

/* put up teh about / setup dialog. */
QWidget* VCalConduit::aboutAndSetup()
{
  return new VCalSetup();
}

QString VCalConduit::TmToISO(struct tm tm)
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

struct tm VCalConduit::ISOToTm(const QString &tStr)
{
  struct tm tm;
  
  tm.tm_wday = 0; // unimplemented
  tm.tm_yday = 0; // unimplemented
  tm.tm_isdst = 0; // unimplemented
  
  sscanf(tStr.data(),"%04d%02d%02dT%02d%02d%02d",
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

int VCalConduit::numFromDay(const QString &day)
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

