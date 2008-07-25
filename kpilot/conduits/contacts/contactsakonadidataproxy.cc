/* contactsakonadidataproxy.h			KPilot
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

#include "contactsakonadidataproxy.h"

#include <akonadi/control.h>
#include <akonadi/collection.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <kabc/addressee.h>

#include "options.h"

#include "akonadicontact.h"

ContactsAkonadiDataProxy::ContactsAkonadiDataProxy( Entity::Id id, const QDateTime& dt   )
	: fId( id ), fLastSyncDateTime( dt )
{
	FUNCTIONSETUP;
	
	// Lets make sure that Akonadi is started.
	if ( !Control::start() )
	{
		DEBUGKPILOT << "Error: Could not start Akonadi.";
	}
}

ContactsAkonadiDataProxy::~ContactsAkonadiDataProxy()
{
	FUNCTIONSETUP;
}

void ContactsAkonadiDataProxy::addCategory( Record* rec, const QString& category )
{
	FUNCTIONSETUP;
	
	AkonadiContact* aRec = static_cast<AkonadiContact*>( rec );
	aRec->addCategory( category );
}

bool ContactsAkonadiDataProxy::createDataStore()
{
	FUNCTIONSETUP;
	// TODO: Implement
	
	return false;
}

bool ContactsAkonadiDataProxy::isOpen() const
{
	FUNCTIONSETUP;
	
	return Control::start();
}

void ContactsAkonadiDataProxy::loadAllRecords()
{
	FUNCTIONSETUP;
	
	// Fetch all items with full payload from the root collection
	ItemFetchJob* job = new ItemFetchJob( Collection( fId ) );
	job->fetchScope().fetchFullPayload();
	
	if ( job->exec() ) {
		Item::List items = job->items();
		foreach( const Item &item, items )
		{
			if( item.hasPayload<KABC::Addressee>() )
			{
				AkonadiContact* ac = new AkonadiContact( item, fLastSyncDateTime );
				fRecords.insert( ac->id(), ac );
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

void ContactsAkonadiDataProxy::setCategory( Record* rec, const QString& category )
{
	FUNCTIONSETUP;
	
	AkonadiContact* aRec = static_cast<AkonadiContact*>( rec );
	aRec->setCategory( category );
}

void ContactsAkonadiDataProxy::syncFinished()
{
	FUNCTIONSETUP;
	// No special things have to be done I think. (Bertjan Broeksema).
}

/* Protected methods */

static qint64 newId = -1;

QString ContactsAkonadiDataProxy::generateUniqueId()
{
	FUNCTIONSETUP;
	
	newId--;
	return QString::number( newId );
}

void ContactsAkonadiDataProxy::commitCreate( Record *rec )
{
	FUNCTIONSETUP;
	
	AkonadiContact* aRec = static_cast<AkonadiContact*>( rec );
	ItemCreateJob* job = new ItemCreateJob( aRec->item(), Collection( fId ) );

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

void ContactsAkonadiDataProxy::commitUpdate( Record *rec )
{
	FUNCTIONSETUP;
	
	AkonadiContact* aRec = static_cast<AkonadiContact*>( rec );
	ItemModifyJob* job = new ItemModifyJob( aRec->item() );

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

void ContactsAkonadiDataProxy::commitDelete( Record *rec )
{
	FUNCTIONSETUP;
	
	AkonadiContact* aRec = static_cast<AkonadiContact*>( rec );
	ItemDeleteJob *job = new ItemDeleteJob( aRec->item() );

	if ( !job->exec() )
	{
		// Hmm an error occured
		DEBUGKPILOT << "Delete failed: " << job->errorString();
	}
}
