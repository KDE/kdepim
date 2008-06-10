#ifndef HHDATAPROXY_H
#define HHDATAPROXY_H
/* hhdataproxy.h			KPilot
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

#include "pilotAppInfo.h"
#include "dataproxy.h"

#include <pi-macros.h> // For recordid_t

class PilotDatabase;
class PilotAppInfoBase;
class PilotRecord;
class HHRecord;

struct CategoryAppInfo;

class KPILOT_EXPORT HHDataProxy : public DataProxy {
public:
	HHDataProxy( PilotDatabase *db );
	
	/**
	 * Returns whether or not the pilot database is opened.
	 */
	virtual bool isOpen() const;
	
	/**
	 * Notifies the proxy that the synchronization is finished and that
	 * no modifications will be done after this.
	 */
	virtual void syncFinished();
	
	/**
	 * Sets the category of @param rec to "Unfiled" or whatever is appropriate for
	 * the conduit to say that the record has no category.
	 */
	void clearCategory( HHRecord *rec );
	
	/**
	 * Returns true if the the PilotDatabase contains the given category, false
	 * otherwhise.
	 */
	bool containsCategory( const QString& category ) const;
	
	/**
	 * Adds the category to the pilot database. If the category app info block is
	 * full and doesn't contain the category it returns false. True otherwhise.
	 */
	bool addGlobalCategory( const QString& category );
	
protected:
	/**
	 * Reads all records from the database.
	 */
	void loadAllRecords();
	
	/** These functions must be implemented by the subclassing conduit **/

	/**
	 * Implementing classes read the appinfo block and return a pointer so that
	 * category information can be read and altered.
	 */
	virtual PilotAppInfoBase* readAppInfo() = 0;
	
	/**
	 * Implementing classes should pack and store fAppInfo into the database so
	 * that Category information is stored.
	 */
	virtual void storeAppInfo() = 0;

	/**
	 * This function creates a (subclass of) HHRecord for @p rec.
	 */
	virtual HHRecord* createHHRecord( PilotRecord *rec ) = 0;
	
	/**
	 * Generates a unique id for a new record.
	 */
	 virtual QString generateUniqueId();
	
	/**
	 * Commits created record @p rec to the datastore.
	 */
	virtual void commitCreate( Record *rec );
	
	/**
	 * Commits updated record @p rec to the datastore.
	 */
	virtual void commitUpdate( Record *rec );
	
	/**
	 * Undo the commit of created record @p rec to the datastore.
	 */
	virtual void commitDelete( Record *rec );

protected:
	PilotDatabase *fDatabase;
	recordid_t fLastUsedUniqueId;
	QList<recordid_t> fResettedRecords;
	PilotAppInfoBase *fAppInfo;
};
#endif
