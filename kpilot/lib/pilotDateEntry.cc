/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This is a C++ wrapper for the Pilot's datebook structures.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <stdlib.h>

#include <qdatetime.h>
#include <qnamespace.h>
#include <qregexp.h>

#include <kglobal.h>

#include "pilotDateEntry.h"

static const char *default_date_category_names[] = {
	"Unfiled",
	"Business",
	"Personal",
	0L
} ;

void PilotDateInfo::resetToDefault()
{
	FUNCTIONSETUP;
	// Reset to all 0s
	memset(&fInfo,0,sizeof(fInfo));
	// Fill up default categories
	for (unsigned int i=0; (i<4) && default_date_category_names[i]; ++i)
	{
		strncpy(fInfo.category.name[i],default_date_category_names[i],sizeof(fInfo.category.name[0]));
	}

	fInfo.startOfWeek = 0;

}


PilotDateEntry::PilotDateEntry():PilotRecordBase()
{
	::memset(&fAppointmentInfo, 0, sizeof(struct Appointment));
}

/* initialize the entry from another one. If rec==NULL, this constructor does the same as PilotDateEntry()
*/
PilotDateEntry::PilotDateEntry(PilotRecord * rec) :
	PilotRecordBase(rec)
{
	::memset(&fAppointmentInfo, 0, sizeof(fAppointmentInfo));
	if (rec)
	{
		// Construct a fake pi_buffer for unpack_Appointment.
		// No ownership changes occur here.
		pi_buffer_t b = { (unsigned char *) rec->data(), rec->size(), rec->size() } ;
		unpack_Appointment(&fAppointmentInfo, &b, datebook_v1);
	}
	return;

}

void PilotDateEntry::_copyExceptions(const PilotDateEntry & e)
{
	if (e.fAppointmentInfo.exceptions > 0)
	{
		size_t blocksize = e.fAppointmentInfo.exceptions *
			sizeof(struct tm);

		fAppointmentInfo.exception = (struct tm *)::malloc(blocksize);

		if (fAppointmentInfo.exception)
		{
			fAppointmentInfo.exceptions =
				e.fAppointmentInfo.exceptions;
			::memcpy(fAppointmentInfo.exception,
				e.fAppointmentInfo.exception, blocksize);
		}
		else
		{
			WARNINGKPILOT << "malloc() failed, exceptions not copied" << endl;
			fAppointmentInfo.exceptions = 0;
		}
	}
	else
	{
		fAppointmentInfo.exceptions = 0;
		fAppointmentInfo.exception = 0L;
	}
}


PilotDateEntry::PilotDateEntry(const PilotDateEntry & e) :
	PilotRecordBase(e)
{
	::memcpy(&fAppointmentInfo, &e.fAppointmentInfo,
		sizeof(struct Appointment));
	// See operator = for explanation
	fAppointmentInfo.exception = 0L;
	fAppointmentInfo.description = 0L;
	fAppointmentInfo.note = 0L;

	_copyExceptions(e);
	setDescriptionP(e.fAppointmentInfo.description);
	setNoteP(e.fAppointmentInfo.note);
}


PilotDateEntry & PilotDateEntry::operator = (const PilotDateEntry & e)
{
	if (this != &e)		// Pointer equality!
	{
		KPILOT_FREE(fAppointmentInfo.exception);
		KPILOT_FREE(fAppointmentInfo.description);
		KPILOT_FREE(fAppointmentInfo.note);
		::memcpy(&fAppointmentInfo, &e.fAppointmentInfo,
			sizeof(fAppointmentInfo));

		// The original pointers were already freed; since we're now
		// got the pointers from the new structure and we're going
		// to use the standard set functions make sure that
		// we don't free() the copies-of-pointers from e, which
		// would be disastrous.
		//
		//
		fAppointmentInfo.exception = 0L;
		fAppointmentInfo.description = 0L;
		fAppointmentInfo.note = 0L;

		_copyExceptions(e);
		setDescriptionP(e.fAppointmentInfo.description);
		setNoteP(e.fAppointmentInfo.note);
	}

	return *this;
}				// end of assignment operator


