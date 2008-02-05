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

#include "hhcategory.h"
#include "options.h"
#include "pilot.h"
#include "pilotRecord.h"
#include "pilotAppInfo.h"

HHRecord::HHRecord( PilotRecord *record ) : fRecord( record )
{
}

HHRecord::~HHRecord()
{
	delete fRecord;
}

void HHRecord::setCategoryNames( const QStringList &cats )
{
	FUNCTIONSETUP;
	Q_UNUSED( cats );
	// FIXME: This type of logic should not be in the datamodel. Handle this in a
	// more appropriate place like the hhdataproxy class.
	
	/*
	int fCatId = -1;
	
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
			
			
			// order of operations:  first, see if the category already exists in our appInfo.  If 
			// it does, then set it to that one.  If it does not, try to add the new category string
			// into our appInfo.  If that doesn't work, then just use Unfiled.
			if( aiCats.contains( cats.first() ) )
			{
				DEBUGKPILOT << "Changing category from " << fCategory << " to "
					<< cats.first();

				fCategory = cats.first();
				// We safely can assume that findCategory won't fail.
				
				// FIXME: For some weird reason fAppInfo->findCategory( fCategory );
				// Fails. Below i do exactly the same but this works. *magic* :S.
				
				for( unsigned int i = 0; i < Pilot::CATEGORY_COUNT; i++ )
				{
					if( fAppInfo->categoryName( i ) == fCategory )
					{
						fCatId = i;
						break;
					}
				}
				
				fRecord->setCategory( fCatId );
			}
			else 
			{
				// Category does not exist in appinfo.  First, see if we can add the new category to 
				// our appInfo.  If we cannot, then set the category to Unfiled.
				fCatId = Pilot::insertCategory(fAppInfo->categoryInfo(), cats.first(), false);
				
				if (fCatId > 0)
				{
					DEBUGKPILOT << "Category: [" << cats.first() 
						<< "] added to database at category id: [" << fCatId << "]";
					
					fCategory = cats.first();
					fRecord->setCategory( fCatId );
				}
				else 
				{
					DEBUGKPILOT << "Category: [" << cats.first() 
						<< "] does not exist and we can't add it. Setting to unfiled.";
						
					fCategory = i18nc( "No category set for this record", "Unfiled" );
					fRecord->setCategory( 0 );
				}
			}
		}
		else
		{
			DEBUGKPILOT << "fAppInfo not initialized, category not changed";
		}
	}
	*/
}

QStringList HHRecord::categoryNames() const
{
	FUNCTIONSETUP;
	
	QStringList categories;
	categories << fCategory.name();
	
	return categories;
}

PilotRecord* HHRecord::pilotRecord() const
{
	FUNCTIONSETUP;
	
	return fRecord;
}


void HHRecord::setCategory( const HHCategory &cat )
{
	FUNCTIONSETUP;
	
	fCategory = cat;
	
	if( fRecord )
	{
		fRecord->setCategory( cat.index() );
	}
}

HHCategory HHRecord::category() const
{
	FUNCTIONSETUP;
	return fCategory;
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
