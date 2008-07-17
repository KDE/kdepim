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

#include "options.h"

#include "record.h"
#include "hhrecord.h"

#include "akonadicontact.h"
#include "hhcontact.h"
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
	
	// TODO: Make the collection id configurable. For now we just hardcode the
	//       collection id. To find out which collection you can sync, use
	//       akonadiconsole->folder properties->internals.
	
	fHHDataProxy = new ContactsHHDataProxy( fDatabase );
	fBackupDataProxy = new ContactsHHDataProxy( fLocalDatabase );
	fPCDataProxy = new ContactsAkonadiDataProxy( 4 );
	
	return true;
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
	
	return new AkonadiContact( hhRec );
}

HHRecord* Contacts::createHHRecord( const Record *pcRec )
{
	FUNCTIONSETUP;
	
	return new HHContact( pcRec );
}

void Contacts::_copy( const Record *from, HHRecord *to )
{
	FUNCTIONSETUP;
	
	const AkonadiContact* aFrom = static_cast<const AkonadiContact*>( from );
	HHContact* hhTo = static_cast<HHContact*>( to );
	aFrom->copyTo( hhTo );
}

void Contacts::_copy( const HHRecord *from, Record *to  )
{
	FUNCTIONSETUP;
	
	const HHContact* hhFrom = static_cast<const HHContact*>( from );
	AkonadiContact* aTo = static_cast<AkonadiContact*>( to );
	hhFrom->copyTo( aTo );
}
