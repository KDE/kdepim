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


TodoConduit::TodoData::TodoData( RecordConduit *conduit, VCalConduitSettings *cfg ) : VCalConduitBase::VCalDataBase( conduit, cfg ) 
{}
TodoConduit::TodoData::~TodoData()
{}
bool TodoConduit::TodoData::initData()
{
	if ( !mCalendar ) return false;
	KCal::Todo::List lst = mCalendar->todos();
	KCal::Todo::List::Iterator it = lst.begin();
	for ( it = lst.begin(); it != lst.end(); ++it ) {
		mIncidences.append( *it );
	}
	return true;
}
// KCal::Incidence *TodoConduitPrivate::findIncidence(PilotAppCategory*tosearch)
// {
// 	PilotTodoEntry*entry=dynamic_cast<PilotTodoEntry*>(tosearch);
// 	if (!entry) return 0L;
// 
// 	QString title=entry->getDescription();
// 	QDateTime dt=readTm( entry->getDueDate() );
// 
// 	KCal::Todo::List::ConstIterator it;
//         for( it = fAllTodos.begin(); it != fAllTodos.end(); ++it ) {
//                 KCal::Todo *event = *it;
// 		if ( (event->dtDue().date() == dt.date()) && (event->summary() == title) ) return event;
// 	}
// 	return 0L;
// }



/****************************************************************************
 *                          TodoConduit class                               *
 ****************************************************************************/

TodoConduit::TodoConduit( KPilotDeviceLink *d,
	const char *n, const QStringList &a) : 
	VCalConduitBase( i18n("To-do"), d, n, a )
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << id_conduit_todo << endl;
#endif
	(void) id_conduit_todo;
}



TodoConduit::~TodoConduit()
{
//	FUNCTIONSETUP;
}


/** Create a buffer for the appInfo and pack it inside. Also, remember to
 *  set appLen to the size of the buffer! 
 *  @param appLen Receives the length of the allocated buffer */
unsigned char *TodoConduit::doPackAppInfo( int *appLen )
{
	int appLength = pack_ToDoAppInfo(&fTodoAppInfo, 0, 0);
	unsigned char *buffer = new unsigned char[appLength];
	pack_ToDoAppInfo(&fTodoAppInfo, buffer, appLength);
	if ( appLen ) *appLen = appLength;
	return buffer;
}


/** Read the appInfo from the buffer provided to this method. 
 *  @param buffer Buffer containing the appInfo in packed format
 *  @param appLen specifies the length of buffer */
bool TodoConduit::doUnpackAppInfo( unsigned char *buffer, int appLen )
{
	FUNCTIONSETUP;
	int len = unpack_ToDoAppInfo( &fTodoAppInfo, buffer, appLen );

#ifdef DEBUG
	DEBUGCONDUIT << fname << " lastUniqueId="
		 << fTodoAppInfo.category.lastUniqueID << endl;
	for (int i = 0; i < 16; i++)
	{
		DEBUGCONDUIT << fname << " cat " << i << " =" << 
			fTodoAppInfo.category.name[i] << endl;
	}
#endif
	return ( len > 0 );
}

VCalConduitSettings *TodoConduit::config()
{ 
  return ToDoConduitFactory::config(); 
}

const QString TodoConduit::getTitle( PilotAppCategory *de )
{
	PilotTodoEntry *d = dynamic_cast<PilotTodoEntry*>(de);
	if ( d ) return QString( d->getDescription() );
	return QString::null;
}

void TodoConduit::readConfig()
{
	FUNCTIONSETUP;
	VCalConduitBase::readConfig();
	// determine if the categories have ever been synce. Needed to prevent loosing the categories on the desktop.
	// also use a full sync for the first time to make sure the palm categories are really transferred to the desktop
	categoriesSynced = config()->conduitVersion()>=CONDUIT_VERSION_CATEGORYSYNC;
#ifdef DEBUG
	DEBUGCONDUIT << "categoriesSynced=" << categoriesSynced << endl;
#endif
}

