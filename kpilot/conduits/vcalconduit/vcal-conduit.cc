/* vcal-conduit.cc		VCalendar Conduit
**
** Copyright (C) 1998-2000 by Dan Pilone, Preston Brown, and
**	Herwin Jan Steehouwer
**
** A program to synchronize KOrganizer's date book with the Palm
** Pilot / KPilot. This program is part of KPilot.
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

// $Revision$

#define DEBUG
#include "options.h"

#include <sys/types.h>
#include <signal.h>
#include <iostream.h>
#include <stdlib.h>
#include <time.h>

#include <qbitarray.h>
#include <qdir.h>
#include <qdatetm.h>
#include <qstring.h>
#include <qmsgbox.h>

#include <kconfig.h>
#include <kdebug.h>

#include "kpilotlink.h"
#include "pilotDatabase.h"
#include "pilotRecord.h"
#include "pilotDateEntry.h"

#include "vcal-conduit.h"
#include "vcal-setup.h"
#include "conduitApp.h"
#include "kpilot.h"

static const char *id=
	"$Id$";


// globals
bool first = TRUE;


int main(int argc, char* argv[])
{
  ConduitApp a(argc, argv, "vcal_conduit",
  	"Calendar / Organizer conduit",
	"4.0b2");
  a.addAuthor("Preston Brown",I18N_NOOP("Organizer author"));
	a.addAuthor("Adriaan de Groot",I18N_NOOP("Maintainer"));
  VCalConduit conduit(a.getMode());
  a.setConduit(&conduit);
  return a.exec(false,true);

	/* NOTREACHED */
	(void) id;
}




VCalConduit::VCalConduit(BaseConduit::eConduitMode mode)
  : BaseConduit(mode)
{
	FUNCTIONSETUP;

	fCalendar = 0L;
}


VCalConduit::~VCalConduit()
{
  if (fCalendar) {
    cleanVObject(fCalendar);
    cleanStrTbl();
  }
}


void VCalConduit::getCalendar()
{
	FUNCTIONSETUP;

	if (fCalendar)
	{
		kdWarning() << __FUNCTION__
			<< ": Already have a calendar file."
			<< endl;
		return;
	}

	KConfig& config = KPilotLink::getConfig(VCalSetup::VCalGroup);
	(void) getDebugLevel(config);
	QString calName = config.readEntry("CalFile");
	first = getFirstTime(config);

	DEBUGCONDUIT << fname
		<< ": Calendar file is " << calName
		<< ( first ? " (first time!)" : "" )
		<< endl;

#ifdef DEBUG
	// This (and the next debug message) need the #ifdefs
	// because they use the debugging functions qstringExpansion
	// and charExpansion.
	//
	//
	DEBUGCONDUIT << fname
		<< ": Using calendar name:"
		<< endl;
	DEBUGCONDUIT << fname
		<< qstringExpansion(calName)
		<< endl;
#endif
	QCString s=QFile::encodeName(calName);

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Encoded calendar name:"
		<< endl;
	DEBUGCONDUIT << fname
		<< charExpansion(s)
		<< endl;
#endif
	fCalendar = Parse_MIME_FromFileName((const char*)s);

	if(fCalendar == 0L) 
	{
		QString message = i18n(
			"The VCalConduit could not open the file `%1'. "
			"Please configure the conduit with the correct "
			"filename and try again.")
			.arg(calName);

		kdError() << __FUNCTION__
			<< ": Couldn't open "
			<< calName
			<< endl;

		KMessageBox::error(0, message,"vCalendar Conduit Fatal Error");
		exit(ConduitMisconfigured);
	}
	else
	{
		DEBUGCONDUIT << fname
			<< ": Got calendar!"
			<< endl;
	}
}

void VCalConduit::doBackup()
{
	FUNCTIONSETUP;
  PilotRecord* rec;
  int index = 0;
  
	getCalendar();

#ifdef DEBUG
	if(debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname
			<< ": Performing full backup"
			<< endl;
	}
#endif

	// Get ALL entries from Pilot
	//
	//
	while ((rec=readRecordByIndex(index++)))
	{
		if (rec->isDeleted()) 
		{
			deleteVObject(rec);
		} 
		else 
		{
			updateVObject(rec);
		}
		delete rec;
	}

	saveVCal();
}

