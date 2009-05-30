#ifndef RECORD_H
#define RECORD_H
/* record.h			KPilot
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

#include <QtCore/QStringList>

#include "kpilot_export.h"

/**
 * If subclassing Record, the only thing that should be reimplemented is the
 * isValid() function. If other methods are reimplemented it's not guaranteed
 * that the conduit will work as expected. If you miss a feature which should be
 * in this class, contact the kpilot developers.
 */
class KPILOT_EXPORT Record {
	
public:
	virtual ~Record() {}
	
	/**
	 * Returns the description of this record. We use this to create a faster
	 * lookup for records, which is especially crucial for first sync situations
	 * with large databases. While each implementing class will have its own
	 * interpretation of what this contains (calendar and todo records will use
	 * the summary, contacts will use some form of firstname/lastname, memos will
	 * use title, etc.), what is important is that this contain something
	 * meaningful to both HH and PC records.
	 */
	virtual QString description() const = 0;
	
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
	 * Returns the number of categories that is set for this record.
	 */
	virtual int categoryCount() const = 0;
	
	/**
	 * Returns whether or not the given category is set for this record.
	 */
	virtual bool containsCategory( const QString& category ) const = 0;

	/**
	 * Returns the list of categories set for this record.
	 */
	virtual QStringList categories() const = 0;

	/**
	 * Returns a string representation of the record.
	 */
	virtual QString toString() const = 0;

	/**
	 * Allows subclasses to do some validity checks on the records. (Optional)
	 */
	virtual bool isValid() const { return true; }
};
#endif
