#ifndef CALENDARAKONADIRECORD_H
#define CALENDARAKONADIRECORD_H
/* calendarakonadirecord.h			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
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

#include "akonadirecord.h"

class CalendarAkonadiRecord : public AkonadiRecord
{
public:
	/**
	 * Creates a AkonadiContact record from an existing item of the data store.
	 * @p lastSync is needed to determine if the record is changed after the last
	 * sync.
	 */
	CalendarAkonadiRecord( const Akonadi::Item& i, const QDateTime& lastSync );
	
	/**
	 * Creates an empty record with given id which is marked for deletion.
	 */
	CalendarAkonadiRecord( const QString& id );

	~CalendarAkonadiRecord();

	void addCategory( const QString& category );

	/**
	 * Returns the number of categories that is set for this record.
	 */
	/* virtual */ int categoryCount() const;
	
	/**
	 * Returns whether or not the given category is set for this record.
	 */
	/* virtual */ bool containsCategory( const QString& category ) const;

	/**
	 * Returns the list of categories set for this record.
	 */
	/* virtual */ QStringList categories() const;

	/**
	 * Returns a string representation of the record.
	 */
	/* virtual */ QString toString() const;

	/**
	 * Validity check.
	 */
	/* virtual */ bool isValid() const;
};

#endif
