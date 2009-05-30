#ifndef CONTACTSHHRECORD_H
#define CONTACTSHHRECORD_H
/* contactshhrecord.h			KPilot
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

#include "hhrecord.h"

class PilotAddress;

class ContactsHHRecord : public HHRecord
{
public:
	ContactsHHRecord( PilotRecord *record, const QString &category );

	virtual QString description() const;

	/**
	 * Returns whether or not the current record is equal to @p other. Implementing 
	 * conduits should add support for both implementing records for this. This
	 * means that if pcRec->equal( hhRec ) is true, then also hhRec->equal( pcRec )
	 * should be true.
	 */
	/* virtual */ bool equal( const HHRecord* other ) const;
	
	/**
	 * Returns the PilotAddress object represented by this record.
	 */
	PilotAddress pilotAddress() const;

	/**
	 * Sets the pilot address represented by this record.
	 */
	void setPilotAddress( const PilotAddress& address );
	
	/**
	 * Returns a string presentation of this record.
	 */
	/* virtual */ QString toString() const;
};

#endif
