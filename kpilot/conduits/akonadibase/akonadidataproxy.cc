/* akonadidataproxy.h			KPilot
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

#include "akonadidataproxy.h"

#include <akonadi/control.h>
#include <akonadi/collection.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <kabc/addressee.h>

#include "idmapping.h"
#include "options.h"

#include "akonadirecord.h"

class AkonadiDataProxyPrivate : public QSharedData
{
public:
	AkonadiDataProxyPrivate( Akonadi::Entity::Id id, const IDMapping& mapping ) 
		: fCollectionId( id ), fMapping( mapping )
	{
	}
	
	Akonadi::Entity::Id fCollectionId;
	// Make it const as the proxy should not make changes to it.
	const IDMapping fMapping;
};

AkonadiDataProxy::AkonadiDataProxy( Akonadi::Entity::Id id, const IDMapping& mapping   )
	: d( new AkonadiDataProxyPrivate( id, mapping ) )
{
	FUNCTIONSETUP;
	
	// Lets make sure that Akonadi is started.
	if ( !Akonadi::Control::start() )
	{
		WARNINGKPILOT << "Error: Could not start Akonadi.";
	}
}

AkonadiDataProxy::~AkonadiDataProxy()
{
	FUNCTIONSETUP;
}

bool AkonadiDataProxy::isOpen() const
{
	FUNCTIONSETUP;
	
	if( Akonadi::Control::start() )
	{
		Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob
			(
				Akonadi::Collection( d->fCollectionId )
				,Akonadi::CollectionFetchJob::Base
			);
		
		return job->exec();
	}
	
	return false;
}

void AkonadiDataProxy::loadAllRecords()
{
	FUNCTIONSETUP;
	
	// Fetch all items with full payload from the root collection
	Akonadi::ItemFetchJob* job
		= new Akonadi::ItemFetchJob( Akonadi::Collection( d->fCollectionId ) );
	job->fetchScope().fetchFullPayload();
	
	if ( job->exec() ) {
		Akonadi::Item::List items = job->items();
		foreach( const Akonadi::Item &item, items )
		{
			if( item.hasPayload<KABC::Addressee>() )
			{
				AkonadiRecord *rec = createAkonadiRecord( item, d->fMapping.lastSyncedDate() );
				fRecords.insert( rec->id(), rec );
			}
		}
		
		// Now add dummy records for deleted records.
		foreach( const QString& mPcId, d->fMapping.pcRecordIds() )
		{
			if( !fRecords.contains( mPcId ) )
			{
				// Well the record with id mPcId doesn't seem to be in the akonadi
				// resource any more so it is deleted.
				AkonadiRecord* ar = createDeletedAkonadiRecord( mPcId );
				Q_ASSERT( ar->isDeleted() );
				Q_ASSERT( ar->isModified() );
				
				fRecords.insert( mPcId, ar );
			}
		}
		
		fCounter.setStartCount( fRecords.size() );
		
		DEBUGKPILOT << "Loaded " << fRecords.size() << " records.";
	}
	else
	{
		DEBUGKPILOT << "Could not load records, is akonadi running?";
	}
}

void AkonadiDataProxy::syncFinished()
{
	FUNCTIONSETUP;
	// No special things have to be done I think. (Bertjan Broeksema).
}

/* Protected methods */

static qint64 newId = -1;

QString AkonadiDataProxy::generateUniqueId()
{
	FUNCTIONSETUP;
	
	newId--;
	return QString::number( newId );
}

void AkonadiDataProxy::commitCreate( Record *rec )
{
	FUNCTIONSETUP;
	
	AkonadiRecord* aRec = static_cast<AkonadiRecord*>( rec );
	Akonadi::ItemCreateJob* job = new Akonadi::ItemCreateJob( aRec->item()
		, Akonadi::Collection( d->fCollectionId ) );

	if ( !job->exec() )
	{
		// Hmm an error occured
		DEBUGKPILOT << "Create failed: " << job->errorString();
	}
	else
	{
		// Update the id of the record.
		QString id = QString::number( job->item().id() );
		rec->setId( id );
	}
}

void AkonadiDataProxy::commitUpdate( Record *rec )
{
	FUNCTIONSETUP;
	
	AkonadiRecord* aRec = static_cast<AkonadiRecord*>( rec );
	Akonadi::ItemModifyJob* job = new Akonadi::ItemModifyJob( aRec->item() );

	if ( !job->exec() )
	{
		// Hmm an error occured
		DEBUGKPILOT << "Update failed: " << job->errorString();
	}
	else
	{
		// Update the id of the record.
		QString id = QString::number( job->item().id() );
		rec->setId( id );
	}
}

void AkonadiDataProxy::commitDelete( Record *rec )
{
	FUNCTIONSETUP;
	
	AkonadiRecord* aRec = static_cast<AkonadiRecord*>( rec );
	Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob( aRec->item() );

	if ( !job->exec() )
	{
		// Hmm an error occured
		DEBUGKPILOT << "Delete failed: " << job->errorString();
	}
}