void TodoConduit::doPostSync()
{
	FUNCTIONSETUP;
	// after this successful sync the categories have been synced for sure
	config()->setConduitVersion( CONDUIT_VERSION );
	config()->writeConfig();
}

/* @TODO: Set categories if cats haven't been synced yet!
void TodoConduit::preRecord(PilotRecord*r)
{
	FUNCTIONSETUP;
	if ( !categoriesSynced && r )
	{
		const PilotAppCategory*de = createPalmEntry( r );
		KCal::Incidence *e = fP->findIncidence( r->getID() );
		setCategory( dynamic_cast<KCal::Todo*>(e), dynamic_cast<const PilotTodoEntry*>(de) );
	}
}
*/

RecordConduit::PCData *TodoConduit::initializePCData()
{
	return new TodoData( this, config() );
}

QString TodoConduit::category( int n ) const
{
	return PilotAppCategory::codec()->
		toUnicode( fTodoAppInfo.category.name[n] );
}


bool TodoConduit::_equal( const PilotAppCategory *cmpPalmEntry, const PCEntry *cmpPCEntry, 
				int flags ) const
{
	FUNCTIONSETUP;
	const PilotTodoEntry *palmEntry = dynamic_cast<const PilotTodoEntry*>( cmpPalmEntry );
	const VCalEntry *pcEntryTmp = dynamic_cast<const VCalEntry*>( cmpPCEntry );
	const KCal::Todo *pcEntry = 0;
	if ( !pcEntryTmp ) 
		pcEntry = dynamic_cast<const KCal::Todo*>( pcEntryTmp->incidence() );

	if ( !palmEntry || !pcEntryTmp || !pcEntry ) {
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": NULL todo given... Skipping it" << endl;
#endif
		if ( !palmEntry && pcEntry && ( pcEntry->syncStatus()==KCal::Incidence::SYNCDEL ) )
			return true;
		if ( !palmEntry && !pcEntry ) 
			return true;
		return false;
	}

	if ( flags & eqFlagsFlags ) {
		if ( isArchived( palmEntry ) && isArchived( cmpPCEntry ) ) return true;
		if ( isArchived( palmEntry ) || isArchived( cmpPCEntry ) ) return false;
	}

	if ( flags & eqFlagsDesc )
	{
		if ( compareStr( palmEntry->getDescription(), pcEntry->summary() ) )
			return false;
		if ( compareStr( palmEntry->getNote(), pcEntry->description() ) )
			return false;
	}
	
	if ( flags & eqFlagsPriority )
	{
		if ( palmEntry->getPriority() != pcEntry->priority() )
			return false;
	}
	
	if ( flags & eqFlagsCompleted )
	{
		if ( palmEntry->getComplete() != pcEntry->isCompleted() )
			return false;
	}
	if ( flags & eqFlagsSecrecy )
	{
		if ( pcEntry->secrecy() == KCal::Todo::SecrecyPublic ) {
			if ( palmEntry->isSecret() ) return false;
		} else {
			if ( !palmEntry->isSecret() ) return false;
		}
	}
	if ( flags & eqFlagsDue )
	{
		if ( pcEntry->hasDueDate()) {
			QDateTime palmDue( readTm( palmEntry->getDueDate() ) );
			if ( palmDue != pcEntry->dtDue() )
				return false;
		} else {
			if ( !palmEntry->getIndefinite() ) 
				return false;
		}
	}
	if ( flags & eqFlagsCategory )
	{
// @TODO:		Compare categories
	}	
	return true;
}


