#ifndef RECORD_H
#define RECORD_H
/* record.h			KPilot
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

#include <QVariant>
#include <QStringList>

#include "kpilot_export.h"

/**
 * If subclassing Record, the onlything that should be reimplemented is the
 * isValid() function. If other methods are reimplemented it's not garanteed
 * that the conduit will work as expected. If you miss a feature which should be
 * in this class, contact the kpilot developers.
 */
class KPILOT_EXPORT Record {
	
public:
	virtual ~Record() {};
	
	virtual const QString id() const = 0;
	
	/**
	 * Sets the id of this record to @p id;
	 */
	virtual void setId( const QString &id ) = 0;

	/**
	 * Returns true if the record is modified and/or if it's marked as deleted.
	 */
	virtual bool isModified() const = 0;
	
	/**
	 * Returns true when this record is marked for deletion.
	 */
	virtual bool isDeleted() const = 0;
	
	/**
	 * Notify the record that syncing is finished so that it can reset flags.
	 * After calling this function Record::isModified() should return false.
	 */
	virtual void synced() = 0;
	
	/**
	 * Returns a string representation of the record.
	 */
	virtual QString toString() const = 0;
	
	/**
	 * Returns wheter or not the current record is equal to @p other. Implementing 
	 * conduits should add support for both implementing records for this. This
	 * means that if pcRec->equal( hhRec ) is true, then also hhRec->equal( pcRec )
	 * should be true.
	 */
	virtual bool equal( const Record* other ) const = 0;
};
#endif
