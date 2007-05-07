/* Todo-Conduit for syncing KPilot and KOrganizer
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown <pbrown@kde.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qdatetime.h>
#include <qtextcodec.h>

#include <libkcal/calendar.h>
#include <libkcal/todo.h>

#include <pilotLocalDatabase.h>

#include "todo-conduit.moc"
#include "vcalconduitSettings.h"
#include "todo-factory.h"

#include "kcalRecord.h"
#include "todoRecord.h"

// define conduit versions, one for the version when categories were synced for the first time, and the current version number
#define CONDUIT_VERSION_CATEGORYSYNC 10
#define CONDUIT_VERSION 10

extern "C"
{
unsigned long version_conduit_todo = Pilot::PLUGIN_API;
}


TodoConduitPrivate::TodoConduitPrivate(KCal::Calendar *b) :
	VCalConduitPrivateBase(b)
{
	fAllTodos.setAutoDelete(false);
}

void TodoConduitPrivate::addIncidence(KCal::Incidence*e)
{
	fAllTodos.append(static_cast<KCal::Todo*>(e));
	fCalendar->addTodo(static_cast<KCal::Todo*>(e));
}

int TodoConduitPrivate::updateIncidences()
{
	fAllTodos = fCalendar->todos();
	fAllTodos.setAutoDelete(false);
	return fAllTodos.count();
}


void TodoConduitPrivate::removeIncidence(KCal::Incidence *e)
{
	fAllTodos.remove(static_cast<KCal::Todo*>(e));
	if (!fCalendar) return;
	fCalendar->deleteTodo(static_cast<KCal::Todo*>(e));
	// now just in case we're in the middle of reading through our list
	// and we delete something, set reading to false so we start at the
	// top again next time and don't have problems with our iterator
	reading = false;
}



KCal::Incidence *TodoConduitPrivate::findIncidence(recordid_t id)
{
	KCal::Todo::List::ConstIterator it;
        for( it = fAllTodos.begin(); it != fAllTodos.end(); ++it ) {
                KCal::Todo *todo = *it;
		if ((recordid_t)(todo->pilotId()) == id) return todo;
	}

	return 0L;
}



KCal::Incidence *TodoConduitPrivate::findIncidence(PilotRecordBase *tosearch)
{
	PilotTodoEntry*entry=dynamic_cast<PilotTodoEntry*>(tosearch);
	if (!entry) return 0L;

	QString title=entry->getDescription();
	QDateTime dt=readTm( entry->getDueDate() );

	KCal::Todo::List::ConstIterator it;
        for( it = fAllTodos.begin(); it != fAllTodos.end(); ++it ) {
                KCal::Todo *event = *it;
		if ( (event->dtDue().date() == dt.date()) && (event->summary() == title) ) return event;
	}
	return 0L;
}



KCal::Incidence *TodoConduitPrivate::getNextIncidence()
{
	FUNCTIONSETUP;
	if (reading) {
		++fAllTodosIterator;
	}
	else {
		reading=true;
		fAllTodosIterator = fAllTodos.begin();
	}

	return(fAllTodosIterator == fAllTodos.end()) ? 0L : *fAllTodosIterator;
}



KCal::Incidence *TodoConduitPrivate::getNextModifiedIncidence()
{
	FUNCTIONSETUP;
	KCal::Todo*e=0L;
	if (!reading)
	{
		reading=true;
		fAllTodosIterator = fAllTodos.begin();
	}
	else
	{
		++fAllTodosIterator;
	}
	if ( fAllTodosIterator != fAllTodos.end() ) e=*fAllTodosIterator;
	while (fAllTodosIterator != fAllTodos.end() &&
		e && e->syncStatus()!=KCal::Incidence::SYNCMOD && e->pilotId())
	{
		e = (++fAllTodosIterator != fAllTodos.end()) ? *fAllTodosIterator : 0L;

#ifdef DEBUG
	if(e)
		DEBUGKPILOT<< e->summary()<<" had SyncStatus="<<e->syncStatus()<<endl;
#endif

	}

	return (fAllTodosIterator == fAllTodos.end()) ? 0L : *fAllTodosIterator;
}



/****************************************************************************
 *                          TodoConduit class                               *
 ****************************************************************************/

TodoConduit::TodoConduit(KPilotLink *d,
	const char *n,
	const QStringList &a) : VCalConduitBase(d,n,a),
	fTodoAppInfo( 0L )
{
	FUNCTIONSETUP;
	fConduitName=i18n("To-do");
}



TodoConduit::~TodoConduit()
{
//	FUNCTIONSETUP;
}