void VCalConduit::doSync()
{
	FUNCTIONSETUP;

   PilotRecord* rec;
	int recordcount=0;

	getCalendar();
	if (fCalendar==0L)
	{
		kdError() << __FUNCTION__ << ": No calendar to sync with."
			<< endl;
		return;
	}

	// get only MODIFIED entries from Pilot, compared with 
	// the above (doBackup), which gets ALL entries
	//
	//
	while ((rec=readNextModifiedRecord())) 
	{
		recordcount++;
		if (rec->isDeleted())
		{
			deleteVObject(rec);
		} 
		else 
		{
			bool pilotRecModified = 
				(rec->getAttrib() & dlpRecAttrDirty);
			if (pilotRecModified) 
			{
				updateVObject(rec);
			} 
			else 
			{
				kdWarning() << __FUNCTION__
					<< "weird! we asked for a modified "
					   "record and got one that wasn't"
					<< endl;
			}
		}
		delete rec;
	}
   
#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname
			<< ": Read a total of "
			<< recordcount
			<< " record from the pilot."
			<< endl;
	}
#endif

   // now, all the stuff that was modified/new on the pilot should be
   // added to the vCalendar.  We now need to add stuff to the pilot
   // that is modified/new in the vCalendar (the opposite).	  
   doLocalSync();
   
   // now we save the vCalendar.
   saveVCal();
}

void VCalConduit::repeatForever(
	PilotDateEntry *dateEntry,
	int rFreq,
	VObject *vevent)
{
	FUNCTIONSETUP;

	const char *s = "<no description>";

	if (vevent)
	{
		VObject *vo = isAPropertyOf(vevent, VCSummaryProp);
		if (vo != 0L)
		{
			s=fakeCString(vObjectUStringZValue(vo));
		}
	}


	dateEntry->setRepeatFrequency(rFreq);
	dateEntry->setRepeatForever();
	kdWarning() << __FUNCTION__
		<< ": repeat duration is forever for "
		<< s
		<< endl;
}

void VCalConduit::repeatUntil(
	PilotDateEntry *dateEntry,
	struct tm *start,
	int rFreq,
	int rDuration,
	PeriodConstants period)
{
	FUNCTIONSETUP;
	time_t end_time = mktime(start);
	struct tm rEnd = *start;

	switch(period)
	{
	case DailyPeriod:
	case WeeklyPeriod:
		// Calculate the end time by adding the right number of
		// repeat periods.
		//
		//
		end_time += rFreq * (rDuration-1) * (int) period;

		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatEnd(*localtime(&end_time));
		break;
	case MonthlyByDayPeriod:
	case MonthlyByPosPeriod:
		dateEntry->setRepeatFrequency(rFreq);
		rEnd.tm_mon += rFreq * (rDuration - 1);
		rEnd.tm_year += rEnd.tm_mon / 12;
		rEnd.tm_mon %= 12;
		dateEntry->setRepeatEnd(rEnd);
		break;
	case YearlyByDayPeriod:
		dateEntry->setRepeatFrequency(rFreq);
		rEnd.tm_year += rFreq * (rDuration - 1);
		dateEntry->setRepeatEnd(rEnd);
		break;
	default:
		kdWarning() << __FUNCTION__
			<< ": Unknown repeat period "
			<< (int) period
			<< endl;
	}
}

/*****************************************************************************/

/*
 * Given a pilot record, check to see what needs to be done to the
 * analogous vobject to bring things into sync.
 */
