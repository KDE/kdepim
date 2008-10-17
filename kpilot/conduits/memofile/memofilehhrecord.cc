/* memofilehhrecord.cc			KPilot
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

#include "memofilehhrecord.h"

#include <QtCore/QDateTime>

#include "options.h"
#include "pilotMemo.h"

MemofileHHRecord::MemofileHHRecord( PilotRecord *record, const QString &category )
	: HHRecord( record, category )
{
}

bool MemofileHHRecord::equal( const HHRecord* other ) const
{
	FUNCTIONSETUP;

	const MemofileHHRecord* hrOther = static_cast<const MemofileHHRecord*>( other );
	return hrOther->pilotMemo() == PilotMemo( fRecord );
}

PilotMemo MemofileHHRecord::pilotMemo() const
{
	return PilotMemo( fRecord );
}

void MemofileHHRecord::setPilotMemo( const PilotMemo& memo )
{
	// Free the old data.
	KPILOT_DELETE( fRecord );
	// And set it to the updated memo.
	fRecord = memo.pack();
}

QString MemofileHHRecord::toString() const
{
	PilotMemo pa = pilotMemo();
	QString rs = id();
	rs += CSL1( ":" ) + pa.getTitle();
	return rs;
}
