/* todoconduit.cc			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
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

#include "todoconduit.h"

#include <akonadi/collection.h>
#include <kcal/todo.h>

#include "idmapping.h"
#include "options.h"
#include "pilotTodoEntry.h"
#include "todoakonadiproxy.h"
#include "todoakonadirecord.h"
#include "todohhrecord.h"
#include "todohhdataproxy.h"
#include "todosettings.h"

class TodoConduit::Private
{
public:
	Private()
	{
		fCollectionId = -1;
		fPrevCollectionId = -2;
	}
	
	Akonadi::Collection::Id fCollectionId;
	Akonadi::Collection::Id fPrevCollectionId;
};

TodoConduit::TodoConduit( KPilotLink *o, const QVariantList &a )
	: RecordConduit( o, a, CSL1( "ToDoDB" ), CSL1( "To-do Conduit" ) )
	, d( new TodoConduit::Private )
{
}

TodoConduit::~TodoConduit()
{
	KPILOT_DELETE( d );
}

static int mapHHPriorityToPC( int hhPriority )
{
	FUNCTIONSETUPL(5);
	
	int pcPriority;

	if( 1 <= hhPriority && hhPriority <= 5 )
	{
		pcPriority =  2 * hhPriority - 1;
	}
	else
	{
		// hhPriority out of range so set pcPriority to undefined value
		WARNINGKPILOT << "HH Priority (" << hhPriority << ") not in range 1..5";
		pcPriority = 0;
	}

	DEBUGKPILOT << "hhPriority=" << hhPriority << ", pcPriority=" << pcPriority;

	return pcPriority;
}

static int mapPCPriorityToHH( int pcPriority )
{
	FUNCTIONSETUPL(5);
	
	int hhPriority;
	
	if( 1 <= pcPriority && pcPriority <= 9 )
	{
		hhPriority = (pcPriority + 1) / 2;
	}
	else if( pcPriority == 0 )
        {
		// force unspecified pc priority to lowest hh priority
		hhPriority = 5;
        }
	else
	{
		// pcPriority out of range, so set hh priority to lowest valid valud
		WARNINGKPILOT << "pcPriority (" << pcPriority << ") not in range 0..9";
		hhPriority = 5;
	}
	
	DEBUGKPILOT << "pcPriority=" << pcPriority << "hhPriority=" << hhPriority;
	
	return hhPriority;
}

void TodoConduit::loadSettings()
{
	FUNCTIONSETUP;
	
	TodoSettings::self()->readConfig();
	d->fCollectionId = TodoSettings::akonadiCollection();
	d->fPrevCollectionId = TodoSettings::prevAkonadiCollection();
}

bool TodoConduit::initDataProxies()
{
	FUNCTIONSETUP;
	
	if( !fDatabase )
	{
		addSyncLogEntry( i18n( "Error: Handheld database is not loaded." ) );
		return false;
	}
	
	if( d->fCollectionId < 0 )
	{
		addSyncLogEntry( i18n( "Error: No valid akonadi collection configured." ) );
		return false;
	}
	
	if( d->fPrevCollectionId != d->fCollectionId )
	{
		// TODO: Enable this in trunk.
		//addSyncLogEntry( i18n( "Note: Collection has changed since last sync, removing mapping." ) );
		DEBUGKPILOT << "Note: Collection has changed since last sync, removing mapping.";
		fMapping.remove();
	}
	
	// At this point we should be able to read the backup and handheld database.
	// However, it might be that Akonadi is not started.
	TodoAkonadiProxy* tadp = new TodoAkonadiProxy( fMapping );
	tadp->setCollectionId( d->fCollectionId );
	if( tadp->isOpen() )
	{
		tadp->loadAllRecords();
	}
	 
	fPCDataProxy = tadp;
	fHHDataProxy = new TodoHHDataProxy( fDatabase );
	fHHDataProxy->loadAllRecords();
	fBackupDataProxy = new TodoHHDataProxy( fLocalDatabase );
	fBackupDataProxy->loadAllRecords();
	//fPCDataProxy->loadAllRecords();
	
	return true;
}

void TodoConduit::syncFinished()
{
	TodoSettings::self()->readConfig();
	TodoSettings::self()->setPrevAkonadiCollection(d->fCollectionId);
	TodoSettings::self()->writeConfig();
}

bool TodoConduit::equal( const Record *pcRec, const HHRecord *hhRec ) const
{
	FUNCTIONSETUP;
	
	const TodoAkonadiRecord* tar = static_cast<const TodoAkonadiRecord*>( pcRec );
	const TodoHHRecord* thr = static_cast<const TodoHHRecord*>( hhRec );
	
	boost::shared_ptr<KCal::Todo> pcTodo
		 = boost::dynamic_pointer_cast<KCal::Todo, KCal::Incidence>( tar->item().payload<IncidencePtr>() );
	PilotTodoEntry hhTodo = thr->todoEntry();
	
	bool descriptionEqual = pcTodo->summary() == hhTodo.getDescription();
	bool noteEqual = pcTodo->description() == hhTodo.getNote();
	bool categoriesEqual = pcTodo->categories().contains( thr->category() );
	bool completeEqual = pcTodo->isCompleted() == (hhTodo.getComplete() != 0 );
	
	bool dueDateEqual;
	if( pcTodo->hasDueDate() && !hhTodo.getIndefinite() )
	{
		DEBUGKPILOT << "Both have due date. PC: [" << pcTodo->dtDue().dateTime().toLocalTime() << "], HH: ["
			    << readTm( hhTodo.getDueDate() ) << ']';
		dueDateEqual = pcTodo->dtDue().dateTime().toUTC() == readTm( hhTodo.getDueDate()).toUTC();
	}
	// This is a bit tricky when getIndefinite() returns true it means that no
	// due date is set.
	else if( pcTodo->hasDueDate() != !hhTodo.getIndefinite() )
	{
		DEBUGKPILOT << "One has and other does not have due date. PC[" 
			<< pcTodo->hasDueDate() << "], HH[" << !hhTodo.getIndefinite() << ']';
		dueDateEqual = false;
	}
	else
	{
		DEBUGKPILOT << "Both do not have duedate.";
		dueDateEqual = true;
	}
	
	// Priorities are equal if the pc priority maps correctly to the hh priority
	bool priorityEqual = mapPCPriorityToHH( pcTodo->priority() ) == hhTodo.getPriority();

	// TODO: Do some mapping for the completed percentage.
	
	DEBUGKPILOT << "descriptionEqual: " << descriptionEqual;
	DEBUGKPILOT << "noteEqual: " << noteEqual;
	DEBUGKPILOT << "categoriesEqual: " << categoriesEqual;
	DEBUGKPILOT << "dueDateEqual: " << dueDateEqual;
	DEBUGKPILOT << "completeEqual: " << completeEqual;
	DEBUGKPILOT << "priorityEqual: " << priorityEqual;
	
	return descriptionEqual
		&& noteEqual
		&& categoriesEqual
		&& dueDateEqual
		&& completeEqual
		&& priorityEqual;
}

Record* TodoConduit::createPCRecord( const HHRecord *hhRec )
{
	FUNCTIONSETUP;

	Akonadi::Item item;
	item.setPayload<IncidencePtr>( IncidencePtr( new KCal::Todo() ) );
	item.setMimeType( "application/x-vnd.akonadi.calendar.todo" );
		
	Record* rec = new TodoAkonadiRecord( item, fMapping.lastSyncedDate() );
	copy( hhRec, rec );
	return rec;
}

HHRecord* TodoConduit::createHHRecord( const Record *pcRec )
{
	HHRecord* hhRec = new TodoHHRecord( PilotTodoEntry().pack(), "Unfiled" );
	copy( pcRec, hhRec );
	return hhRec;
}

void TodoConduit::_copy( const Record *from, HHRecord *to )
{
	const TodoAkonadiRecord* tar = static_cast<const TodoAkonadiRecord*>( from );
	TodoHHRecord* thr = static_cast<TodoHHRecord*>( to );
	
	boost::shared_ptr<KCal::Todo> pcFrom
		 = boost::dynamic_pointer_cast<KCal::Todo, KCal::Incidence>( tar->item().payload<IncidencePtr>() );
	
	PilotTodoEntry hhTo = thr->todoEntry();
	
	// set secrecy, start/end times, alarms, recurrence, exceptions, summary and description:
	if( pcFrom->secrecy() != KCal::Todo::SecrecyPublic )
	{
		hhTo.setSecret( true );
	}

	if( pcFrom->hasDueDate() )
	{
		struct tm t = writeTm( pcFrom->dtDue().dateTime().toLocalTime() );
		hhTo.setDueDate( t );
		hhTo.setIndefinite( 0 );
	}
	else
	{
		hhTo.setIndefinite( 1 );
	}

	// Map priority from PC to HH
	hhTo.setPriority( mapPCPriorityToHH( pcFrom->priority() ) );

	// Set Completed
	hhTo.setComplete( pcFrom->isCompleted() );

	// what we call summary pilot calls description.
	hhTo.setDescription( pcFrom->summary() );

	// what we call description pilot puts as a separate note
	hhTo.setNote( pcFrom->description() );
	
	// NOTE: copyCategory( from, to ); is called before _copy( from, to ). Make
	// sure that the TodoHHRecord::setTodoEntry() keeps the information in the
	// pilot record.

	thr->setTodoEntry( hhTo );
}

void TodoConduit::_copy( const HHRecord *from, Record *to  )
{
	TodoAkonadiRecord* tar = static_cast<TodoAkonadiRecord*>( to );
	const TodoHHRecord* thr = static_cast<const TodoHHRecord*>( from );
	
	boost::shared_ptr<KCal::Todo> pcTo
		 = boost::dynamic_pointer_cast<KCal::Todo, KCal::Incidence>( tar->item().payload<IncidencePtr>() );
		 
	PilotTodoEntry hhFrom = thr->todoEntry();
	
	pcTo->setSecrecy( hhFrom.isSecret() ? KCal::Todo::SecrecyPrivate : KCal::Todo::SecrecyPublic );

	if( hhFrom.getIndefinite() )
	{
		pcTo->setHasDueDate( false );
	}
	else
	{
		pcTo->setDtDue(KDateTime(readTm(hhFrom.getDueDate()), KDateTime::Spec::LocalZone()));
		pcTo->setHasDueDate( true );
	}

	// Map priority from HH to PC
	pcTo->setPriority( mapHHPriorityToPC( hhFrom.getPriority() ) );

	// Properly set the completion state
	if( hhFrom.getComplete() && !pcTo->hasCompletedDate() )
	{
		pcTo->setCompleted( KDateTime::currentLocalDateTime() );
	}
	else if( !hhFrom.getComplete() )
	{
		pcTo->setCompleted( false );
	}

	pcTo->setSummary( hhFrom.getDescription() );
	pcTo->setDescription( hhFrom.getNote() );
	
	// This is not needed as we modified the Todo using a pointer. Uncommenting
	// this give problems as there are now two IncidencePtr objects managing the
	// same raw pointer.
	// Akonadi::Item item( tar->item() );
	// item.setPayload<IncidencePtr>( IncidencePtr( pcTo ) );
	// tar->setItem( item );
}
