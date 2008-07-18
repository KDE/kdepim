#ifndef HHCONTACT_H
#define HHCONTACT_H
/* hhcontact.h			KPilot
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

#include "hhrecord.h"

#include <kabc/addressee.h>

class Record;
class AkonadiContact;

class HHContact : public HHRecord
{
public:
	HHContact( PilotRecord* rec, const QString& category );
	
	HHContact( const Record* rec );
	
	/**
	 * Returns the addressee object represented by this record.
	 */
	KABC::Addressee addressee() const;
	
	/**
	 * Copies the complete content of this record to the AkonadiContact,
	 * overwriting all values.
	 */
	void copyTo( AkonadiContact* to ) const;
	
	/**
	 * Returns whether or not the current record is equal to @p other. Implementing 
	 * conduits should add support for both implementing records for this. This
	 * means that if pcRec->equal( hhRec ) is true, then also hhRec->equal( pcRec )
	 * should be true.
	 */
	/* virtual */ bool equal( const Record* other ) const;
	
	/**
	 * Returns whether or the addressee represented by this record is equal to
	 * @p addressee.
	 */
	bool equal( const KABC::Addressee& addressee ) const;

	/**
	 * Sets the addressee object represented by this record.
	 */
	void setAddressee( const KABC::Addressee& a ) const;

	/**
	 * Returns a string representation of the record.
	 */
	/* virtual */ QString toString() const;
};

#endif