void TodoConduit::_setAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information

	if( !fTodoAppInfo )
	{
		DEBUGKPILOT << fname << ": fTodoAppInfo is NULL" << endl;
		return;
	}
	if( !fDatabase )
	{
		DEBUGKPILOT << fname << ": fDatabase is NULL" << endl;
		return;
	}

	fTodoAppInfo->writeTo(fDatabase);
}

void TodoConduit::_getAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information

	KPILOT_DELETE( fTodoAppInfo );
	fTodoAppInfo = new PilotToDoInfo(fDatabase);
	fTodoAppInfo->dump();
}



const QString TodoConduit::getTitle(PilotRecordBase *de)
{
	PilotTodoEntry*d=dynamic_cast<PilotTodoEntry*>(de);
	if (d)
	{
		return QString(d->getDescription());
	}
	return QString::null;
}



void TodoConduit::readConfig()
{
	FUNCTIONSETUP;
	VCalConduitBase::readConfig();
	// determine if the categories have ever been synce. Needed to prevent loosing
	// the categories on the desktop. Also use a full sync for the first time to
	// make sure the palm categories are really transferred to the desktop.
	//
	categoriesSynced = config()->conduitVersion()>=CONDUIT_VERSION_CATEGORYSYNC;
	if (!categoriesSynced && !isFullSync() )
	{
		changeSync(SyncMode::eFullSync);
	}
	DEBUGKPILOT<<"categoriesSynced=" << categoriesSynced << endl;
}

void TodoConduit::preSync()
{
	FUNCTIONSETUP;
	VCalConduitBase::preSync();
	_getAppInfo();
}

void TodoConduit::postSync()
{
	FUNCTIONSETUP;
	VCalConduitBase::postSync();
	// after this successful sync the categories have been synced for sure
	config()->setConduitVersion( CONDUIT_VERSION );
	config()->writeConfig();
	_setAppInfo();
}



PilotRecord *TodoConduit::recordFromIncidence(PilotRecordBase *de, const KCal::Incidence *e)
{
	FUNCTIONSETUP;

	if (!de || !e)
	{
		DEBUGKPILOT << fname
			<< ": got NULL entry or NULL incidence." << endl;
		return 0L;
	}

	PilotTodoEntry *todoEntry = dynamic_cast<PilotTodoEntry*>(de);
	if (!todoEntry)
	{
		// Secretly wasn't a todo entry after all
		return 0L;
	}

	const KCal::Todo *todo = dynamic_cast<const KCal::Todo *>(e);
	if (!todo)
	{
		DEBUGKPILOT << fname << ": Incidence is not a todo." << endl;
		return 0L;
	}

	// don't need to check for null pointers here, the recordFromIncidence(PTE*, KCal::Todo*) will do that.
	if (KCalSync::setTodoEntry(todoEntry,todo,*fTodoAppInfo->categoryInfo()))
	{
		return todoEntry->pack();
	}
	else
	{
		return 0L;
	}
}

KCal::Incidence *TodoConduit::incidenceFromRecord(KCal::Incidence *e, const PilotRecordBase *de)
{
	FUNCTIONSETUP;

	if (!de || !e)
	{
		DEBUGKPILOT << fname
			<< ": Got NULL entry or NULL incidence." << endl;
		return 0L;
	}

	const PilotTodoEntry *todoEntry = dynamic_cast<const PilotTodoEntry *>(de);
	if (!todoEntry)
	{
		DEBUGKPILOT << fname << ": HH record not a todo entry." << endl;
		return 0L;
	}

	KCal::Todo *todo = dynamic_cast<KCal::Todo *>(e);
	if (!todo)
	{
		DEBUGKPILOT << fname << ": Incidence is not a todo." << endl;
		return 0L;
	}

	KCalSync::setTodo(todo, todoEntry,*fTodoAppInfo->categoryInfo());
	return e;
}





void TodoConduit::preRecord(PilotRecord*r)
{
	FUNCTIONSETUP;
	if (!categoriesSynced && r)
	{
		const PilotRecordBase *de = newPilotEntry(r);
		KCal::Incidence *e = fP->findIncidence(r->id());
		KCalSync::setCategory(dynamic_cast<KCal::Todo*>(e),
			dynamic_cast<const PilotTodoEntry*>(de),
			*fTodoAppInfo->categoryInfo());
	}
}






static VCalConduitSettings *config_vcal = 0L;

VCalConduitSettings *TodoConduit::theConfig() {
	if (!config_vcal)
	{
		config_vcal = new VCalConduitSettings(CSL1("Calendar"));
	}

	return config_vcal;
}

VCalConduitSettings *TodoConduit::config() {
	return theConfig();
}
