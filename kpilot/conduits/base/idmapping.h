#ifndef IDMAPPING_H
#define IDMAPPING_H
/* idmapping.h			KPilot
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

#include <QtCore/QDateTime>
#include <QtCore/QString>

#include "options.h"
#include "pi-macros.h"

class IDMappingPrivate;

class KPILOT_EXPORT IDMapping
{
public:
	/**
	 * Creates an empty and invalid mapping.
	 */
	IDMapping();
	
	/**
	 * Creates a new mapping object for given user and conduit.
	 */
	IDMapping( const QString &userName, const QString &conduit );
	
	/**
	 * Copy constructor.
	 */
	IDMapping( const IDMapping& other );
	
	/**
	 * Destructor.
	 */
	~IDMapping();
	
	/**
	 * Marks the pcRecord to which @p hhRecordId is mapped as archived. Does
	 * nothing if there exists no mapping.
	 */
	void archiveRecord( const QString &hhRecordId );
	
	/**
	 * Change the HH id @p from to @p to.
	 */
	void changeHHId( const QString &from, const QString &to );
	
	/**
	 * Change the PC id @p from to @p to.
	 */
	void changePCId( const QString &from, const QString &to );
	
	/**
	 * Saves the changes to persistent storage.
	 */
	bool commit();
	
	/**
	 * Method to find out whether or not there is a mapping for hh record with 
	 * @p hhRecordId.
	 */
	bool containsHHId( const QString &hhRecordId ) const;

	/**
	 * Method to find out whether or not there is a mapping for pc record with 
	 * @p pcRecordId.
	 */
	bool containsPCId( const QString &pcRecordId ) const;
	
	/**
	 * Returns the category for the given record or QString() if no category is
	 * set.
	 */
	QString hhCategory( const QString &hhRecordId ) const;
	
	/**
	 * Searches for a mapping which contains @p pcRecordId and returns the id to
	 * which it is mapped. Returns QString() if no mapping is found.
	 */
	QString hhRecordId( const QString &pcRecordId ) const;
	
		/**
	 * Returns true when the record from the pc data store with @p pcRecordId is
	 * marked as an archived record. This means that there was once a record on
	 * the handheld, which is deleted by the user, but was marked to be archived
	 * on the pc.
	 */
	bool isArchivedRecord( const QString &pcRecordId ) const;
	
	/**
	 * Validates the mapping file with given dataproxy. The mapping is considered 
	 * valid if:
	 * 1. The number of mappings matches the number of records in @p ids.
	 * 2. Every id that is in @p recordIds has a mapping.
	 *
	 * NOTE: The list of id's should be a list of handheld ids or a list of pc 
	 * ids. Not a mix of them.
	 */
	bool isValid( const QList<QString> &recordIds ) const;
	
	/**
	 * Returns the last time that a sync occurred.
	 */
	QDateTime lastSyncedDate() const;
	
	/**
	 * Deletes any mapping that exists for @p hhRecordId and @p pcRecordId and 
	 * then creates a new mapping between @p hhRecordId and @p pcRecordId.
	 */
	void map( const QString &hhRecordId, const QString &pcRecordId );
	
	/**
	 * Returns the categories for the given record or QString() if no category is
	 * set.
	 */
	QStringList pcCategories( const QString &pcRecordId ) const;
	
	/**
	 * Searches for a mapping which contains @p hhRecordId and returns the id to
	 * which it is mapped. Returns QString() if no mapping is found.
	 */
	QString pcRecordId( const QString &hhRecordId ) const;
	
	/**
	 * Returns a list of all pc ids that are in the mapping.
	 */
	QStringList pcRecordIds() const;
	
	/**
	 * Search for a mapping for which the hh id == @p hhRecordId and if one is
	 * found it will be removed.
	 */
	void removeHHId( const QString &hhRecordId );
	
	/**
	 * Search for a mapping for which the pc id == @p pcRecordId and if one is
	 * found it will be removed.
	 */
	void removePCId( const QString &pcRecordId );

	/**
	 * Tries to undo the changes in persistent storage.
	 */
	bool rollback();

	/**
	 * Allows for complete removable of persistant storage.
	 */
	bool remove();
	
	/**
	 * Sets the date/time on which the last sync is executed to @p dateTime.
	 */
	void setLastSyncedDate( const QDateTime &dateTime );

	/**
	 * Sets the pc on which the last sync is executed to @p pc.
	 */
	void setLastSyncedPC( const QString &pc );
	
	/**
	 * Stores the category for the record with @p hhRecordId in the mapping. Does
	 * nothing when there is no mapping for @p hhRecordId.
	 */
	void storeHHCategory( const QString &hhRecordId, const QString &category );

	/**
	 * Stores the categories for the record with @p pcRecordId in the mapping.
	 * Does nothing when there is no mapping for @p pcRecordId.
	 */
	void storePCCategories( const QString &pcRecordId
	                      , const QStringList &categories );
	
	/**
	 * Assignment operator.
	 */
	IDMapping& operator=( const IDMapping& other );

private:
	QSharedDataPointer<IDMappingPrivate> d;
};

#endif