void VCalConduit::updateVObject(PilotRecord *rec)
{
	FUNCTIONSETUP;

  VObject *vevent;
  VObject *vo;
  QDateTime todaysDate = QDateTime::currentDateTime();
  QString dateString, tmpStr;
  QString numStr;
  PilotDateEntry dateEntry(rec);
  
  vevent = findEntryInCalendar(rec->getID());
  if (!vevent) {
    // no event was found, so we need to add one with some initial info
    vevent = addProp(fCalendar, VCEventProp);
    
    dateString.sprintf("%.2d%.2d%.2dT%.2d%.2d%.2d",
			todaysDate.date().year(), todaysDate.date().month(),
		       todaysDate.date().day(), todaysDate.time().hour(),
		       todaysDate.time().minute(), todaysDate.time().second());
    addPropValue(vevent, VCDCreatedProp, dateString.latin1());
    numStr.sprintf("KPilot - %d",rec->getID());
    addPropValue(vevent, VCUniqueStringProp, numStr.latin1());
    addPropValue(vevent, VCSequenceProp, "1");
    addPropValue(vevent, VCLastModifiedProp, dateString.latin1());
    
    addPropValue(vevent, VCPriorityProp, "0");
    addPropValue(vevent, VCTranspProp, "0");
    addPropValue(vevent, VCRelatedToProp, "0");
    addPropValue(vevent, KPilotIdProp, numStr.setNum(dateEntry.getID()).latin1());
    addPropValue(vevent, KPilotStatusProp, "0");
  }
  
  // determine whether the vobject has been modified since the last sync
  bool vcalRecModified = true ;
  vo = isAPropertyOf(vevent, KPilotStatusProp);
  if (vo)
  {
	const wchar_t *vp = 0;
	const char *s = 0;
	int status = 1;

  	vp = vObjectUStringZValue(vo);
	if (vp)
	{
		s = fakeCString(vp);
	}
	if (s)
	{
		status=atol(s);
	}

	vcalRecModified = (status==1);
  }
  
  if (vcalRecModified) {
    // we don't want to modify the vobject with pilot info, because it has
    // already been  modified on the desktop.  The VObject's modified state
    // overrides the PilotRec's modified state.
    return;
  }
  // otherwise, the vObject hasn't been touched.  Updated it with the
  // info from the PilotRec.
  
#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname
			<< ": Updating start time"
			<< endl;
	}
#endif

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
	{
		setVObjectUStringZValue_(vo, 
			fakeUnicode(dateString.latin1(), 0));
	}
	else 
	{
		addPropValue(vevent, VCDTstartProp, dateString.latin1());
	}
  } else {
    // the event floats
    dateString.sprintf("%.4d%.2d%.2dT000000",
		       1900 + dateEntry.getEventStart().tm_year,
		       dateEntry.getEventStart().tm_mon + 1,
		       dateEntry.getEventStart().tm_mday);
	if (vo)
	{
		setVObjectUStringZValue_(vo, 
			fakeUnicode(dateString.latin1(), 0));
	}
	else
	{
		addPropValue(vevent, VCDTstartProp, dateString.latin1());
	}
}

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname
			<< ": Updating end time"
			<< endl;
	}
#endif
  
  // END TIME //
  vo = isAPropertyOf(vevent, VCDTendProp);
  
  // Patch by Heiko
  //
  //
  bool multiDay = ( (dateEntry.getRepeatType() == repeatDaily) &&
		dateEntry.getEvent() );

  if (multiDay) {
  	// I honestly don't know what was supposed to go here
	//
	//
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
  {
    setVObjectUStringZValue_(vo, fakeUnicode(dateString.latin1(), 0));
  }
  else 
  {
	// we don't want to add it if it isn't there already, or if the
	// event isn't multiday/floating.
	// it is deprecated to have both DTSTART and DTEND set to 000000 for
	// their times.
	if (!dateEntry.getEvent() || multiDay)
	{
		addPropValue(vevent, VCDTendProp, dateString.latin1());
	}
  }

  
#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname
			<< ": Updating alarms"
			<< endl;
	}
