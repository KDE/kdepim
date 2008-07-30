#ifndef AKONADICONTACT_H
#define AKONADICONTACT_H
/* akonadicontact.h			KPilot
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

#include "record.h"

#include <akonadi/item.h>

#include <QtCore/QDateTime>

class AkonadiRecordPrivate;

class AkonadiRecord : public Record
{
public:
	/**
	 * Creates a AkonadiContact record from an existing item of the data store.
	 * @p lastSync is needed to determine if the record is changed after the last
	 * sync.
	 */
	AkonadiRecord( const Akonadi::Item& item, const QDateTime& lastSync );

	/**
	 * Creates an AkonadiRecord which is marked for deletion.
	 */
	AkonadiRecord( const QString& id );
	
	/**
	 * Destroys the AkonadiRecord object.
	 */
	~AkonadiRecord();
	
	/**
	 * Returns the id of this record.
	 */
	/* virtual */ const QString id() const;

	/**
	 * Returns a reference of the Akonadi::Item used to represent this contact.
	 */
	Akonadi::Item& item();

	/**
	 * Returns true when this record is marked for deletion.
	 */
	/* virtual */ bool isDeleted() const;
	
	/**
	 * Returns true if the record is modified and/or if it's marked as deleted.
	 */
	/* virtual */ bool isModified() const;

	/**
	 * Sets the id of this record to @p id;
	 */
	/* virtual */ void setId( const QString &id );

	/**
	 * Notify the record that syncing is finished so that it can reset flags.
	 * After calling this function Record::isModified() should return false.
	 */
	/* virtual */ void synced();

private:
	QSharedDataPointer<AkonadiRecordPrivate> d;
};

#endif
