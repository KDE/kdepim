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
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>

#include "options.h"

ContactsAkonadiDataProxy::ContactsAkonadiDataProxy( Entity::Id id ) : fId( id )
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
	// TODO: Implement
}

void ContactsAkonadiDataProxy::addCategory( Record* rec, const QString& category )
{
	FUNCTIONSETUP;
	// TODO: Implement
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
	ItemFetchJob *job = new ItemFetchJob( Collection( 4 ) );
	job->fetchScope().fetchFullPayload();
	
	if ( job->exec() ) {
		// TODO: Implement
	}
	else
	{
		DEBUGKPILOT << "Could not load records, is akonadi running?";
	}
}

void ContactsAkonadiDataProxy::setCategory( Record* rec, const QString& category )
{
	FUNCTIONSETUP;
	// TODO: Implement
}

void ContactsAkonadiDataProxy::syncFinished()
{
	FUNCTIONSETUP;
	// TODO: Implement
}

/* Protected methods */

QString ContactsAkonadiDataProxy::generateUniqueId()
{
	FUNCTIONSETUP;
	// TODO: Implement
	
	return QString();
}

void ContactsAkonadiDataProxy::commitCreate( Record *rec )
{
	FUNCTIONSETUP;
	// TODO: Implement
}

void ContactsAkonadiDataProxy::commitUpdate( Record *rec )
{
	FUNCTIONSETUP;
	// TODO: Implement
}

void ContactsAkonadiDataProxy::commitDelete( Record *rec )
{
	FUNCTIONSETUP;
	// TODO: Implement
}
