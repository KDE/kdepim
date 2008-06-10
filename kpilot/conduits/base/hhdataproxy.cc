/* hhdataproxy.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2007 by Jason "vanRijn" Kasper <vr@movingparts.net>
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

#include "hhdataproxy.h"

#include <klocalizedstring.h>

#include "pilotDatabase.h"
#include "pilotAppInfo.h"
#include "pilotRecord.h"
#include "options.h"
#include "pilot.h"

#include "hhrecord.h"

HHDataProxy::HHDataProxy( PilotDatabase *db ) : fDatabase( db )
	, fLastUsedUniqueId( 0L )
{
}

void HHDataProxy::syncFinished()
{
	FUNCTIONSETUP;
	
	if( fDatabase && fDatabase->isOpen() )
	{
		fDatabase->cleanup();
		fDatabase->resetSyncFlags();
		//saveCategories(); Make this storeAppInfo()
	}
}

void HHDataProxy::clearCategory( HHRecord *rec )
{
	FUNCTIONSETUP;
	
	rec->setCategory( Pilot::Unfiled, CSL1( "Unfiled" ) );
}

bool HHDataProxy::containsCategory( const QString& category ) const
{
	FUNCTIONSETUP;
	
	return fAppInfo->findCategory( category, false ) != -1;
}
	
bool HHDataProxy::addGlobalCategory( const QString& category )
{
	FUNCTIONSETUP;
	
	Q_UNUSED( category );
	
	// TODO: IMPLEMENT
	
	return false;
}

QString HHDataProxy::generateUniqueId()
{
	recordid_t id = 0;
	
	QList<QString> ids = fRecords.keys();
	
	for( int i = 0; i < fRecords.size(); i++ )
	{
		if( ids.at( i ).toULong() > id )
		{
			id = ids.at( i ).toULong();
		}
	}
	
	return QString::number( id + 1 );
}

void HHDataProxy::commitCreate( Record *rec )
{
	FUNCTIONSETUP;
	
	if( fDatabase && rec )
	{
		if( HHRecord *hhRec = static_cast<HHRecord*>( rec ) )
		{
			// Set the id to 0 to make sure that the database asigns a valid id to the
			// record.
			hhRec->setId( QString::number( 0 ) );
			fDatabase->writeRecord( hhRec->pilotRecord() );
		}
		else
		{
			DEBUGKPILOT << "Record " << rec->id() << " is not of type HHRecord*.";
		}
	}
}

void HHDataProxy::commitUpdate( Record *rec )
{
	FUNCTIONSETUP;

	if( fDatabase && rec )
	{
		if( HHRecord *hhRec = static_cast<HHRecord*>( rec ) )
		{
			fDatabase->writeRecord( hhRec->pilotRecord() );
		}
		else
		{
			DEBUGKPILOT << "Record " << rec->id() << " is not of type HHRecord*.";
		}
	}
}

void HHDataProxy::commitDelete( Record *rec )
{
	FUNCTIONSETUP;
	
	if( !rec || !fDatabase )
	{
		return;
	}
	else
	{
		if( HHRecord *hhRec = static_cast<HHRecord*>( rec ) )
		{
			fDatabase->deleteRecord( hhRec->pilotRecord()->id() );
		}
		else
		{
			DEBUGKPILOT << "Record " << rec->id() << " is not of type HHRecord*.";
		}
	}
}

bool HHDataProxy::isOpen() const
{
	FUNCTIONSETUP;
	
	if( fDatabase )
	{
		return fDatabase->isOpen();
	}
	else
	{
		return false;
	}
}

void HHDataProxy::loadAllRecords()
{
	FUNCTIONSETUP;
	
	if( fDatabase && fDatabase->isOpen() )
	{
		fAppInfo = readAppInfo();
	
		int index = 0;
		
		PilotRecord *pRec = fDatabase->readRecordByIndex( index );
		
		while( pRec )
		{
			// Create a record object.
			HHRecord *rec = createHHRecord( pRec );
			fRecords.insert( rec->id(), rec );
			
			QString cat = fAppInfo->categoryName( pRec->category() );
			
			if( cat.isEmpty() )
			{
				// This is strange, should not happen I think. However if it happens
				// make sure that the record has some reasonable values.
				rec->setCategory( Pilot::Unfiled, cat );
			}
			else
			{
				rec->setCategory( pRec->category(), cat );
			}
				
			// Read the next one.
			pRec = fDatabase->readRecordByIndex( ++index );
		}
		fCounter.setStartCount( fRecords.count() );
		
		DEBUGKPILOT << "Loaded " << fRecords.count() << " records.";
	}
}
