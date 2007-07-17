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
protected:
	/**
	 * Record id
	 */
	QString fId;
	
	/**
	 * The list of supported fields for this record.
	 */
	QStringList fFields;
	
	/**
	 * Values for the fields.
	 */
	QMap<QString, QVariant> fFieldValues;
	
	/**
	 * Indicator for dirtyness of this record.
	 */
	bool fModified;
	
	/**
	 * Indicates wheter or not this record is deleted.
	 */
	bool fDeleted;
	
public:
	Record( const QStringList& fields );
	
	Record( const QStringList& fields, const QString &id );
	
	Record( const Record &other );
	
	virtual ~Record() {}
	
	const QString id() const;
	
	/**
	 * Sets the id of this record to @p id;
	 */
	void setId( const QString &id );

	/**
	 * Returns the value for @p field or an invalid QVariant if the field does not
	 * exists.
	 */
	QVariant value( const QString &field ) const;

	/**
	 * Sets the value of @p field to @p value and returns true. Returns false if 
	 * the field does not exists or if the value is not of an appropriate type for
	 * the field. If this succeeds the record must return true for isModified().
	 */
	bool setValue( const QString &field, const QVariant &value );

	/**
	 * Returns true if the record is modified and if it's marked as deleted.
	 */
	bool isModified() const;
	
	void setModified();
	
	void setDeleted();
	
	/**
	 * Returns true when this record is marked for deletion.
	 */
	bool isDeleted() const;
	
	/**
	 * Notify the record that syncing is finished so that it can reset flags.
	 */
	void synced();
	
	/**
	 * Returns a string representation of the record.
	 */
	QString toString() const;
	
	/**
	 * Returns the list of fields that this record has.
	 */
	const QStringList fields() const { return fFields; }

	/**
	 * Compares the fields and the field values of this record with @p other.
	 */
	bool operator==( const Record &other ) const;
	
	bool operator!=( const Record &other ) const;

protected:
	/**
	 * Virtual function which can be overloaded by the implementing record class
	 * to check if the QVariant @p value contains a valid type/value for @p field.
	 * Default implementation returns always true.
	 */
	virtual bool isValid( const QString &field, const QVariant &value );
};
#endif