bool TodoConduit::_copy( PilotAppCategory *toPalmEntry, const PCEntry *fromPCEntry )
{
	FUNCTIONSETUP;
	PilotTodoEntry *palmEntry = dynamic_cast<PilotTodoEntry*>( toPalmEntry );
	const VCalEntry *pcEntryTmp = dynamic_cast<const VCalEntry*>( fromPCEntry );
	const KCal::Todo *pcEntry = 0;
	if ( !pcEntryTmp ) 
		pcEntry = dynamic_cast<const KCal::Todo*>( pcEntryTmp->incidence() );
	if ( !palmEntry || !pcEntryTmp || !pcEntry ) {
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": NULL todo given... Skipping it" << endl;
#endif
		return false;
	}

	// set secrecy, start/end times, alarms, recurrence, exceptions, summary and description:
	if ( pcEntry->secrecy() != KCal::Todo::SecrecyPublic ) 
		palmEntry->makeSecret();

	// update it from the iCalendar Todo.

	if ( pcEntry->hasDueDate()) {
		struct tm t = writeTm( pcEntry->dtDue() );
		palmEntry->setDueDate( t );
		palmEntry->setIndefinite( 0 );
	} else {
		palmEntry->setIndefinite( 1 );
	}

	// TODO: take recurrence (code in VCAlConduit) from ActionNames

	setCategory( palmEntry, pcEntry );

	// TODO: sync the alarm from ActionNames. Need to extend PilotTodoEntry
	palmEntry->setPriority( pcEntry->priority() );

	palmEntry->setComplete( pcEntry->isCompleted() );

	// what we call summary pilot calls description.
	palmEntry->setDescription( pcEntry->summary() );

	// what we call description pilot puts as a separate note
	palmEntry->setNote( pcEntry->description() );

#ifdef DEBUG
DEBUGCONDUIT << "-------- " << pcEntry->summary() << endl;
#endif
	return true;
}


bool TodoConduit::_copy( PCEntry *toPCEntry, const PilotAppCategory *fromPalmEntry )
{
	FUNCTIONSETUP;
	const PilotTodoEntry *palmEntry = dynamic_cast<const PilotTodoEntry*>( fromPalmEntry );
	VCalEntry *pcEntryTmp = dynamic_cast<VCalEntry*>( toPCEntry );
	KCal::Todo *pcEntry = 0;
	if ( !pcEntryTmp ) 
		pcEntry = dynamic_cast<KCal::Todo*>( pcEntryTmp->incidence() );
	if ( !palmEntry || !pcEntryTmp || !pcEntry ) {
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": NULL todo given... Skipping it" << endl;
#endif
		return false;
	}

	VCalDataBase *db = dynamic_cast<VCalDataBase *>(mPCData);
	// @TODO: What about the common name???
	if ( pcEntry->organizer().isEmpty() && db && db->calendar() ) 
		pcEntry->setOrganizer( db->calendar()->getEmail());
	pcEntry->setPilotId( palmEntry->getID() );
	pcEntry->setSecrecy( palmEntry->isSecret() ? KCal::Todo::SecrecyPrivate : KCal::Todo::SecrecyPublic );

	if ( palmEntry->getIndefinite() ) {
		pcEntry->setHasDueDate( false );
	} else {
		pcEntry->setDtDue( readTm( palmEntry->getDueDate() ) );
		pcEntry->setHasDueDate( true );
	}

	// Categories
	// TODO: Sync categories
	// first remove all categories and then add only the appropriate one
	setCategory( pcEntry, palmEntry );

	// PRIORITY //
	pcEntry->setPriority( palmEntry->getPriority() );

	// COMPLETED? //
	pcEntry->setCompleted( palmEntry->getComplete() );
	if ( palmEntry->getComplete() && !pcEntry->hasCompletedDate() ) {
		pcEntry->setCompleted( QDateTime::currentDateTime() );
	}

	pcEntry->setSummary( palmEntry->getDescription() );
	pcEntry->setDescription( palmEntry->getNote() );

	// copy archived flag over!
	if ( palmEntry->getAttrib() & dlpRecAttrArchived ) 
		pcEntry->setSyncStatus( KCal::Incidence::SYNCDEL );
	else
		pcEntry->setSyncStatus( KCal::Incidence::SYNCNONE );

	return true;
}

