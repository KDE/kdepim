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

class Record {
public:
	virtual ~Record() = 0;
	
	virtual const QString id() const = 0;
	
	virtual const QString setId( const QString &id ) const = 0;

	/**
	 * Returns true if the record knows that it's modified since last sync.
	 */
	virtual bool isModified() const = 0;
	
	/**
	 * Returns the list of fields that this record has.
	 */
	virtual const QStringList fields() const = 0;
	
	/**
	 * Sets the value of @p field to @p value and returns true. Returns false if 
	 * the field does not exists or if the value is not of an appropriate type for
	 * the field.
	 */
	virtual bool setValue( const QString &field, const QVariant &value ) = 0;

	/**
	 * Returns the value for @p field or an invalid QVariant if the field does not
	 * exists.
	 */
	virtual const QVariant value( const QString &field ) const = 0;
	
	virtual QString toString() const = 0;
};
#endif
