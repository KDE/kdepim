#ifndef DATAPROXY_H
#define DATAPROXY_H
/* dataproxy.h			KPilot
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

#include <QtCore/QMap>
#include <QtCore/QMultiMap>

#include "cudcounter.h"
#include "kpilot_export.h"

class Record;

class KPILOT_EXPORT DataProxy {
public:
	enum Mode {
		All=1,
		Modified=2
	};
	
	DataProxy();
	
	virtual ~DataProxy();

	/**
		* Adds the record to the database and returns the internal id for the added
		* record. This method resets the iterator.
		*/
	QString create( Record *record );

	/**
	 * Deletes record with @p id from the database. This method resets the 
	 * iterator.
	 */
	void remove( const QString &id );

	/**
	 * Updates the fields of record with @p id with the fields from @p record.
	 * This method resets the iterator.
	 */
	void update( const QString &id, Record *record );

	/**
	 * Returns the list with ids from the records in the proxy.
	 */
	QList<QString> ids() const;

	/**
	 * Returns the CUDCounter. Because the dataproxy is the only one who is 
	 * authorized to change it the pointer is const.
	 */
	const CUDCounter* counter() const;

	/**
	 * Sends the end count for the CUD counter.
	 */
	void setEndcount();

	/**
	 * Notifies the proxy that the synchronization is finished and that
	 * no modifications will be done after this.
	 */
	virtual void syncFinished() {}

	/**
	 * Sets the mode which is used to iterate over the loaded records. It defaults
	 * to iterate over all records.
	 */
	void setIterateMode( const Mode m = All );
	
	/**
	 * Returns the record count. Keep in mind that if there are uncommitted
	 * changes this may differ from the record count of the actual datastore.
	 */
	unsigned int recordCount() const;
	
	/**
	 * Looks for a matching record. Should return 0L if there is no match.
	 */
	Record* find( const QString &id ) const;

	/**
	 * Looks for records with given description. Returns an empty list if no
	 * records are found with such a description.
	 */
	QList<Record*> findByDescription( const QString &description ) const;
	
	/**
	 * Resets the iterator.
	 */
	void resetIterator();
	
	/**
	 * Depending on the iterateMode it should give if there is a next record or if
	 * there is a next modified/deleted record.
	 */
	bool hasNext() const;

	/**
	 * Depending on the iterateMode it should give the next record, the next
	 * modified/deleted record or 0L if there are no more records to iterate over.
	 */
	Record* next();
	
	/**
	 * Commits all changes to the data store.
	 */
	bool commit();
	
	/**
	 * Reverts all changes that are committed to the data store.
	 */
	bool rollback();
	
	/**
	 * After a commit/rollback this returns a list of pairs with ids that have
	 * changed. The keys are the old ids and the values are the new ids for the
	 * records. Note: before a commit or a rollback the list is cleared.
	 */
	QMap<QString, QString> changedIds();
	
	/**
	 * Sets the given category as the only category to the record and might do
	 * some internal things needed for category handling in the datastore.
	 * 
	 * All other categories that might have been set to this record should be
	 * removed.
	 */
	virtual void setCategory( Record* rec, const QString& category ) = 0;

	/**
	 * Adds the given category to the record and might do some internal things
	 * needed for category handling in the datastore.
	 * 
	 * All other categories that might have been set to this record should be
	 * unchanged.
	 */
	virtual void addCategory( Record* rec, const QString& category ) = 0;

	/**
	 * Returns true when the proxy was able to open the underlying data store 
	 * in read/write mode.
	 */
	virtual bool isOpen() const = 0;

	/**
	 * Loads all records from underlying data source, sets the start count of the
	 * counter and resets the iterator.
	 */
	virtual void loadAllRecords() = 0;
	
	/**
	 * Tries to create a new Datastore and returns whether or not it succeeded.
	 */
	virtual bool createDataStore() = 0;

protected: // Functions
	/**
	 * Generates a unique id for a new record.
	 */
	virtual QString generateUniqueId() = 0;
	
	/**
	 * This can be reimplemented by subclasses to do things after the regular
	 * commit action. This method is called by commit() after the changes to
	 * records are committed.
	 *
	 * WARNING: Please make sure that you do nothing with records unless:
	 * a) you know what you are doing.
	 * b) you *really* have to
	 */
	virtual bool _commit() { return true; }
	
	/**
	 * This can be implemented by subclasses to undo things from _commit(). This
	 * method is called by rollback() after the changes to the records are undone.
	 */
	bool _rollback() { return true; }
	
	/**
	 * Commits created record @p rec to the datastore. Returns the id that the
	 * data store created for this record.
	 *
	 * @return true when the record was successfully added to the datastore managed
	 *  by this proxy, false otherwise.
	 */
	virtual bool commitCreate( Record *rec ) = 0;
	
	/**
	 * Commits updated record @p rec to the datastore. Must return rec->id() even
	 * if it's unchanged.
	 *
	 * @return true when the changes are successfully committed to the datastore
	 *  managed by this proxy, false otherwise.
	 */
	virtual bool commitUpdate( Record *rec ) = 0;
	
	/**
	 * Delete record @p rec from the datastore.
	 *
	 * @return true when the record is successfully deleted from the datastore
	 *  managed by this proxy, false otherwise.
	 */
	virtual bool commitDelete( Record *rec ) = 0;

protected: // Members
	Mode fMode;
	CUDCounter fCounter;
	QMap<QString, Record*> fRecords;
	QMultiMap<QString, Record*> fRecordsByDescription;
	QMapIterator<QString, Record*> fIterator;
	
	// These are kept for rollback.
	/**
	 * Id's from created records. The bool indicates whether or not the created
	 * records are committed to the data store.
	 */
	QMap<QString, bool> fCreated;
	
	/**
	 * Old values of updated records.
	 */
	QMap<QString, Record*> fOldRecords;
	
	/**
	 * Id's from updated records. The bool indicates whether or not the updated
	 * records are committed to the data store.
	 */
	QMap<QString, bool> fUpdated;
	
	/**
	 * Deleted records;
	 */
	QMap<QString, Record*> fDeletedRecords;
	
	/**
	 * Id's from deleted records. The bool indicates whether or not the records
	 * are deleted from the data store.
	 */
	QMap<QString, bool> fDeleted;
	
	/**
	 * The list of ids that has changed during a commit or a rollback.
	 */
	QMap<QString, QString> fChangedIds;
};
#endif
