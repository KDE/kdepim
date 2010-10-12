#ifndef _KPILOT_IDMAPPER_H
#define _KPILOT_IDMAPPER_H
/*
** Copyright (C) 2006 Bertjan Broeksema <bbroeksema@bluebottle.com>
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

#include <qstring.h>
#include <qdatetime.h>
#include <qvaluelist.h>

#include "pi-macros.h"

#include <kconfig.h>

class IDMapperPrivate;

/**
 * Much of the conduits are recordbased. This class can be used bij the conduits
 * to keep track of the mapping between records on the handheld and records on
 * the pc.
 */
class IDMapper
{
public:
	/**
	 * Creates a new IDMapper with default datasource.
	 */
	IDMapper();
	
	/**
	 * Creates a new IDMapper with file as datasource.
	 */
	IDMapper( const QString &file );
	
	~IDMapper();
	
	/**
	 * Adds an uid for PC objects to the database.
	 */
	void registerPCObjectId( const QString &conduit, const QString &uid );
	
	/**
	 * Returns all known uid's for given conduit.
	 */
	QValueList<QString> getPCObjectIds( const QString &conduit );
	
	/**
	 * Adds a pid for HH objects to the database.
	 */
	void registerHHObjectId( const QString &conduit, recordid_t pid );
	
	/**
	 * Returns all know pids for given conduit.
	 */
	QValueList<recordid_t> getHHObjectIds( const QString &conduit );
	
	/**
	 * Sets the PC uid for the handheld record with pid. Does nothing when
	 * there is no handheld record with pid.
	 */
	void setPCObjectId( const QString &conduit, recordid_t pid
		, const QString &uid );
	
	/**
	 * Sets the PC uid for the handheld record with pid. Does nothing when
	 * there is no handheld record with pid.
	 */
	void setHHObjectId( const QString &conduit, const QString &uid
		, recordid_t pid );
	
	/**
	 * Returns the PC uid for the handheld record with pid. Returns 0 when no
	 * pid exists for given uid.
	 */
	recordid_t getHHObjectId( const QString &conduit, const QString &uid );
	
	/**
	 * Returns the HH pid for the PC record with uid. Returns an empty string
	 * when no uid exists for given pid.
	 */
	QString getHHObjectId( const QString &conduit, recordid_t pid );
	
	/**
	 * Returns true when there is a uid set for given pid. The conduit itself
	 * must determine if the two objects are in sync if this function returns
	 * true.
	 */
	bool hasPCId( const QString &conduit, recordid_t pid );
	
	/**
	 * Returns true when there is a pid set for given uid. The conduit itself
	 * must determine if the two objects are in sync if this function returns
	 * true.
	 */
	bool hasHHId( const QString &conduit, const QString &uid );
	
	/**
	 * Sets the time that the two objects where last synced. The conduits
	 * should call this method (or the pid version) when two objects are synced.
	 * When the uid does not exist nothing happens.
	 */
	void setLastSyncTime( const QString &conduit, const QString &uid,
		const QDateTime &date );
	
	/**
	 * Sets the time that the two objects where last synced. The conduits
	 * should call this (or the uid version) method when two objects are synced.
	 * When the pid does not exist nothing happens.
	 */
	void setLastSyncTime( const QString &conduit, recordid_t pid
		, const QDateTime &date );
	
	/**
	 * Returns the date/time for the last time that the item with uid was
	 * synced. This date is set by:
	 * - setLastSyncTime (uid/pid)
	 *
	 * Returns a null datetime when the pid does not excist.
	 */
	QDateTime lastTimeSynced( const QString &conduit, const QString &uid );
	
	/**
	 * Returns the date/time for the last time that the item with pid was
	 * synced. This date is set by:
	 * - setLastSyncTime (uid/pid)
	 *
	 * Returns a null datetime when the pid does not excist.
	 */
	QDateTime lastTimeSynced( const QString &conduit, recordid_t pid );

protected:
	bool openDatasource( const QString &file );

private:
	IDMapperPrivate *fP;
};

#endif
