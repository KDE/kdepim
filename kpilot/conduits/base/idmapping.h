#ifndef IDMAPPING_H
#define IDMAPPING_H
/* idmapping.h			KPilot
**
** Copyright (C) 2004-2007 by Bertjan Broeksema
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

#include <QDateTime>
#include <QString>
#include <QList>

#include "pi-macros.h"

class DataProxy;

class IDMapping {
private:
	QString fConduitName;
public:
	/**
	 * $HOME/.kde/share/apps/kpilot/conduits/<Username>/mapping/<Conduit>-mapping.xml.
	 */
	IDMapping( const QString &userName, const QString &conduitname );

	/**
	 * Validates the mapping file with given dataproxy. The mapping is considered 
	 * valid if:
	 * 1. The number of mappings matches the number of records in the list.
	 * 2. Every record that is in the backup database has a mapping.
	 */
	bool isValid( const QList<QString> &hhIds );

	/**
	 * Searches for a mapping which contains @p id and returns the id to which it
	 * is mapped.
	 */
	QString recordId( const QString &id );

	void setLastSyncedDate( const QDateTime &dateTime );

	void setLastSyncedPC( const QString &pc );

	/**
	 *
	 */
	bool commit();
	
	bool rollback();

	/**
	 * Deletes any mapping that exists for @p hhRecordId and @p pcRecordId and 
	 * then creates a new mapping between @p hhRecordId and @p pcRecordId.
	 */
	void map( const QString &hhRecordId, const QString &pcRecordId );
	
	/**
	 * Search for a mapping which contains @p recordId and if one is found it will
	 * be removed.
	 */
	void remove( const QString &recordId );

	bool contains( const QString &id );
};
#endif
