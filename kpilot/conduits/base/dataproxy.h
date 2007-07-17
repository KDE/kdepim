#ifndef DATAPROXY_H
#define DATAPROXY_H
/* dataproxy.h			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
** Copyright (C) 2007 by Jason "vanRijn" Kasper
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

#include "cudcounter.h"

#include <QMap>

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
	 * Notifies the proxy that the synchronisation is finished and that
	 * no modifications will be done after this.
	 */
	void syncFinished();

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
	 * Looks for a matching record. Should return 0 if there is no match.
	 */
	Record* find( const QString &id ) const;
	
	/**
	 * Resets the iterator.
	 */
	void resetIterator();
	
	/**
	 * Depending on the iterateMode it should give if there is a next record or if
	 * there is a next modified record.
	 */
	bool hasNext() const;

	/**
	 * Depending on the iterateMode it should give the next record, the next
	 * modified record or 0 if there are no more records to iterate over.
	 */
	Record* next();
	
	/**
	 * Returns true when the proxy was able to open the underlying data store 
	 * in read/write mode.
	 */
	virtual bool isOpen() const = 0;

	/**
	 * Commits all changes to the data store.
	 */
	virtual bool commit() = 0;
	
	/**
	 * Reverts all changes that are committed to the data store.
	 */
	virtual bool rollback() = 0;

	/**
	 * Loads all records from underlying data source, sets the startcount of the
	 * counter and resets the iterator.
	 */
	virtual void loadAllRecords() = 0;
	
protected:
	Mode fMode;
	CUDCounter fCounter;
	QMap<QString, Record*> fRecords;
	QMapIterator<QString, Record*> fIterator;
	
	// These are kept for rollback.
	/**
	 * Id's from created records.
	 */
	QList<QString> fCreated;
	
	/**
	 * Old values of updated records.
	 */
	QList<Record*> fUpdated;
	
	/**
	 * Deleted records;
	 */
	QList<Record*> fDeleted;

private:
	int fLastId;
};
#endif
