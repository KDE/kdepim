/* idmapping.cc			KPilot
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

#include <QtCore/QSet>
#include <QtCore/QtDebug>

#include "idmapping.h"

#include "options.h"

IDMapping::IDMapping( const QString &userName, const QString &conduit )
	: fSource( userName, conduit )
{
	FUNCTIONSETUP;
	
	fSource.loadMapping();
}

bool IDMapping::isValid( const QList<QString>& ids ) const
{
	FUNCTIONSETUP;
	
	const QMap<QString, QString>* mappings = fSource.constMappings();
	
	// If both are empty we have a valid mapping.
	if( ids.isEmpty() && mappings->isEmpty() )
	{
		return true;
	}
	
	// If both have different sizes then the mapping is surely not correct.
	if( ids.size() != mappings->size() )
	{
		return false;
	}
	
	// should be a 1..1 mapping between keys and values
	bool equalSize = (mappings->uniqueKeys().size() == mappings->size() );
	
	if( equalSize && !ids.isEmpty() )
	{
		bool idsInMapping = true;
		
		// now, check for validity of mappings.  *note* we can stop
		// looking if we find at least one problem.
		if( containsHHId( ids.first() ) )
		{
			foreach( const QString& storedId, fSource.constMappings()->keys() )
			{
				if( !ids.contains( storedId ) )
				{
					DEBUGKPILOT << "IDMapping::isValid(): HH Id " << storedId << " is in"
						<< " mapping but does not seem to occur in the give id list.";
					qDebug() << ids;
					return false;
				}
			}
		}
		else
		{
			// The ids are pc ids.
			foreach( const QString& storedId, fSource.constMappings()->values() )
			{
				if( !ids.contains( storedId ) )
				{
					DEBUGKPILOT << "IDMapping::isValid(): PC Id " << storedId << " is in"
						<< " mapping but does not seem to occur in the give id list.";
					return false;
				}
			}
		}
		
		// if we're otherwise valid, double-check and make sure that 
		// we also have only unique values in our mapping...
		if (idsInMapping)
		{
			// build a unique list (set) of values and make sure we
			// have a 1..1 mapping
			QSet<QString> values = mappings->values().toSet();
			idsInMapping = (values.size() == mappings->size() );
			DEBUGKPILOT << "Reverse map integrity: ["
				<< idsInMapping << "]";
		}

		DEBUGKPILOT << "Returning: [" << idsInMapping << "]";

		return idsInMapping;
	}
	
	return false;
}

void IDMapping::map( const QString &hhRecordId, const QString &pcId )
{
	FUNCTIONSETUP;
	
	// check to see if we already have a key with this value
	QString existingHhRecordId = fSource.constMappings()->key( pcId );
	
	// if we already have a key for this one and it isn't the hhRecordId
	// that is being passed in, it's an error
	if( !existingHhRecordId.isEmpty() && existingHhRecordId != hhRecordId )
	{ 
		WARNINGKPILOT << "Error.  pcId:[" << pcId 
			<< "] already mapped to hhRecordId: [" << existingHhRecordId
			<< "].  Shouldn't have same pcId mapped also to incoming: ["
			<< hhRecordId << "].  Removing it.";
		fSource.mappings()->remove( existingHhRecordId );
	}
	
	fSource.mappings()->insert( hhRecordId, pcId );
}

QString IDMapping::pcRecordId( const QString &id ) const
{
	FUNCTIONSETUP;
	
	return fSource.constMappings()->value( id );
}

QStringList IDMapping::pcRecordIds() const
{
	FUNCTIONSETUP;
	
	return fSource.constMappings()->values();
}

QString IDMapping::hhRecordId( const QString &id ) const
{
	FUNCTIONSETUP;

	return fSource.constMappings()->key( id );
}

void IDMapping::removeHHId( const QString &recordId )
{
	FUNCTIONSETUP;
	
	fSource.mappings()->remove( recordId );
}

void IDMapping::removePCId( const QString &recordId )
{
	FUNCTIONSETUP;
	
	// The recordId is one of a pc record.
	QString hhId = fSource.mappings()->key( recordId );
	if( !hhId.isEmpty() )
	{
		fSource.mappings()->remove( hhId );
	}
}

void IDMapping::storeHHCategory( const QString &hhRecordId
                               , const QString &category )
{
	FUNCTIONSETUP;

	if( containsHHId( hhRecordId ) )
	{
		fSource.setHHCategory( hhRecordId, category );
	}
}

QString IDMapping::hhCategory( const QString &hhRecordId ) const
{
	FUNCTIONSETUP;
	
	return fSource.hhCategory( hhRecordId );
}

void IDMapping::storePCCategories( const QString &pcRecordId
                                 , const QStringList &categories )
{
	FUNCTIONSETUP;
	
	if( containsPCId( pcRecordId ) )
	{
		fSource.setPCCategories( pcRecordId, categories );
	}
}

QStringList IDMapping::pcCategories( const QString &pcRecordId ) const
{
	FUNCTIONSETUP;
	
	return fSource.pcCategories( pcRecordId );
}

bool IDMapping::containsHHId( const QString &recordId ) const
{
	FUNCTIONSETUP;
	
	return fSource.constMappings()->contains( recordId );
}

bool IDMapping::containsPCId( const QString &recordId ) const
{
	FUNCTIONSETUP;
	
	return fSource.constMappings()->values().contains( recordId );
}

void IDMapping::changeHHId( const QString &from, const QString &to )
{
	FUNCTIONSETUP;
	
	QString pcId = pcRecordId( from );
	fSource.mappings()->remove( from );
	fSource.mappings()->insert( to, pcId );
}

void IDMapping::changePCId( const QString &from, const QString &to )
{
	FUNCTIONSETUP;
	
	QString hhId = hhRecordId( from );
	fSource.mappings()->insert( hhId, to );
}

bool IDMapping::isArchivedRecord( const QString &pcRecordId ) const
{
	FUNCTIONSETUP;
	
	return fSource.constArchivedRecords()->contains( pcRecordId );
}

void IDMapping::archiveRecord( const QString &hhRecordId )
{
	FUNCTIONSETUP;
	
	if( containsHHId( hhRecordId ) )
	{
		QString pcRecordId = this->pcRecordId( hhRecordId );
		fSource.archivedRecords()->append( pcRecordId );
	}
}

QDateTime IDMapping::lastSyncedDate() const
{
	return fSource.lastSyncedDate();
}

void IDMapping::setLastSyncedDate( const QDateTime &dateTime )
{
	FUNCTIONSETUP;
	
	fSource.setLastSyncedDate( dateTime );
}

void IDMapping::setLastSyncedPC( const QString &pc )
{
	FUNCTIONSETUP;
	
	fSource.setLastSyncedPC( pc );
}

bool IDMapping::commit()
{
	FUNCTIONSETUP;
	
	return fSource.saveMapping();
}

bool IDMapping::rollback()
{
	FUNCTIONSETUP;
	
	return fSource.rollback();
}
