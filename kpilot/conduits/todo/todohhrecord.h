#ifndef TODOHHRECORD_H
#define TODOHHRECORD_H
/* todohhrecord.h			KPilot
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

#include "pilotTodoEntry.h"

class TodoHHRecord : public HHRecord
{
public:
	TodoHHRecord( PilotRecord *record, const QString &category );
	
	/* virtual */ bool equal( const HHRecord* other ) const;
	
	virtual QString description() const;

	/**
	 * Updates the PilotRecord with the data in entry. If @param keepPrevCategory
	 * is true then the category that this record had before the call we be saved.
	 * otherwise it will be equal to wathever is set in entry.
	 */
	void setTodoEntry( const PilotTodoEntry& entry, bool keepPrevCategory = true );
	
	PilotTodoEntry todoEntry() const;

	/**
	 * Returns a string presentation of this record.
	 */
	/* virtual */ QString toString() const;

};

#endif
