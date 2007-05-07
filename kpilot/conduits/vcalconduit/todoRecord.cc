/* vcalRecord.cc                       KPilot
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

#include "options.h"

#include <libkcal/calendar.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/recurrence.h>
#include <libkcal/vcalformat.h>

#include "pilot.h"
#include "pilotTodoEntry.h"

#include "kcalRecord.h"
#include "todoRecord.h"

bool KCalSync::setTodoEntry(PilotTodoEntry *de,
	const KCal::Todo *todo,
	const CategoryAppInfo &info)
{
	FUNCTIONSETUP;
	if (!de || !todo) {
		DEBUGKPILOT << fname << ": NULL todo given... Skipping it" << endl;
		return false;
	}

	// set secrecy, start/end times, alarms, recurrence, exceptions, summary and description:
	if (todo->secrecy()!=KCal::Todo::SecrecyPublic)
	{
		de->setSecret( true );
	}

	// update it from the iCalendar Todo.

	if (todo->hasDueDate()) {
		struct tm t = writeTm(todo->dtDue());
		de->setDueDate(t);
		de->setIndefinite(0);
	} else {
		de->setIndefinite(1);
	}

	// TODO: take recurrence (code in VCAlConduit) from ActionNames

	setCategory(de, todo, info);

	// TODO: sync the alarm from ActionNames. Need to extend PilotTodoEntry
	de->setPriority(todo->priority());

	de->setComplete(todo->isCompleted());

	// what we call summary pilot calls description.
	de->setDescription(todo->summary());

	// what we call description pilot puts as a separate note
	de->setNote(todo->description());

	DEBUGKPILOT << "-------- " << todo->summary() << endl;
	return de->pack();
}

bool KCalSync::setTodo(KCal::Todo *e,
	const PilotTodoEntry *de,
	const CategoryAppInfo &info)
{
	FUNCTIONSETUP;

	if (!e)
	{
		DEBUGKPILOT << fname
			<< ": null todo entry given. skipping..." << endl;
		return false;
	}
	if (!de)
	{
		DEBUGKPILOT << fname
			<< "! NULL todo entry given... Skipping it" << endl;
		return false;
	}


	e->setPilotId(de->id());
	DEBUGKPILOT<<fname<<": set KCal item to pilotId: [" << e->pilotId() << "] ..."<<endl;

	e->setSecrecy(de->isSecret() ? KCal::Todo::SecrecyPrivate : KCal::Todo::SecrecyPublic);

	if (de->getIndefinite()) {
		e->setHasDueDate(false);
	} else {
		e->setDtDue(readTm(de->getDueDate()));
		e->setHasDueDate(true);
	}

	// Categories
	setCategory(e, de, info);

	// PRIORITY //
	e->setPriority(de->getPriority());

	// COMPLETED? //
	e->setCompleted(de->getComplete());
	if ( de->getComplete() && !e->hasCompletedDate() ) {
		e->setCompleted( QDateTime::currentDateTime() );
	}

	e->setSummary(de->getDescription());
	e->setDescription(de->getNote());

	// NOTE: This MUST be done last, since every other set* call
	// calls updated(), which will trigger an
	// setSyncStatus(SYNCMOD)!!!
	e->setSyncStatus(KCal::Incidence::SYNCNONE);

	return true;
}
