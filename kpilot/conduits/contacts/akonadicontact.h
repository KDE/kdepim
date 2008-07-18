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

class HHRecord;
class HHContact;

class AkonadiContact : public Record
{
private:
	Akonadi::Item fItem;

public:
	AkonadiContact( const Akonadi::Item& item );
	
	/**
	 * Creates an AkonadiContact object which is a copy of @param other.
	 */
	AkonadiContact( const HHRecord* other );
	
	~AkonadiContact();
	
	/**
	 * Adds the given category to the record.
	 *
	 * All other categories that might have been set to this record remain
	 * unchanged.
	 */
	void addCategory( const QString& category );
	
	/**
	 * Returns the list of categories set for this record.
	 */
	/* virtual */ QStringList categories() const;
	
	/**
	 * Returns the number of categories that is set for this record.
	 */
	/* virtual */ int categoryCount() const;
	
	/**
	 * Returns wether or not the given category is set for this record.
	 */
	/* virtual */ bool containsCategory( const QString& category ) const;
	
	/**
	 * Copies the complete content of this record to the HHContact, overwriting all
	 * values.
	 */
	void copyTo( HHContact* to ) const;
	
	/**
	 * Returns whether or not the current record is equal to @p other. Implementing 
	 * conduits should add support for both implementing records for this. This
	 * means that if pcRec->equal( hhRec ) is true, then also hhRec->equal( pcRec )
	 * should be true.
	 */
	/* virtual */ bool equal( const Record* other ) const;
	
	/**
	 * Returns the id of this record.
	 */
	/* virtual */ const QString id() const;
	
	/**
	 * Returns the Akonadi::Item used to represent this contact.
	 */
	Akonadi::Item item() const;
	
	/**
	 * Returns true when this record is marked for deletion.
	 */
	/* virtual */ bool isDeleted() const;
	
	/**
	 * Returns true if the record is modified and/or if it's marked as deleted.
	 */
	/* virtual */ bool isModified() const;
	
	/**
	 * Sets the given category as the only category to the record.
	 * 
	 * All other categories that might have been set to this record are removed.
	 */
	void setCategory( const QString& category );
	
	/**
	 * Sets the id of this record to @p id;
	 */
	/* virtual */ void setId( const QString &id );
	
	/**
	 * Notify the record that syncing is finished so that it can reset flags.
	 * After calling this function Record::isModified() should return false.
	 */
	/* virtual */ void synced();

	/**
	 * Returns a string representation of the record.
	 */
	/* virtual */ QString toString() const;
};

#endif
