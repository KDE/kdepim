/* hhrecord.h			KPilot
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

#include "options.h"
#include "pilotRecord.h"

HHRecord::HHRecord( PilotRecord *record )
{
	fRecord = record;
}

HHRecord::~HHRecord()
{
	delete fRecord;
}

PilotRecord* HHRecord::pilotRecord() const
{
	return fRecord;
}

bool HHRecord::isArchived() const
{
	FUNCTIONSETUP;
	
	return fRecord->isArchived();
}

void HHRecord::setArchived()
{
	FUNCTIONSETUP;
	
	fRecord->setDeleted();
	fRecord->setArchived();
}

/** Record methods */

const QString HHRecord::id() const
{
	FUNCTIONSETUP;
	
	return QString::number( fRecord->id() );
}

void HHRecord::setId( const QString &id )
{
	FUNCTIONSETUP;
	
	bool converted;
	recordid_t rid = id.toULong( &converted );
	
	if( !converted )
	{
		DEBUGKPILOT <<"Could not convert " << id << " to ulong. Id not set!";
		return;
	}
	else
	{
		fRecord->setID( rid );
	}
}

bool HHRecord::isModified() const
{
	return fRecord->isModified() || isDeleted();
}

bool HHRecord::isDeleted() const
{
	return fRecord->isDeleted();
}

void HHRecord::synced()
{
	fRecord->setDeleted( false );
	fRecord->setModified( false );
}

QString HHRecord::toString() const
{
	return fRecord->textRepresentation();
}
