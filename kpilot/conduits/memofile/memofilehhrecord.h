#ifndef MEMOFILEHHRECORD_H
#define MEMOFILEHHRECORD_H
/* memofilehhrecord.h			KPilot
**
** Copyright (C) 2008 by Jason 'vanRijn' Kasper <vR@movingparts.net>
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

class PilotMemo;

class MemofileHHRecord : public HHRecord
{
public:
	MemofileHHRecord( PilotRecord *record, const QString &category );

	/**
	 * Returns whether or not the current record is equal to @p other. Implementing
	 * conduits should add support for both implementing records for this. This
	 * means that if pcRec->equal( hhRec ) is true, then also hhRec->equal( pcRec )
	 * should be true.
	 */
	/* virtual */ bool equal( const HHRecord* other ) const;

	/**
	 * Returns the PilotMemo object represented by this record.
	 */
	PilotMemo pilotMemo() const;

	/**
	 * Sets the pilot memo represented by this record.
	 */
	void setPilotMemo( const PilotMemo& memo );

	/**
	 * Returns a string presentation of this record.
	 */
	/* virtual */ QString toString() const;
};

#endif