QString PilotDateEntry::getTextRepresentation(Qt::TextFormat richText)
{
	QString text, tmp;
	QString par = (richText==Qt::RichText) ?CSL1("<p>"):QString::null;
	QString ps = (richText==Qt::RichText) ?CSL1("</p>"):CSL1("\n");
	QString br = (richText==Qt::RichText) ?CSL1("<br/>"):CSL1("\n");

	// title + name
	text += par;
	tmp=richText?CSL1("<b><big>%1</big></b>"):CSL1("%1");
	text += tmp.arg(rtExpand(getDescription(), richText));
	text += ps;

	QDateTime dt(readTm(getEventStart()));
	QString startDate(dt.toString(Qt::LocalDate));
	text+=par;
	text+=i18n("Start date: %1").arg(startDate);
	text+=ps;

	if (isEvent())
	{
		text+=par;
		text+=i18n("Whole-day event");
		text+=ps;
	}
	else
	{
		dt=readTm(getEventEnd());
		QString endDate(dt.toString(Qt::LocalDate));
		text+=par;
		text+=i18n("End date: %1").arg(endDate);
		text+=ps;
	}

	if ( isAlarmEnabled() )
	{
		text+=par;
		tmp=i18n("%1 is the duration, %2 is the time unit", "Alarm: %1 %2 before event starts").
			arg(getAdvance());
		switch (getAdvanceUnits())
		{
			case advMinutes: tmp=tmp.arg(i18n("minutes")); break;
			case advHours: tmp=tmp.arg(i18n("hours")); break;
			case advDays: tmp=tmp.arg(i18n("days")); break;
			default: tmp=tmp.arg(QString::null); break;;
		}
		text+=tmp;
		text+=ps;
	}

	if (getRepeatType() != repeatNone)
	{
		text+=par;
		tmp=i18n("Recurrence: every %1 %2");
		int freq = getRepeatFrequency();
		tmp=tmp.arg(freq);

		switch(getRepeatType())
		{
			case repeatDaily: tmp=tmp.arg(i18n("day(s)")); break;
			case repeatWeekly: tmp=tmp.arg(i18n("week(s)")); break;
			case repeatMonthlyByDay:
			case repeatMonthlyByDate: tmp=tmp.arg(i18n("month(s)")); break;
			case repeatYearly: tmp=tmp.arg(i18n("year(s)")); break;
			default: tmp=tmp.arg(QString::null); break;
		}
		text+=tmp;
		text+=br;

		bool repeatsForever = getRepeatForever();
		if (repeatsForever)
		{
			text+=i18n("Repeats indefinitely");
		}
		else
		{
			dt = readTm(getRepeatEnd()).date();
			text+=i18n("Until %1").arg(dt.toString(Qt::LocalDate));
		}
		text+=br;

		if (getRepeatType()==repeatMonthlyByDay) text+=i18n("Repeating on the i-th day of week j")+br;
		if (getRepeatType()==repeatMonthlyByDate) text+=i18n("Repeating on the n-th day of the month")+br;
		// TODO: show the dayArray when repeating weekly
		/*QBitArray dayArray(7);
		if (getRepeatType()==repeatWeekly) text+=i18n("Repeat day flags: %1").arg(getRepeatDays
		const int *days = dateEntry->getRepeatDays();
		// Rotate the days of the week, since day numbers on the Pilot and
		// in vCal / Events are different.
		if (days[0]) dayArray.setBit(6);
		for (int i = 1; i < 7; i++)
		{
			if (days[i]) dayArray.setBit(i-1);
		}*/
		text+=ps;
	}

	if (getExceptionCount()>0 )
	{
		text+=par;
		text+=i18n("Exceptions:")+br;
		for (int i = 0; i < getExceptionCount(); i++)
		{
			QDate exdt=readTm(getExceptions()[i]).date();
			text+=exdt.toString(Qt::LocalDate);
			text+=br;
		}
		text+=ps;
	}

	if (!getNote().isEmpty())
	{
		text += richText?CSL1("<hr/>"):CSL1("-------------------------\n");
		text+=par;
		text+=richText?i18n("<b><em>Note:</em></b><br>"):i18n("Note:\n");
		text+=rtExpand(getNote(), richText);
		text+=ps;
	}

	return text;
}

QDateTime PilotDateEntry::dtStart() const
{
	FUNCTIONSETUP;
	return readTm( getEventStart() );
}

QDateTime PilotDateEntry::dtEnd() const
{
	FUNCTIONSETUP;
	return readTm( getEventEnd() );
}

QDateTime PilotDateEntry::dtRepeatEnd() const
{
	FUNCTIONSETUP;
	return readTm( getRepeatEnd() );
}

