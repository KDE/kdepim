/* todo-conduit.cc  Todo-Conduit for syncing KPilot and KOrganizer
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
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

// define conduit versions, one for the version when categories were synced for the first time, and the current version number
#define CONDUIT_VERSION_CATEGORYSYNC 10
#define CONDUIT_VERSION 10

extern "C"
{
long version_conduit_todo = KPILOT_PLUGIN_API;

const char *id_conduit_todo = "$Id$";

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
	fCalendar->deleteTodo(static_cast<KCal::Todo*>(e));
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



KCal::Incidence *TodoConduitPrivate::findIncidence(PilotAppCategory*tosearch)
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
		if ( fAllTodosIterator != fAllTodos.end() ) e=*fAllTodosIterator;
	}
	else
	{
		++fAllTodosIterator;
	}
	while (fAllTodosIterator != fAllTodos.end() &&
		e && e->syncStatus()!=KCal::Incidence::SYNCMOD)
	{
		e = (++fAllTodosIterator != fAllTodos.end()) ? *fAllTodosIterator : 0L;

#ifdef DEBUG
	if(e)
		DEBUGCONDUIT<< e->summary()<<" had SyncStatus="<<e->syncStatus()<<endl;
#endif

	}

	return (fAllTodosIterator == fAllTodos.end()) ? 0L : *fAllTodosIterator;
}



/****************************************************************************
 *                          TodoConduit class                               *
 ****************************************************************************/

TodoConduit::TodoConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &a) : VCalConduitBase(d,n,a)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << id_conduit_todo << endl;
#endif
	fConduitName=i18n("To-do");
        
        (void) id_conduit_todo;
}



TodoConduit::~TodoConduit()
{
//	FUNCTIONSETUP;
}



void TodoConduit::_setAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information
	int appLen = pack_ToDoAppInfo(&fTodoAppInfo, 0, 0);
	unsigned char *buffer = new unsigned char[appLen];
	pack_ToDoAppInfo(&fTodoAppInfo, buffer, appLen);
	if (fDatabase) fDatabase->writeAppBlock(buffer, appLen);
	if (fLocalDatabase) fLocalDatabase->writeAppBlock(buffer, appLen);
	delete[] buffer;
}
void TodoConduit::_getAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information
	unsigned char *buffer =
		new unsigned char[PilotRecord::APP_BUFFER_SIZE];
	int appLen = fDatabase->readAppBlock(buffer,PilotRecord::APP_BUFFER_SIZE);

	unpack_ToDoAppInfo(&fTodoAppInfo, buffer, appLen);
	delete[]buffer;
	buffer = NULL;

#ifdef DEBUG
	DEBUGCONDUIT << fname << " lastUniqueId"
		<< fTodoAppInfo.category.lastUniqueID << endl;
#endif
	for (int i = 0; i < 16; i++)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << " cat " << i << " =" <<
			fTodoAppInfo.category.name[i] << endl;
#endif
	}

}



const QString TodoConduit::getTitle(PilotAppCategory*de)
{
	PilotTodoEntry*d=dynamic_cast<PilotTodoEntry*>(de);
	if (d) return QString(d->getDescription());
	return QString::null;
}



