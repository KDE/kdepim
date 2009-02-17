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

#include <akonadi/collection.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/servermanager.h>

#include "idmapping.h"
#include "options.h"

#include "akonadirecord.h"

class AkonadiDataProxy::Private
{
public:
	Private( const IDMapping& mapping )
		: fCollectionId( -1 ), fMapping( mapping ), fNextTempId( -1 )
	{
	}

	Akonadi::Entity::Id fCollectionId;
	// Make it const as the proxy should not make changes to it.
	const IDMapping fMapping;
	qint64 fNextTempId;
};

AkonadiDataProxy::AkonadiDataProxy( const IDMapping& mapping )
	: d( new Private( mapping ) )
{
	FUNCTIONSETUP;
}

AkonadiDataProxy::~AkonadiDataProxy()
{
	FUNCTIONSETUP;

	delete d;
}

bool AkonadiDataProxy::createDataStore()
{
	FUNCTIONSETUP;
	// TODO: We don't support creation of akonadi datastores yet. The user should
	// use akonadiconsole for that.
	DEBUGKPILOT << "We don't support creation of akonadi datastores yet. Not doing anything.";
	// TODO: figure out how to tell our user via their Palm sync log that
	// they need to configure akonadi.

	return false;
}

bool AkonadiDataProxy::isOpen() const
{
	FUNCTIONSETUP;

	if( Akonadi::ServerManager::isRunning() )
	{
		Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob
			(
				Akonadi::Collection( d->fCollectionId )
				,Akonadi::CollectionFetchJob::Base
			);
		
		if( !job->exec() )
		{
			WARNINGKPILOT << "Error: Could not fetch collection with id: " << d->fCollectionId;
			return false;
		}
	}
	else
	{
		WARNINGKPILOT << "Error: Akonadi is not running.";
		return false;
	}

	return true;
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
			if( hasValidPayload( item ) )
			{
				AkonadiRecord *rec = createAkonadiRecord( item, d->fMapping.lastSyncedDate() );
				fRecords.insert( rec->id(), rec );
			}
		}
		int loadedFromAkonadi = fRecords.size();
		int dummyDeletedRecords = 0;

		// Now add dummy records for deleted records.
		foreach( const QString& mPcId, d->fMapping.pcRecordIds() )
		{
			if( !fRecords.contains( mPcId ) )
			{
				// Well the record with id mPcId doesn't seem to be in the akonadi
				// resource any more so it is deleted.
				AkonadiRecord* ar = createDeletedAkonadiRecord( mPcId );
				ar->setDummy();
				Q_ASSERT( ar->isDeleted() );
				Q_ASSERT( ar->isModified() );
				Q_ASSERT( ar->id() == mPcId );

				fRecords.insert( mPcId, ar );
				++dummyDeletedRecords;
			}
		}
		fCounter.setStartCount( fRecords.size() );

		DEBUGKPILOT << "Loaded: " << loadedFromAkonadi
			    << " records from Akonadi, created: " << dummyDeletedRecords
			    << " dummy deleted records. Total starting record count: "
			    << fRecords.size();
	}
	else
	{
		DEBUGKPILOT << "Could not load records, is akonadi running?";
	}
}

void AkonadiDataProxy::setCollectionId( const Akonadi::Collection::Id id )
{
	d->fCollectionId = id;
}

void AkonadiDataProxy::syncFinished()
{
	FUNCTIONSETUP;
	// No special things have to be done I think. (Bertjan Broeksema).
}

/* Protected methods */

QString AkonadiDataProxy::generateUniqueId()
{
	FUNCTIONSETUP;

	return QString::number( d->fNextTempId-- );
}

bool AkonadiDataProxy::commitCreate( Record *rec )
{
	FUNCTIONSETUP;

	AkonadiRecord* aRec = static_cast<AkonadiRecord*>( rec );
	Akonadi::ItemCreateJob* job = new Akonadi::ItemCreateJob( aRec->item()
		, Akonadi::Collection( d->fCollectionId ) );

	if ( !job->exec() )
	{
		// Hmm an error occurred
		DEBUGKPILOT << "Create failed: " << job->errorString();
		return false;
	}
	else
	{
		// Update the item of the record.
		aRec->setItem( job->item() );
		return true;
	}
}

bool AkonadiDataProxy::commitUpdate( Record *rec )
{
	FUNCTIONSETUP;

	AkonadiRecord* aRec = static_cast<AkonadiRecord*>( rec );
	Akonadi::ItemModifyJob* job = new Akonadi::ItemModifyJob( aRec->item() );

	if ( !job->exec() )
	{
		// Hmm an error occurred
		DEBUGKPILOT << "Update failed: " << job->errorString();
		return false;
	}
	else
	{
		// Update the item of the record.
		aRec->setItem( job->item() );
		return true;
	}
}

bool AkonadiDataProxy::commitDelete( Record *rec )
{
	FUNCTIONSETUP;

	AkonadiRecord* aRec = static_cast<AkonadiRecord*>( rec );
	Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob( aRec->item() );

	if ( !job->exec() )
	{
		/**
		 * An error occurred, but it could be that it's because we're trying
		 * to delete something that doesn't exist in Akonadi (our dummy,
		 * used-only-for-deletion records). Check for the validity of our
		 * object and if it's valid, then fail, but otherwise, ignore the
		 * failure.
		 */
		DEBUGKPILOT << "Delete failed. error: " << job->error()
			    << ", message: " << job->errorString();
		// TODO: Akonadi needs to get enhanced to return useful return codes
		// that we can check. In KDE 4.2, it's not there yet, so we just use
		// this. But in the future, we should look for Akonadi error codes too.
		if ( aRec->isValid() )
		{
			return false;
		}
	}

	return true;
}
