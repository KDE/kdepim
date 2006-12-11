#ifndef _KPILOT_VCALRECORD_H
#define _KPILOT_VCALRECORD_H
/* vcal-conduit.h                       KPilot
**
** Copyright (C) 2006 by Adriaan de Groot <groot@kde.org>
** Copyright (C) 2002-2003 by Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
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
** Bug reports and questions can be sent to kde-pim@kde.org
*/

namespace KCal
{
	class Event;
}

class PilotDateEntry;

class VCalRecord
{
public:
	static bool setEvent( KCal::Event *e, const PilotDateEntry *de);
	static bool setDateEntry(PilotDateEntry *de, const KCal::Event *e);

protected:
	static void setStartEndTimes(KCal::Event *,const PilotDateEntry *);
	static void setAlarms(KCal::Event *,const PilotDateEntry *);
	static void setRecurrence(KCal::Event *,const PilotDateEntry *);
	static void setExceptions(KCal::Event *,const PilotDateEntry *);
	static void setCategory(KCal::Event *, const PilotDateEntry *);

	static void setStartEndTimes(PilotDateEntry *, const KCal::Event * );
	static void setAlarms(PilotDateEntry *, const KCal::Event * );
	static void setRecurrence(PilotDateEntry *, const KCal::Event * );
	static void setExceptions(PilotDateEntry *, const KCal::Event * );
	static void setCategory(PilotDateEntry *, const KCal::Event *);

} ;

#endif