void TodoConduit::readConfig()
{
	FUNCTIONSETUP;
	VCalConduitBase::readConfig();
	// determine if the categories have ever been synce. Needed to prevent loosing the categories on the desktop.
	// also use a full sync for the first time to make sure the palm categories are really transferred to the desktop
	categoriesSynced = config()->conduitVersion()>=CONDUIT_VERSION_CATEGORYSYNC;
	if (!categoriesSynced & !isFullSync() ) 
		setSyncDirection(SyncAction::eFullSync);
#ifdef DEBUG
	DEBUGCONDUIT<<"categoriesSynced="<<categoriesSynced<<endl;
#endif
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



PilotRecord*TodoConduit::recordFromIncidence(PilotAppCategory*de, const KCal::Incidence*e)
{
	// don't need to check for null pointers here, the recordFromIncidence(PTE*, KCal::Todo*) will do that.
	PilotTodoEntry *tde = dynamic_cast<PilotTodoEntry*>(de);
	const KCal::Todo *te = dynamic_cast<const KCal::Todo*>(e);

	return recordFromTodo(tde, te);
}



PilotRecord*TodoConduit::recordFromTodo(PilotTodoEntry*de, const KCal::Todo*todo)
{
	FUNCTIONSETUP;
	if (!de || !todo) {
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": NULL todo given... Skipping it"<<endl;
#endif
		return NULL;
	}

	// set secrecy, start/end times, alarms, recurrence, exceptions, summary and description:
	if (todo->secrecy()!=KCal::Todo::SecrecyPublic) de->makeSecret();

	// update it from the iCalendar Todo.

	if (todo->hasDueDate()) {
		struct tm t = writeTm(todo->dtDue());
		de->setDueDate(t);
		de->setIndefinite(0);
	} else {
		de->setIndefinite(1);
	}

	// TODO: take recurrence (code in VCAlConduit) from ActionNames

	setCategory(de, todo);

	// TODO: sync the alarm from ActionNames. Need to extend PilotTodoEntry
	de->setPriority(todo->priority());

	de->setComplete(todo->isCompleted());

	// what we call summary pilot calls description.
	de->setDescription(todo->summary());

	// what we call description pilot puts as a separate note
	de->setNote(todo->description());

#ifdef DEBUG
DEBUGCONDUIT<<"-------- "<<todo->summary()<<endl;
#endif
	return de->pack();
}



void TodoConduit::preRecord(PilotRecord*r)
{
	FUNCTIONSETUP;
	if (!categoriesSynced && r)
	{
		const PilotAppCategory*de=newPilotEntry(r);
		KCal::Incidence *e = fP->findIncidence(r->id());
		setCategory(dynamic_cast<KCal::Todo*>(e), dynamic_cast<const PilotTodoEntry*>(de));
	}
}



void TodoConduit::setCategory(PilotTodoEntry*de, const KCal::Todo*todo)
{
	if (!de || !todo) return;
	de->setCategory(_getCat(todo->categories(), de->getCategoryLabel()));
}



/**
 * _getCat returns the id of the category from the given categories list. If none of the categories exist
 * on the palm, the "Nicht abgelegt" (don't know the english name) is used.
 */
QString TodoConduit::_getCat(const QStringList cats, const QString curr) const
{
	int j;
	if (cats.size()<1) return QString::null;
	if (cats.contains(curr)) return curr;
	for ( QStringList::ConstIterator it = cats.begin(); it != cats.end(); ++it ) {
		for (j=1; j<=15; j++)
		{
			QString catName = PilotAppCategory::codec()->
				toUnicode(fTodoAppInfo.category.name[j]);
			if (!(*it).isEmpty() && !(*it).compare( catName ) )
			{
				return catName;
			}
		}
	}
	// If we have a free label, return the first possible cat
	QString lastName(fTodoAppInfo.category.name[15]);
	if (lastName.isEmpty()) return cats.first();
	return QString::null;
}



KCal::Incidence *TodoConduit::incidenceFromRecord(KCal::Incidence *e, const PilotAppCategory *de)
{
	return dynamic_cast<KCal::Incidence*>(incidenceFromRecord(dynamic_cast<KCal::Todo*>(e), dynamic_cast<const PilotTodoEntry*>(de)));
}



KCal::Todo *TodoConduit::incidenceFromRecord(KCal::Todo *e, const PilotTodoEntry *de)
{
	FUNCTIONSETUP;

	KCal::Todo*vtodo=e;
	if (!vtodo)
	{
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": null todo entry given. skipping..."<<endl;
#endif
		return NULL;
	}

   // We don't want this, do we?
//	e->setOrganizer(fCalendar->getEmail());
	e->setPilotId(de->id());
	e->setSyncStatus(KCal::Incidence::SYNCNONE);
	e->setSecrecy(de->isSecret() ? KCal::Todo::SecrecyPrivate : KCal::Todo::SecrecyPublic);

	// we don't want to modify the vobject with pilot info, because it has
	// already been  modified on the desktop.  The VObject's modified state
	// overrides the PilotRec's modified state.
	// TODO: Also include this in the vcal conduit!!!
//	if (e->syncStatus() != KCal::Incidence::SYNCNONE) return e;

	// otherwise, the vObject hasn't been touched.  Updated it with the
	// info from the PilotRec.
	if (de->getIndefinite()) {
		e->setHasDueDate(false);
	} else {
		e->setDtDue(readTm(de->getDueDate()));
		e->setHasDueDate(true);
	}

	// Categories
	// TODO: Sync categories
	// first remove all categories and then add only the appropriate one
	setCategory(e, de);

	// PRIORITY //
	e->setPriority(de->getPriority());

	// COMPLETED? //
	e->setCompleted(de->getComplete());
	if ( de->getComplete() && !e->hasCompletedDate() ) {
		e->setCompleted( QDateTime::currentDateTime() );
	}

	e->setSummary(de->getDescription());
	e->setDescription(de->getNote());

	e->setSyncStatus(KCal::Incidence::SYNCNONE);

	return e;
}



void TodoConduit::setCategory(KCal::Todo *e, const PilotTodoEntry *de)
{
	if (!e || !de) return;
	QStringList cats=e->categories();
	int cat=de->getCat();
	if (0<cat && cat<=15)
	{
		QString newcat=PilotAppCategory::codec()->toUnicode(fTodoAppInfo.category.name[cat]);
		if (!cats.contains(newcat))
		{
			cats.append( newcat );
			e->setCategories(cats);
		}
	}
}

VCalConduitSettings *TodoConduit::config()
{ 
  return ToDoConduitFactory::config(); 
}
