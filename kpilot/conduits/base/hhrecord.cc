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
#include "pilot.h"
#include "pilotRecord.h"
#include "pilotAppInfo.h"

HHRecord::HHRecord( PilotRecord *record, const QString &category ) 
	: fRecord( record ), fCategory( category )
{
}

HHRecord::~HHRecord()
{
	delete fRecord;
}

PilotRecord* HHRecord::pilotRecord() const
{
	FUNCTIONSETUP;

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
	if( fTempId.isEmpty() )
	{
		return QString::number( fRecord->id() );
	}
	else
	{
		return fTempId;
	}
}

void HHRecord::setId( const QString &id )
{
	FUNCTIONSETUP;

	// ids < 0 are temporary ids
	if( id.toLongLong() < 0 )
	{
		fTempId = id;
	}
	else
	{
		bool converted;
		recordid_t rid = id.toULong( &converted );

		if( !converted )
		{
			DEBUGKPILOT <<"Could not convert " << id << " to ulong. Id not set!";
			return;
		}
		else
		{
			fTempId.clear();
			fRecord->setID( rid );
		}
	}
}


int HHRecord::categoryCount() const
{
	FUNCTIONSETUP;
	
	return 1;
}

bool HHRecord::containsCategory( const QString& category ) const
{
	FUNCTIONSETUP;
	
	return fCategory == category;
}

QStringList HHRecord::categories() const
{
	FUNCTIONSETUP;
	
	QStringList categories;
	categories << fCategory;
	return categories;
}


QString HHRecord::category() const
{
	FUNCTIONSETUP;

	return fCategory;
}

void HHRecord::setCategory( int id, const QString& category )
{
	FUNCTIONSETUP;
	
	fCategory = category;
	fRecord->setCategory( id );
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
