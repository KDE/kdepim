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
	 * Given a list of category names from the PC side @p pcCategories, look for
	 * the category @em best matching the category @p category in the appinfo
	 *  block. Here, best is defined as follows:
	 * - if the name of category @p category is in the list, use it
	 * - otherwise use the first category from the list that is a valid
	 *   category on the handheld.
	 * - use Unfiled if none match.
	 *
	 * @return Category that best matches.
	 * @return Unfiled if no best match.
	 */
	QString bestMatchCategory( const QStringList& pcCategories
	                         , const QString& category ) const;
	
	/**
	 * Sets the category of @param rec to "Unfiled" or whatever is appropriate for
	 * the conduit to say that the record has no category.
	 */
	void clearCategory( HHRecord *rec );
	
	/**
	 * Returns true if the PilotDatabase contains the given category, false
	 * otherwise.
	 */
	bool containsCategory( const QString& category ) const;
	
	/**
	 * Reads all records from the database.
	 */
	void loadAllRecords();
	
	/**
	 * Adds the category to the pilot database. If the category app info block is
	 * full and doesn't contain the category it returns false. True otherwise.
	 */
	bool addGlobalCategory( const QString& category );
	
	/**
	 * Reads the right category id from app info and updates the record
	 * accordingly. When the Category is not found, it tries to add the Category
	 * to the database. If there is no category id found and the Category could
	 * not be added to the database the category of the record remains unchanged.
	 */
	/* virtual */ void setCategory( Record* rec, const QString& category );

	/**
	 * Does nothing because handheld records only can have only one category.
	 */
	/* virtual */ void addCategory( Record*, const QString& ) {};
	
protected:
	/** These functions must be implemented by the subclassing conduit **/

	/**
	 * Implementing classes read the appinfo block and return a pointer so that
	 * category information can be read and altered.
	 */
	virtual PilotAppInfoBase* readAppInfo() = 0;

	/**
	 * This function creates a (subclass of) HHRecord for @p rec.
	 */
	virtual HHRecord* createHHRecord( PilotRecord *rec ) = 0;
	
	/**
	 * Generates a unique id for a new record.
	 */
	virtual QString generateUniqueId();
	
	/**
	 * Commits the changes of global added categories to the database.
	 */
	virtual bool _commit();
	
	/**
	 * Undo the changes of global added categories to the database.
	 */
	virtual bool _rollback();
	
	/**
	 * Commits created record @p rec to the datastore.
	 */
	virtual bool commitCreate( Record *rec );
	
	/**
	 * Commits updated record @p rec to the datastore.
	 */
	virtual bool commitUpdate( Record *rec );
	
	/**
	 * Undo the commit of created record @p rec to the datastore.
	 */
	virtual bool commitDelete( Record *rec );

protected:
	PilotDatabase *fDatabase;
	recordid_t fLastUsedUniqueId;
	QList<recordid_t> fResettedRecords;
	PilotAppInfoBase *fAppInfo;
	QMap<uint, QString> fAddedCategories;
	qint64 fNextTempId;
};
#endif
