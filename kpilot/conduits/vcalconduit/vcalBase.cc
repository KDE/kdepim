/* vcalBase.cc			Program
**
** Copyright (C) 2001 by Adriaan de Groot
**
** Program description
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


#ifndef _KPILOT_OPTIONS_H
#include "options.h"
#endif

#ifndef QFILE_H
#include <qfile.h>
#endif

#ifndef QDATETIME_H
#include <qdatetime.h>
#endif

#ifndef _KMESSAGEBOX_H_
#include "kmessagebox.h"
#endif

#ifndef _KPILOT_KPILOTCONFIG_H
#include "kpilotConfig.h"
#endif

#ifndef _VCAL_VCALBASE_H
#include "vcalBase.h"
#endif

#include "vcc.h"

VCalBaseConduit::VCalBaseConduit(BaseConduit::eConduitMode mode) :
	BaseConduit(mode)
{
	FUNCTIONSETUP;
	
	fCalendar = 0L;
}

/* virtual */ VCalBaseConduit::~VCalBaseConduit()
{
	FUNCTIONSETUP;
	
	if (fCalendar)
	{
		cleanVObject(fCalendar);
		cleanStrTbl();
	}
	
	fCalendar=0L;
}

bool VCalBaseConduit::getCalendar(const QString &group)
{
	FUNCTIONSETUP;

	if (fCalendar)
	{
		kdWarning() << __FUNCTION__
			<< ": Already have a calendar file."
			<< endl;
		return true;
	}

	KConfig& config = KPilotConfig::getConfig(group);
	(void) getDebugLevel(config);
	calName = config.readEntry("CalFile");
	fFirstTime = getFirstTime(config);

	DEBUGCONDUIT << fname
		<< ": Calendar file is " << calName
		<< ( fFirstTime ? " (first time!)" : "" )
		<< endl;

	QCString s=QFile::encodeName(calName);

	fCalendar = Parse_MIME_FromFileName((const char*)s);

	if(fCalendar == 0L)
	{
		kdError() << __FUNCTION__
			<< ": Couldn't open "
			<< calName
			<< endl;

		return false;
	}
	else
	{
		DEBUGCONDUIT << fname
			<< ": Got calendar!"
			<< endl;
		return true;
	}
}


void VCalBaseConduit::saveVCal()
{
	FUNCTIONSETUP;

	if (fCalendar)
	{
		QCString filename(QFile::encodeName(calName));
		writeVObjectToFile(const_cast<char *>((const char *)filename), fCalendar);
	}
}



void VCalBaseConduit::noCalendarError(const QString &conduitName)
{
	QString message = i18n(
		"The %1 could not open the file `%2'. "
		"Please configure the conduit with the correct "
		"filename and try again.")
		.arg(conduitName)
		.arg(calName);
		
	KMessageBox::error(0, message,
		i18n("%1 Fatal Error").arg(conduitName));
}
	
void VCalBaseConduit::setSummary(VObject *vevent,const char *summary)
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
				fakeUnicode(qsummary.utf8(),0));
		}
		else
		{
			addPropValue(vevent, VCSummaryProp,
				qsummary.utf8());
		}
	}
}

void VCalBaseConduit::setNote(VObject *vevent,const char *s)
{
	FUNCTIONSETUP;

	VObject *vo = isAPropertyOf(vevent, VCDescriptionProp);
	
	if (s && *s)
	{
		QString qnote (s);

		// There is a note for this event
		//
		//
		if (vo)
		{
			setVObjectUStringZValue_(vo,
				fakeUnicode(qnote.utf8(),0));
		}
		else
		{
			vo = addPropValue(vevent, VCDescriptionProp,
				qnote.utf8());
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

int VCalBaseConduit::getTimeZone() const
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

/*
 * Given an pilot id, search the vCalendar for a matching vobject, and return
 * the pointer to that object.  If not found, return NULL.
 */
VObject* VCalBaseConduit::findEntryInCalendar(unsigned int id)
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


void VCalBaseConduit::setSecret(VObject *vevent,bool secret)
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

void VCalBaseConduit::setStatus(VObject *vevent,int status)
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

void VCalBaseConduit::addDateProperty(VObject *vevent,
	const char *prop,
	const QDateTime& dt,
	bool truncateTime)
{
	QString dateString = TmToISO(dt,truncateTime);
	addPropValue(vevent, prop, dateString.latin1());
}

void VCalBaseConduit::addDateProperty(VObject *vevent,
	const char *prop,
	const struct tm *t,
	bool truncateTime)
{
	QString dateString = TmToISO(t,truncateTime);
	addPropValue(vevent, prop, dateString.latin1());
}

void VCalBaseConduit::setDateProperty(VObject *vevent,
	const QDateTime& dt,
	bool truncateTime)
{
	setVObjectUStringZValue_(vevent,
		fakeUnicode(TmToISO(dt,truncateTime).latin1(), 0));
}

void VCalBaseConduit::setDateProperty(VObject *vevent,
	const struct tm *p,
	bool truncateTime)
{
	setVObjectUStringZValue_(vevent,
		fakeUnicode(TmToISO(p,truncateTime).latin1(), 0));
}



QString VCalBaseConduit::TmToISO(const QDateTime &dt,
	bool truncateTime)
{
	FUNCTIONSETUP;

	QString dateString;

	if (truncateTime)
	{
	dateString.sprintf("%.4d%.2d%.2dT000000",
		dt.date().year(), dt.date().month(), dt.date().day());
	}
	else
	{
	dateString.sprintf("%.4d%.2d%.2dT%.2d%.2d%.2d",
		dt.date().year(), dt.date().month(), dt.date().day(), 
		dt.time().hour(), dt.time().minute(), dt.time().second());
	}

	return dateString;
}


QString VCalBaseConduit::TmToISO(const struct tm *tm,
	bool truncateTime)
{
  QString dStr;

  if (truncateTime)
  {
  dStr.sprintf("%.4d%.2d%.2dT000000",
	       1900 + tm->tm_year,
	       tm->tm_mon + 1,
	       tm->tm_mday);
  }
  else
  {
  dStr.sprintf("%.4d%.2d%.2dT%.2d%.2d%.2d",
	       1900 + tm->tm_year,
	       tm->tm_mon + 1,
	       tm->tm_mday,
	       tm->tm_hour,
	       tm->tm_min,
	       tm->tm_sec);
  }

  return dStr;
}

/* static */ struct tm VCalBaseConduit::ISOToTm(const QString &tStr,int timeZone)
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


int VCalBaseConduit::numFromDay(const QString &day)
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

/*
 * The pilot record specified was deleted on the pilot.  Remove
 * the corresponding vobject from the vCalendar.
 */
void VCalBaseConduit::deleteVObject(PilotRecord *rec)
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

// $Log$
// Revision 1.3  2001/04/01 17:32:06  adridg
// Fiddling around with date properties
//
// Revision 1.2  2001/03/24 16:11:06  adridg
// Fixup some date-to-vcs functions
//
// Revision 1.1  2001/03/10 18:26:04  adridg
// Refactored vcal conduit and todo conduit
//

