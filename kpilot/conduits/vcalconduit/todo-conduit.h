#ifndef _KPILOT_TODO_CONDUIT_H
#define _KPILOT_TODO_CONDUIT_H
/* todo-conduit.h                       KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown <pbrown@kde.org>
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <libkcal/todo.h>
#include <pilotTodoEntry.h>
#include "vcal-conduitbase.h"

class PilotRecord;
class PilotSerialDatabase;
class PilotLocalDatabase;
class PilotTodoEntry;

class TodoConduitPrivate : public VCalConduitPrivateBase
{
public:
	TodoConduitPrivate(KCal::Calendar *buddy);
	virtual ~TodoConduitPrivate() {};

	KCal::Todo::List fAllTodos;
	KCal::Todo::List::ConstIterator fAllTodosIterator;

	virtual int updateIncidences();
	virtual void addIncidence(KCal::Incidence*);
	virtual void removeIncidence(KCal::Incidence *);
	virtual KCal::Incidence *findIncidence(recordid_t);
	virtual KCal::Incidence *findIncidence(PilotAppCategory*tosearch);
	virtual KCal::Incidence *getNextIncidence();
	virtual KCal::Incidence *getNextModifiedIncidence();
	virtual int count() {return fAllTodos.count();};
} ;



class TodoConduit : public VCalConduitBase
{
Q_OBJECT
public:
	TodoConduit(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~TodoConduit();

protected:
	virtual const QString getTitle(PilotAppCategory*de);

	virtual const QString dbname() { return CSL1("ToDoDB"); };
	virtual void preSync() {_getAppInfo(); };
	virtual VCalConduitPrivateBase* newVCalPrivate(KCal::Calendar *fCalendar) {
		return new TodoConduitPrivate(fCalendar);
	};

	virtual void readConfig();
	void _getAppInfo();
	void _setAppInfo();
	virtual void postSync();
	QString _getCat(const QStringList cats, const QString curr) const;

	virtual PilotAppCategory*newPilotEntry(PilotRecord*r) {
		FUNCTIONSETUP;
		if (r) return new PilotTodoEntry(fTodoAppInfo, r);
		else return new PilotTodoEntry(fTodoAppInfo);
	};
	virtual KCal::Incidence*newIncidence() { return new KCal::Todo; };

	virtual void preRecord(PilotRecord*r);
	virtual VCalConduitSettings *config();

protected:

	PilotRecord *recordFromIncidence(PilotAppCategory*de, const KCal::Incidence*e);
	PilotRecord *recordFromTodo(PilotTodoEntry*de, const KCal::Todo*e);
	KCal::Incidence *incidenceFromRecord(KCal::Incidence *, const PilotAppCategory *);
	KCal::Todo *incidenceFromRecord(KCal::Todo *, const PilotTodoEntry *);

	void setCategory(PilotTodoEntry*de, const KCal::Todo*todo);
	void setCategory(KCal::Todo*todo, const PilotTodoEntry*de);

	struct ToDoAppInfo fTodoAppInfo;
	bool categoriesSynced;
} ;

#endif
