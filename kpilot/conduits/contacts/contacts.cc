/* contacts.cc			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2008 by Jason "vanRijn" Kasper <vr@movingparts.net>
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

#include "contacts.h"

#include <akonadi/control.h>
#include <akonadi/collection.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>

#include "options.h"

#include "record.h"
#include "hhrecord.h"

#include "contactshhdataproxy.h"
#include "contactsakonadidataproxy.h"

using namespace Akonadi;

Contacts::Contacts( KPilotLink *o, const QVariantList &a )
 : RecordConduit( o, a, CSL1( "AddressDB" ), CSL1( "Contacts Conduit" ) )
{
}

void Contacts::loadSettings()
{
	FUNCTIONSETUP;
}
	
bool Contacts::initDataProxies()
{
	FUNCTIONSETUP;
	
	if( !fDatabase )
	{
		addSyncLogEntry( i18n( "Error: Handheld database is not loaded." ) );
		return false;
	}
	
	// Lets make sure that Akonadi is started.
	if ( !Control::start() )
	{
		addSyncLogEntry( i18n( "Error: Could not start Akonadi." ) );
		return false;
	}
	
	fHHDataProxy = new ContactsHHDataProxy( fDatabase );
	fBackupDataProxy = new ContactsHHDataProxy( fLocalDatabase );
	
	// TODO: Make the collection id configurable. For now we just hardcode the
	//       collection id. To find out which collection you can sync, use
	//       akonadiconsole->folder properties->internals.
	// Fetch all items with full payload from the root collection
	ItemFetchJob *job = new ItemFetchJob( Collection( 4 ) );
	job->fetchScope().fetchFullPayload();
	
	if ( job->exec() ) {
		fPCDataProxy = new ContactsAkonadiDataProxy( /* job->items() */ );
		return true;
	}
	
	return false;
}

bool Contacts::equal( const Record *pcRec, const HHRecord *hhRec ) const
{
	FUNCTIONSETUP;
	
	if( !pcRec || !hhRec )
	{
		return false;
	}
	
	return pcRec->equal( hhRec );
}

Record* Contacts::createPCRecord( const HHRecord *hhRec )
{
	FUNCTIONSETUP;
	//TODO: IMPLEMENT
	return 0L;
}

HHRecord* Contacts::createHHRecord( const Record *pcRec )
{
	FUNCTIONSETUP;
	//TODO: IMPLEMENT
	return 0L;
}

void Contacts::_copy( const Record *from, HHRecord *to )
{
	FUNCTIONSETUP;
	
	//TODO: IMPLEMENT
}

void Contacts::_copy( const HHRecord *from, Record *to  )
{
	FUNCTIONSETUP;
	
	//TODO: IMPLEMENT
}