unsigned int PilotDateEntry::alarmLeadTime() const
{
	FUNCTIONSETUP;
	if (!isAlarmEnabled()) return 0;

	int adv = getAdvance();
	if ( adv < 0 )
	{
		return 0; // Not possible to enter on the pilot
	}
	unsigned int t = adv;
	int u = getAdvanceUnits();


	switch(u)
	{
	case advMinutes : t *= 60; break;
	case advHours : t *= 3600; break;
	case advDays : t *= 3600 * 24; break;
	default: t = 0;
	}

	return t;
}

PilotRecord *PilotDateEntry::pack() const
{
	int i;

	pi_buffer_t *b = pi_buffer_new( sizeof(fAppointmentInfo) );
	i = pack_Appointment(const_cast<Appointment_t *>(&fAppointmentInfo), b, datebook_v1);
	if (i<0)
	{
		// Generic error from the pack_*() functions.
		return 0;
	}

	// pack_Appointment sets b->used
	return new PilotRecord( b, this );
}

/* setExceptions sets a new set of exceptions. Note that
	PilotDateEntry assumes ownership of the array and will
	delete the old one. */
void PilotDateEntry::setExceptions(struct tm *e) {
	if (fAppointmentInfo.exception != e)
	{
		KPILOT_FREE(fAppointmentInfo.exception);
	}
	fAppointmentInfo.exception=e;
}


void PilotDateEntry::setDescriptionP(const char *desc, int l)
{
	FUNCTIONSETUP;
	KPILOT_FREE(fAppointmentInfo.description);

	if (desc && *desc)
	{
		if (-1 == l) l=::strlen(desc);
		fAppointmentInfo.description =
			(char *) ::malloc(l + 1);
		if (fAppointmentInfo.description)
		{
			strlcpy(fAppointmentInfo.description, desc, l+1);
		}
		else
		{
			WARNINGKPILOT << "malloc() failed, description not set" << endl;
		}
	}
	else
	{
		fAppointmentInfo.description = 0L;
	}
}

void PilotDateEntry::setNoteP(const char *note, int l)
{
	FUNCTIONSETUP;
	KPILOT_FREE(fAppointmentInfo.note);

	if (note && *note)
	{
		if (-1 == l) l=::strlen(note);
		fAppointmentInfo.note = (char *)::malloc(l + 1);
		if (fAppointmentInfo.note)
		{
			strlcpy(fAppointmentInfo.note, note,l+1);
		}
		else
		{
			WARNINGKPILOT << "malloc() failed, note not set" << endl;
		}
	}
	else
	{
		fAppointmentInfo.note = 0L;
	}
}

void PilotDateEntry::setNote(const QString &s)
{
	QCString t = Pilot::toPilot(s);
	setNoteP( t.data(),t.length() );
}

void PilotDateEntry::setLocation(const QString &s)
{
	QString note = Pilot::fromPilot(getNoteP());
	QRegExp rxp = QRegExp("^[Ll]ocation:[^\n]+\n");

	// per QString docs, this covers null and 0 length
	if( s.isEmpty() )
	{
		note.replace(rxp,"");
	}
	else
	{
		QString location = "Location: " + s + "\n";
		int pos = note.find(rxp);

		if(pos >= 0)
		{
			note.replace( rxp, location );
		}
		else
		{
			note = location + note;
			setNote( note );
		}
	}
}

QString PilotDateEntry::getLocation() const
{
	// Read the complete note here and not the filtered
	// one from PilotDateEntry::getNote();
	QString note = Pilot::fromPilot(getNoteP());
	QRegExp rxp = QRegExp("^[Ll]ocation:[^\n]+\n");
	int pos = note.find(rxp, 0);

	if(pos >= 0)
	{
		QString location = rxp.capturedTexts().first();
		rxp = QRegExp("^[Ll]ocation:[\\s|\t]*");
		location.replace(rxp,"");
		location.replace("\n", "");
		return location;
	}
	else
	{
		return "";
	}
}

void PilotDateEntry::setDescription(const QString &s)
{
	QCString t = Pilot::toPilot(s);
	setDescriptionP( t.data(),t.length() );
}

QString PilotDateEntry::getNote() const
{
	QString note = Pilot::fromPilot(getNoteP());
	QRegExp rxp = QRegExp("^[Ll]ocation:[^\n]+\n");
	note.replace(rxp, "" );
	return note;
}

QString PilotDateEntry::getDescription() const
{
	return Pilot::fromPilot(getDescriptionP());
}

