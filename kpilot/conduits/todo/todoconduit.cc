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
	}
	
	Akonadi::Collection::Id fCollectionId;
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

void TodoConduit::loadSettings()
{
	FUNCTIONSETUP;
	
	TodoSettings::self()->readConfig();
	d->fCollectionId = TodoSettings::akonadiCollection();
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
	
	// At this point we should be able to read the backup and handheld database.
	// However, it might be that Akonadi is not started.
	TodoAkonadiProxy* tadp = new TodoAkonadiProxy( fMapping );
	tadp->setCollectionId( d->fCollectionId );
	if( tadp->isOpen() )
	{
		tadp->loadAllRecords();
	}
	 
	fPCDataProxy = tadp;
	fHHDataProxy = new TodoHHDataProxy( fLocalDatabase );
	fHHDataProxy->loadAllRecords();
	fBackupDataProxy = new TodoHHDataProxy( fLocalDatabase );
	fPCDataProxy->loadAllRecords();
	
	return true;
}

bool TodoConduit::equal( const Record *pcRec, const HHRecord *hhRec ) const
{
	FUNCTIONSETUP;
	
	const TodoAkonadiRecord* tar = static_cast<const TodoAkonadiRecord*>( pcRec );
	const TodoHHRecord* thr = static_cast<const TodoHHRecord*>( hhRec );
	
	KCal::Todo pcTodo = tar->item().payload<KCal::Todo>();
	PilotTodoEntry hhTodo = thr->todoEntry();
	
	bool descriptionEqual = pcTodo.summary() == hhTodo.getDescription();
	bool noteEqual = pcTodo.description() == hhTodo.getNote();
	bool categoriesEqual = pcTodo.categories().contains( thr->category() );
	bool dueDateEqual = pcTodo.dtDue().dateTime() == readTm( hhTodo.getDueDate() );
	bool completeEqual = pcTodo.isCompleted() == (hhTodo.getComplete() != 0 );
	
	// TODO: Do some mapping for the priority.
	// TODO: Do some mapping for the completed percentage.
	
	return descriptionEqual
		&& noteEqual
		&& categoriesEqual
		&& dueDateEqual
		&& completeEqual;
}

Record* TodoConduit::createPCRecord( const HHRecord *hhRec )
{
	FUNCTIONSETUP;

	// FIXME: We should use IncidencePtr here. This code doesn't work.
	Akonadi::Item item;
	item.setPayload<KCal::Todo>( KCal::Todo() );
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
	
	KCal::Todo pcFrom = tar->item().payload<KCal::Todo>();
	PilotTodoEntry hhTo = thr->todoEntry();
	
	// set secrecy, start/end times, alarms, recurrence, exceptions, summary and description:
	if( pcFrom.secrecy() != KCal::Todo::SecrecyPublic )
	{
		hhTo.setSecret( true );
	}

	if( pcFrom.hasDueDate() )
	{
		struct tm t = writeTm( pcFrom.dtDue().dateTime() );
		hhTo.setDueDate( t );
		hhTo.setIndefinite( 0 );
	}
	else
	{
		hhTo.setIndefinite( 1 );
	}

	// TODO: Map priority of KCal::Todo to PilotTodoEntry
	// hhTo.setPriority( todo->priority() );

	hhTo.setComplete( pcFrom.isCompleted() );

	// what we call summary pilot calls description.
	hhTo.setDescription( pcFrom.summary() );

	// what we call description pilot puts as a separate note
	hhTo.setNote( pcFrom.description() );
	
	// NOTE: copyCategory( from, to ); is called before _copy( from, to ). Make
	// sure that the TodoHHRecord::setTodoEntry() keeps the information in the
	// pilot record.

	thr->setTodoEntry( hhTo );
}

void TodoConduit::_copy( const HHRecord *from, Record *to  )
{
	TodoAkonadiRecord* tar = static_cast<TodoAkonadiRecord*>( to );
	const TodoHHRecord* thr = static_cast<const TodoHHRecord*>( from );
	
	KCal::Todo pcTo = tar->item().payload<KCal::Todo>();
	PilotTodoEntry hhFrom = thr->todoEntry();
	
	pcTo.setSecrecy( hhFrom.isSecret() ? KCal::Todo::SecrecyPrivate : KCal::Todo::SecrecyPublic );

	if ( hhFrom.getIndefinite() )
	{
		pcTo.setHasDueDate( false );
	}
	else
	{
		pcTo.setDtDue(KDateTime(readTm(hhFrom.getDueDate()), KDateTime::Spec::LocalZone()));
		pcTo.setHasDueDate( true );
	}

	// PRIORITY //
	// TODO: e->setPriority(de->getPriority());

	pcTo.setCompleted( KDateTime() );
	if ( hhFrom.getComplete() && !pcTo.hasCompletedDate() )
	{
		pcTo.setCompleted( KDateTime::currentLocalDateTime() );
	}

	pcTo.setSummary( hhFrom.getDescription() );
	pcTo.setDescription( hhFrom.getNote() );
	
	Akonadi::Item item( tar->item() );
	item.setPayload<KCal::Todo>( pcTo );
	
	tar->setItem( item );
}
