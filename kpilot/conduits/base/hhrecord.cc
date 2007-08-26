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

HHRecord::HHRecord( PilotRecord *record, PilotAppInfoBase *appInfo )
	: fRecord( record ), fAppInfo( appInfo )
{
}

HHRecord::~HHRecord()
{
	delete fRecord;
}

void HHRecord::setCategoryNames( const QStringList &cats )
{
	FUNCTIONSETUP;
	
	if( cats.size() < 1 )
	{
		fCategory = i18nc( "No category set for this record", "Unfiled" );
		fRecord->setCategory( 0 );
	}
	else
	{
		if( fAppInfo )
		{
			QStringList aiCats = Pilot::categoryNames( fAppInfo->categoryInfo() );
			
			if( aiCats.contains( cats.first() ) )
			{
				DEBUGKPILOT << "Changing category from " << fCategory << " to "
					<< cats.first();

				fCategory = cats.first();
				// We safely can assume that findCategory won't fail.
				
				// FIXME: For some weird reason fAppInfo->findCategory( fCategory );
				// Fails. Below i do exactly the same but this works. *magic* :S.
				int fCatId = -1;
				
				for( unsigned int i = 0; i < Pilot::CATEGORY_COUNT; i++ )
				{
					if( fAppInfo->categoryName( i ) == fCategory )
					{
						fCatId = i;
					}
				}
				
				fRecord->setCategory( fCatId );
			}
			else
			{
				// Category does not exist in appinfo, set it to unfiled.
				DEBUGKPILOT << "Category " << cats.first() 
					<< " does not exist, setting to unfiled.";
					
				fCategory = i18nc( "No category set for this record", "Unfiled" );
				fRecord->setCategory( 0 );
			}
		}
		else
		{
			DEBUGKPILOT << "fAppInfo not initialized, category not changed";
		}
	}
}

QStringList HHRecord::categoryNames() const
{
	FUNCTIONSETUP;
	
	QStringList categories;
	categories << fCategory;
	
	return categories;
}

PilotRecord* HHRecord::pilotRecord() const
{
	FUNCTIONSETUP;
	
	return fRecord;
}

PilotAppInfoBase* HHRecord::appInfo() const
{
	FUNCTIONSETUP;
	
	return fAppInfo;
}

void HHRecord::setCategory( int catId, const QString name )
{
	FUNCTIONSETUP;
	
	DEBUGKPILOT << "Setting category: " << name << " which has id: " << catId;
	
	fCategory = name;
	if( fRecord )
	{
		fRecord->setCategory( catId );
	}
}

QString HHRecord::categoryName() const
{
	FUNCTIONSETUP;
	
	return fCategory;
}

int HHRecord::categoryId() const
{
	FUNCTIONSETUP;
	
	if( fRecord )
	{
		return fRecord->category();
	}
	
	// No record return unfiled.
	return Pilot::Unfiled;
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
