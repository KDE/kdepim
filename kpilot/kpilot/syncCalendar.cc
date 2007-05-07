/* KPilot
**
** Copyright (C) 2005 by Adriaan de Groot <groot@kde.org>
**
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/

#include "options.h"

#include <kpilotlink.h>
#include <pilotDatabase.h>
#include <pilotDateEntry.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/event.h>


typedef DatabaseInterpreter<KCal::Event,PilotDateEntry,PilotDateEntry::Mapper> DatebookDB;

class CalendarDumper
{
public:
	CalendarDumper() {}

	KCal::CalendarLocal *retrieve(int fd);
	KCal::CalendarLocal *retrieve(const QString &filename);

protected:
	KCal::CalendarLocal *retrieve(DatebookDB *db);
} ;



KCal::CalendarLocal *CalendarDumper::retrieve(int fd)
{
	PilotSerialDatabase sdb( fd, CSL1("DatebookDB") );
	DatebookDB db(&sdb);
	return retrieve(db);
}

KCal::CalendarLocal *CalendarDumped::retrieve(const QString &fn)
{
	PilotLocalDatabase ldb( fn );
	DatebookDB db(&ldb);
	return retrieve(db);
}

KCal::CalendarLocal *retrieve(DatebookDB *db)
{
	KCal::CalendarLocal *cal = new CalendarLocal( QString::null );

	int count = db->db()->recordCount();

	if (count < 1)
	{
		return cal;
	}

	for (int i=0; i<count; i++)
	{
		KCal::Event *e = db->readRecordByIndex(i);
		if (!e) continue;
		cal->addEvent(e);
	}

	return cal;
}
