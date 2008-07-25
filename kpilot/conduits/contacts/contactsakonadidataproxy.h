#ifndef CONTACTSAKONADIDATAPROXY_H
#define CONTACTSAKONADIDATAPROXY_H
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

#include "dataproxy.h"

#include <QtCore/QDateTime>

#include <akonadi/entity.h>

class AkonadiContact;

using namespace Akonadi;

class KPILOT_EXPORT ContactsAkonadiDataProxy : public DataProxy
{
private:
	Entity::Id fId;
	QDateTime fLastSyncDateTime;

public:
	ContactsAkonadiDataProxy( Entity::Id id, const QDateTime& lastSynced  );
	
	/* virtual */ ~ContactsAkonadiDataProxy();

	/**
	 * Adds the given category to the record and might do some internal things
	 * needed for category handling in the datastore.
	 * 
	 * All other categories that might have been set to this record should be
	 * unchanged.
	 */
	/* virtual */ void addCategory( Record* rec, const QString& category );

	/**
	 * Tries to create a new Datastore and returns whether or not it succeeded.
	 */
	/* virtual */ bool createDataStore();

	/**
	 * Returns true when the proxy was able to open the underlying data store 
	 * in read/write mode.
	 */
	/* virtual */ bool isOpen() const;

	/**
	 * Loads all records from underlying data source, sets the startcount of the
	 * counter and resets the iterator.
	 */
	/* virtual */ void loadAllRecords();

	/**
	 * Sets the given category as the only category to the record and might do
	 * some internal things needed for category handling in the datastore.
	 * 
	 * All other categories that might have been set to this record should be
	 * removed.
	 */
	/* virtual */ void setCategory( Record* rec, const QString& category );

	/**
	 * Notifies the proxy that the synchronization is finished and that
	 * no modifications will be done after this.
	 */
	/* virtual */ void syncFinished();

protected: // Functions
	/**
	 * Generates a unique id for a new record.
	 */
	/* virtual */ QString generateUniqueId();
	
	/**
	 * Commits created record @p rec to the datastore. Sets the id that the
	 * data store created for this record to rec.
	 */
	/* virtual */ void commitCreate( Record *rec );
	
	/**
	 * Commits updated record @p rec to the datastore. Must return rec->id() even
	 * if it's unchanged.
	 */
	/* virtual */ void commitUpdate( Record *rec );
	
	/**
	 * Delete record @p rec from the datastore.
	 */
	/* virtual */ void commitDelete( Record *rec );
};

#endif