#endif
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
	switch(advanceUnits)
	{
	case advMinutes : advanceUnits = 1; break;
	case advHours : advanceUnits = 60; break;
	case advDays : advanceUnits = 60*24; break;
	default :
		kdDebug() << fname << ": Unknown advance units "
			<< advanceUnits
			<< endl;
		advanceUnits=1;
	}

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
      setVObjectUStringZValue_(vo, fakeUnicode(dateString.latin1(), 0));
    } else {
      vo = addProp(vevent, VCDAlarmProp);
      addPropValue(vo, VCRunTimeProp, dateString.latin1());
      addPropValue(vo, VCRepeatCountProp, "1");
      addPropValue(vo, VCDisplayStringProp, "beep!");
    }

  } else {
    if (vo)
      {
      addProp(vo, KPilotSkipProp);
      }
  }
   
#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname
			<< ": Updating recurrence"
			<< endl;
	}
#endif
  // RECURRENCE(S) //
  vo = isAPropertyOf(vevent, VCRRuleProp);
  // pilot entries that repeat daily are not what we consider daily
  // repeating events in vCalendar/KOrganizer.  It is actually a multi-day
  // appointment.  We need to convert it to multi-day (done above).
  if (dateEntry.getRepeatType() != repeatNone &&
		!multiDay) {
    tmpStr = "";
    switch(dateEntry.getRepeatType()) {
	case repeatDaily:
		tmpStr.sprintf("D%i ", dateEntry.getRepeatFrequency());
		break;
    case repeatWeekly:
	{
	const int *days = dateEntry.getRepeatDays();

	DEBUGCONDUIT << fname
		<< ": Got repeat-weekly entry, by-days="
		<< days[0] << " "
		<< days[1] << " "
		<< days[2] << " "
		<< days[4] << " "
		<< days[5] << " "
		<< days[6] << " "
		<< endl;

      tmpStr.sprintf("W%i ", dateEntry.getRepeatFrequency());
      if (days[0]) tmpStr.append("SU ");
      if (days[1]) tmpStr.append("MO ");
      if (days[2]) tmpStr.append("TU ");
      if (days[3]) tmpStr.append("WE ");
      if (days[4]) tmpStr.append("TH ");
      if (days[5]) tmpStr.append("FR ");
      if (days[6]) tmpStr.append("SA ");
      break;
    }
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
      kdDebug() << fname << ": argh! we think it repeats, "
      	<< "but dateEntry has repeatNone!\n";
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
      setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.latin1(), 0));
    else
      addPropValue(vevent, VCRRuleProp,tmpStr.latin1());
  } else {
    if (vo)
      addProp(vo, KPilotSkipProp);
  }


#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname
			<< ": Updating exceptions"
			<< endl;
	}
#endif
  // EXCEPTION(S) //
	if (multiDay && dateEntry.getExceptionCount())
	{
		kdDebug() << fname
			<< ": WARNING Exceptions ignored for multi-day event "
			<< dateEntry.getDescription()
			<< endl ;
	}
  vo = isAPropertyOf(vevent, VCExDateProp);
  tmpStr = QString::null ;
  if (dateEntry.getExceptionCount()) {
    for (int i = 0; i < dateEntry.getExceptionCount(); i++) {
      tmpStr += TmToISO(dateEntry.getExceptions()[i]);
      tmpStr += ";";
    }
    tmpStr.truncate(tmpStr.length()-1);
    if (vo)
      setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.latin1(), 0));
    else
      addPropValue(vevent, VCExDateProp, tmpStr.latin1());
  } else {
    if (vo)
      addProp(vo, KPilotSkipProp);
  }


	// The remainder of the updates are in individual methods
	//
	//
	setSummary(vevent,dateEntry.getDescription());
	setNote(vevent,dateEntry.getNote());
	setSecret(vevent,(rec->getAttrib() & dlpRecAttrSecret));
	setStatus(vevent,0);

#ifdef DEBUG
	if (debug_level & SYNC_TEDIOUS)
	{
		kdDebug() << fname
			<< ": Done updating"
			<< endl;
	}
#endif
}

void VCalConduit::setSummary(VObject *vevent,const char *summary)
{
	FUNCTIONSETUP;

	VObject *vo = isAPropertyOf(vevent, VCSummaryProp);
	QString qsummary (summary);
	qsummary = qsummary.simplifyWhiteSpace();
	if (qsummary.isEmpty())
	{
		// We should probably update (empty) the
		// summary in the VObject if there
		// is one.
		//
		//
	}
	else
	{
		if (vo)
		{
			setVObjectUStringZValue_(vo, 
				fakeUnicode(qsummary.latin1(), 0));
		}
		else
		{
			addPropValue(vevent, VCSummaryProp, 
				qsummary.latin1());
		}
	}
}

void VCalConduit::setNote(VObject *vevent,const char *s)
{
	FUNCTIONSETUP;

	VObject *vo = isAPropertyOf(vevent, VCDescriptionProp);

	if (s && *s)
	{
		// There is a note for this event
		//
		//
		if (vo)
		{
			setVObjectUStringZValue_(vo, fakeUnicode(s,0));
		}
		else
		{
			vo = addPropValue(vevent, VCDescriptionProp,s);
		}

		// vo now certainly (?) points to the note property.
		//
		//
		if (!vo)
		{
			kdError() << __FUNCTION__
				<< ": No object for note property."
				<< endl;
			return;
		}

		if (strchr(s,'\n') && 
			!isAPropertyOf(vo, VCQuotedPrintableProp))
		{
			// Note takes more than one line so we need
			// to add the Quoted-Printable property
			//
			//
			addProp(vo,VCQuotedPrintableProp);
		}
	}
	else
	{
		// No note at all
		//
		//
		if (vo)
		{
			// So skip existing note
			//
			//
			addProp(vo,KPilotSkipProp);
		}
	}
}

void VCalConduit::setSecret(VObject *vevent,bool secret)
{
	FUNCTIONSETUP;

	VObject *vo = isAPropertyOf(vevent, VCClassProp);
	const char *s = secret ? "PRIVATE" : "PUBLIC" ;

	if (vo)
	{
		setVObjectUStringZValue_(vo,fakeUnicode(s,0));
	}
	else
	{	
		addPropValue(vevent,VCClassProp,s);
	}
}

void VCalConduit::setStatus(VObject *vevent,int status)
{
	FUNCTIONSETUP;

	VObject *vo = isAPropertyOf(vevent, KPilotStatusProp);
	char buffer[2+4*sizeof(int)];	// One byte produces at most 3 digits
					// in decimal, add some slack and 
					// space for a -

	sprintf(buffer,"%d",status);

	if (vo)
	{

		setVObjectUStringZValue_(vo,fakeUnicode(buffer,0));
	}
	else
	{
		addPropValue(vevent,KPilotStatusProp,buffer);
	}
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
    // we now use the additional 'KPilotSkip' property, instead of a special
    // value for KPilotStatusProp.
    addProp(delvo, KPilotSkipProp);
  }  
}

/*****************************************************************************/

void VCalConduit::saveVCal()
{
	FUNCTIONSETUP;

	KConfig& config = KPilotLink::getConfig(VCalSetup::VCalGroup);
	QString calName = config.readEntry("CalFile");
	if (fCalendar)
	{
		writeVObjectToFile((char*)calName.latin1(), fCalendar);  
	}
}

int VCalConduit::getTimeZone() const
{
	FUNCTIONSETUP;

	VObject *vo = isAPropertyOf(fCalendar, VCTimeZoneProp);

	if (!vo)
	{
		return 0;
	}
  
	bool neg = FALSE;
	int hours, minutes;
	char *s;

	QString tmpStr(s = fakeCString(vObjectUStringZValue(vo)));
#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname
			<< ": Got time zone string "
			<< s
			<< endl;
	}
#endif
	deleteStr(s);

	if (tmpStr.left(1) == "-")
	{
		neg = TRUE;
	}
	if (tmpStr.left(1) == "-" || tmpStr.left(1) == "+")
	{
		tmpStr.remove(0, 1);
	}

	hours = tmpStr.left(2).toInt();
	if (tmpStr.length() > 2)
	{
		minutes = tmpStr.right(2).toInt();
	}
	else
	{
		minutes = 0;
	}


	int timeZone = (60*hours+minutes);
	if (neg)
	{
		timeZone = -timeZone;
	}

#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname
			<< ": Calculated TZ offset "
			<< timeZone
			<< endl;
	}
#endif


	return timeZone;
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
#ifdef DEBUG
		kdDebug() << fname
			<< ": This is the first time the vcal conduit is run"
			<< endl;
#endif
	}
	else
	{
		// We need one of KPilot's global 
		// settings.
		//
		KConfig& config=KPilotLink::getConfig();
		LocalOverridesPilot=config.readNumEntry("OverwriteRemote",0);
	}

#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname << ": Performing local sync."
			<< endl;
	}
  
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname << ": Getting timezone."
			<< endl;
	}
#endif

	timeZone = getTimeZone();

  
#ifdef DEBUG
	if (debug_level & SYNC_MINOR)
	{
		kdDebug() << fname << ": Initializing iterator."
			<< endl;
	}
#endif
	initPropIterator(&i, fCalendar);
  
	int recordcount=0;

	  // go through the whole vCalendar.  If the event has the 
	  // dirty (modified)
	  // flag set, make a new pilot record and add it.
	  // we only take events that have KPilotStatusProp as a property.  If
	  // this property isn't present, ignore the event.
	  //
	  //
	while (moreIteration(&i)) 
	{
		recordcount++;
		vevent = nextVObject(&i);
		vo = isAPropertyOf(vevent, KPilotStatusProp);

#ifdef DEBUG
		if (debug_level & SYNC_TEDIOUS)
		{
			kdDebug() << fname
				<< ": Read the following calendar entry:"
				<< endl;
			printVObject(stderr,vevent);
		}
#endif
    
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
	  }
	  
	  if (!pRec) {
	    dateEntry = new PilotDateEntry();
	    id = 0;
	  } else {
	    dateEntry = new PilotDateEntry(pRec);
	  }
	} else {
	  dateEntry = new PilotDateEntry();
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
	      if (tmpStr.right(1) == QString("Z"))
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
	      if (tmpStr.right(1) == QString("Z"))
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

	if (tmpDtStart.daysTo(tmpDtEnd) &&
		dateEntry->getEventStart().tm_hour == 0 &&
		dateEntry->getEventStart().tm_min == 0 &&
		dateEntry->getEventStart().tm_sec == 0)
	{
	  // multi day event
	  dateEntry->setRepeatType(repeatDaily);
	  dateEntry->setRepeatFrequency(1);
	  dateEntry->setRepeatEnd(dateEntry->getEventEnd());
		if (isAPropertyOf(vevent, VCExDateProp) != 0L) 
		{
			char *s=0L;
			vo = isAPropertyOf(vevent, VCSummaryProp);
			if (vo) s = fakeCString(vObjectUStringZValue(vo)) ;

			kdDebug() << fname
				<< ": WARNING: exceptions ignored "
				   "for multi-day event "
				<< s
				<< endl ;
		}
	  struct tm thing = dateEntry->getEventEnd();
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
	      struct tm rEndDate = (ISOToTm(tmpStr.mid(index, tmpStr.length() - index).latin1()));
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
		// repeatForever(dateEntry,rFreq,vevent);
		repeatUntil(dateEntry,&start,rFreq,rDuration,DailyPeriod);
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
	      struct tm rEndDate = (ISOToTm(tmpStr.mid(index, tmpStr.length() - index).latin1()));
	      dateEntry->setRepeatFrequency(rFreq);
	      dateEntry->setRepeatEnd(rEndDate);
	    } else {
	      int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	      if (rDuration == 0) {
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever();
	      } else {
		repeatUntil(dateEntry,&start,rFreq,rDuration,WeeklyPeriod);
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
	      struct tm rEndDate = (ISOToTm(tmpStr.mid(index, tmpStr.length() - index).latin1()));
	      dateEntry->setRepeatFrequency(rFreq);
	      dateEntry->setRepeatEnd(rEndDate);
	    } else {
	      int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	      if (rDuration == 0) {
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever();
	      } else {
			repeatUntil(dateEntry,&start,rFreq,rDuration,
				MonthlyByPosPeriod);
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
	      struct tm rEndDate = (ISOToTm(tmpStr.mid(index, tmpStr.length() - index).latin1()));
	      dateEntry->setRepeatFrequency(rFreq);
	      dateEntry->setRepeatEnd(rEndDate);
	    } else {
	      int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	      if (rDuration == 0) {
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever();
	      } else {
			repeatUntil(dateEntry,&start,rFreq,rDuration,
				MonthlyByDayPeriod);
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
	      struct tm rEndDate = (ISOToTm(tmpStr.mid(index, tmpStr.length() - index).latin1()));
	      dateEntry->setRepeatFrequency(rFreq);
	      dateEntry->setRepeatEnd(rEndDate);
	    } else {
	      int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	      if (rDuration == 0) {
		dateEntry->setRepeatFrequency(rFreq);
		dateEntry->setRepeatForever();
	      } else {
			repeatUntil(dateEntry,&start,rFreq,rDuration,
				YearlyByDayPeriod);
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
	if (id > 0) 
	{
	  QString tmpStr;
	  tmpStr.setNum(id);
	  vo = isAPropertyOf(vevent, KPilotIdProp);
	  // give it an id.
	  setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.latin1(), 0));
	  vo = isAPropertyOf(vevent, KPilotStatusProp);
	  tmpStr = "0"; // no longer a modified event.
	  setVObjectUStringZValue_(vo, fakeUnicode(tmpStr.latin1(), 0));
	} 
	else 
	{
	}
      }
    }
  }

#ifdef DEBUG
	if (debug_level & SYNC_MAJOR)
	{
		kdDebug() << fname
			<< ": Read "
			<< recordcount
			<< " records total."
			<< endl;
	}
#endif

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
  QValueList<int> deletedList;
  
  
  //  Get all entries from Pilot
  while ((rec = readRecordByIndex(index++)) != 0) 
  {
	DEBUGCONDUIT << fname
		<< ": Got record "
		<< index-1
		<< " @"
		<< (int) rec
		<< endl;

    dateEntry = new PilotDateEntry(rec); // convert to date structure

	if (dateEntry)
	{
		DEBUGCONDUIT << fname
			<< ": Created date entry @"
			<< (int) dateEntry
			<< " with id "
			<< rec->getID()
			<< endl;
	}
	else
	{
		kdWarning() << __FUNCTION__
			<< ": No dateentry!"
			<< endl;
		continue;
	}

    vevent = findEntryInCalendar(rec->getID());
    if (vevent == 0L) 
    {
		DEBUGCONDUIT << fname
			<< ": Entry not found in calendar."
			<< endl;


      if ( !first ) 
      {

		// In this case we want it entered into the vcalendar
		if (!LocalOverridesPilot) {
			DEBUGCONDUIT << fname
				<< ": !LocalOverridesPilot so updating object."
				<< endl;
				
	   	 	updateVObject(rec);
		}
		else
		{
			DEBUGCONDUIT << fname
				<< ": Scheduling this entry for deletion."
				<< endl;

		// add event to list of pilot events to delete
		deletedList.append(rec->getID());
		}
      } 
      else 
      { 
	if (insertall == 0) {
		DEBUGCONDUIT << fname
			<< ": Questioning event disposition."
			<< endl;

		QString text = i18n("This is the first time that "
			"you have done a HotSync\n"
			"with the vCalendar conduit. "
			"There is an appointment\n"
			"in the PalmPilot which is not "
			"in the vCalendar (KOrganizer).\n\n");
		text += i18n("Appointment: %1.\n\n"
			"What must be done with this appointment?")
			.arg(dateEntry->getDescription());

		response = QMessageBox::information(0, 
			i18n("KPilot vCalendar Conduit"), 
			text, 
			i18n("&Insert"),i18n("&Delete"), i18n("Insert &All")
			,0);

		DEBUGCONDUIT << fname 
			<< ": Event disposition "
			<< response
			<< endl;

		switch(response) 
		{
		case 1:
			deletedList.append(rec->getID());
			break;
		case 2 :
			insertall = 1;
			updateVObject(rec);
			break;

		// Default is to insert this single entry
		// and ask again later.
		//
		//
		default : 
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
#ifdef DEBUG
		if (debug_level & SYNC_MAJOR)
		{
			kdDebug() << fname << ": Deleting records from pilot."
				<< endl;
		}
#endif

		QValueList<int>::Iterator it;
		for (it = deletedList.begin(); it != deletedList.end(); ++it)
		{
			rec = readRecordById(*it);
			rec->setAttrib(~dlpRecAttrDeleted);
			writeRecord(rec);
			delete rec;
		}
	}
	else
	{
#ifdef DEBUG
		if (debug_level & SYNC_MAJOR)
		{
			kdDebug() << fname << ": Leaving records in pilot, "
				"even those not found in organizer."
				<< endl;
		}
#endif
	}

	deletedList.clear();
  
	KConfig& config = KPilotLink::getConfig(VCalSetup::VCalGroup);
	setFirstTime(config,false);
}

/*
 * Given an pilot id, search the vCalendar for a matching vobject, and return
 * the pointer to that object.  If not found, return NULL.
 */
VObject* VCalConduit::findEntryInCalendar(unsigned int id)
{
	FUNCTIONSETUP;

	VObjectIterator i;
	VObject* entry = 0L;
	VObject* objectID;

	initPropIterator(&i, fCalendar);

	// go through all the vobjects in the vcal
	while (moreIteration(&i)) 
	{
		entry = nextVObject(&i);
		if (!entry)
		{
			kdWarning() << __FUNCTION__
				<< ": nextVObject returned NULL!"
				<< endl;
			break;
		}

		objectID = isAPropertyOf(entry, KPilotIdProp);

		if (objectID && 
			(strcmp(vObjectName(entry), VCEventProp) == 0)) 
		{
			const char *s = fakeCString(
				vObjectUStringZValue(objectID));
			if (!s)
			{
				kdWarning() << __FUNCTION__
					<< ": fakeCString returned NULL!"
					<< endl;
				continue;
			}
#if 0
			else
			{
				DEBUGCONDUIT << fname
					<< ": Looking at object with id "
					<< s
					<< endl;
			}
#endif

			if(strtoul(s, 0L, 0) == id) 
			{
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

/* virtual */ void VCalConduit::doTest()
{
	FUNCTIONSETUP;

	QString message("This is a test. This is only a test.\n"
		"If this had been a real error message\n"
		"you would also have had an 18Mb core file\n"
		"on your hands.");

	KMessageBox::error(0, message, "Testing vCalendar Conduit");

	getCalendar();
}


// $Log$
// Revision 1.24  2001/02/05 19:16:28  adridg
// Removing calls to exit() from internal functions
//
// Revision 1.23  2001/02/02 17:31:57  adridg
// Reduced debugging message overload
//
// Revision 1.22  2001/01/15 14:50:50  bero
// Fix build
//
// Revision 1.21  2001/01/08 22:27:12  adridg
// Fixed QFile::encodeName stupidity
//
// Revision 1.20  2001/01/05 12:06:01  adridg
// Updated version number, removed VCalConduit::version()
//
// Revision 1.19  2001/01/03 00:05:13  adridg
// Administrative
//
// Revision 1.18  2000/12/31 16:44:00  adridg
// Patched up the debugging stuff again
//
// Revision 1.17  2000/12/30 20:27:42  adridg
// Dag's Patches for Repeating Events
//
// Revision 1.16  2000/12/09 12:51:07  adridg
// New patches
//
// Revision 1.15  2000/12/05 07:47:27  adridg
// New debugging stuff
//
// Revision 1.14  2000/11/26 01:41:48  adridg
// Last of Heiko's patches
//
