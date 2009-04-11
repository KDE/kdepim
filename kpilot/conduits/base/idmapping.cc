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

#include "idmapping.h"

#include <QtCore/QSet>
#include <QtCore/QtDebug>

#include "idmappingxmlsource.h"
#include "options.h"

class IDMappingPrivate : public QSharedData
{
public:
	IDMappingPrivate( const QString &userName, const QString &conduit )
		: fSource( userName, conduit )
	{
	}
	
	IDMappingPrivate()
	{
	}
	
	IDMappingPrivate( const IDMappingPrivate& other ) : QSharedData( other )
	{
		fSource = other.fSource;
	}
	
	IDMappingXmlSource fSource;
};

IDMapping::IDMapping() : d( new IDMappingPrivate )
{
}

IDMapping::IDMapping( const QString &userName, const QString &conduit )
	: d( new IDMappingPrivate( userName, conduit ) )
{
	FUNCTIONSETUP;
	
	d->fSource.loadMapping();
}

IDMapping::IDMapping( const IDMapping& other ) : d( other.d )
{
}

IDMapping::~IDMapping()
{
}

void IDMapping::archiveRecord( const QString &hhRecordId )
{
	FUNCTIONSETUP;
	
	if( containsHHId( hhRecordId ) )
	{
		QString pcRecordId = this->pcRecordId( hhRecordId );
		d->fSource.archivedRecords()->append( pcRecordId );
	}
}

void IDMapping::changeHHId( const QString &from, const QString &to )
{
	FUNCTIONSETUP;
	
	QString pcId = pcRecordId( from );
	d->fSource.mappings()->remove( from );
	d->fSource.mappings()->insert( to, pcId );
}

void IDMapping::changePCId( const QString &from, const QString &to )
{
	FUNCTIONSETUP;
	
	QString hhId = hhRecordId( from );
	d->fSource.mappings()->insert( hhId, to );
}

bool IDMapping::commit()
{
	FUNCTIONSETUP;
	
	return d->fSource.saveMapping();
}

bool IDMapping::containsHHId( const QString &recordId ) const
{
	FUNCTIONSETUP;
	
	return d->fSource.constMappings()->contains( recordId );
}

bool IDMapping::containsPCId( const QString &recordId ) const
{
	FUNCTIONSETUP;
	
	return d->fSource.constMappings()->values().contains( recordId );
}

QString IDMapping::hhCategory( const QString &hhRecordId ) const
{
	FUNCTIONSETUP;
	
	return d->fSource.hhCategory( hhRecordId );
}

QString IDMapping::hhRecordId( const QString &id ) const
{
	FUNCTIONSETUP;

	return d->fSource.constMappings()->key( id );
}

bool IDMapping::isArchivedRecord( const QString &pcRecordId ) const
{
	FUNCTIONSETUP;
	
	return d->fSource.constArchivedRecords()->contains( pcRecordId );
}

bool IDMapping::isValid( const QList<QString>& ids ) const
{
	FUNCTIONSETUP;
	
	const QMap<QString, QString>* mappings = d->fSource.constMappings();
	
	// If both are empty we have a valid mapping.
	if( ids.isEmpty() && mappings->isEmpty() )
	{
		return true;
	}
	
	DEBUGKPILOT << "At least one contains items.";
	
	// If both have different sizes then the mapping is surely not correct.
	if( ids.size() != mappings->size() )
	{
		DEBUGKPILOT << "Unequal size: given[" << ids.size() << "], mapping[" << mappings->size() << ']';
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
			foreach( const QString& storedId, d->fSource.constMappings()->keys() )
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
			foreach( const QString& storedId, *(d->fSource.constMappings()) )
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

QDateTime IDMapping::lastSyncedDate() const
{
	return d->fSource.lastSyncedDate();
}


void IDMapping::map( const QString &hhRecordId, const QString &pcId )
{
	FUNCTIONSETUP;
	
	// check to see if we already have a key with this value
	QString existingHhRecordId = d->fSource.constMappings()->key( pcId );
	
	// if we already have a key for this one and it isn't the hhRecordId
	// that is being passed in, it's an error
	if( !existingHhRecordId.isEmpty() && existingHhRecordId != hhRecordId )
	{ 
		WARNINGKPILOT << "Error.  pcId:[" << pcId 
			<< "] already mapped to hhRecordId: [" << existingHhRecordId
			<< "].  Should not have same pcId mapped also to incoming: ["
			<< hhRecordId << "].  Removing it.";
		d->fSource.mappings()->remove( existingHhRecordId );
	}
	
	d->fSource.mappings()->insert( hhRecordId, pcId );
}

QStringList IDMapping::pcCategories( const QString &pcRecordId ) const
{
	FUNCTIONSETUP;
	
	return d->fSource.pcCategories( pcRecordId );
}

QString IDMapping::pcRecordId( const QString &id ) const
{
	FUNCTIONSETUP;
	
	return d->fSource.constMappings()->value( id );
}

QStringList IDMapping::pcRecordIds() const
{
	FUNCTIONSETUP;
	
	return d->fSource.constMappings()->values();
}

void IDMapping::removeHHId( const QString &recordId )
{
	FUNCTIONSETUP;
	
	d->fSource.mappings()->remove( recordId );
}

void IDMapping::removePCId( const QString &recordId )
{
	FUNCTIONSETUP;
	
	// The recordId is one of a pc record.
	QString hhId = d->fSource.mappings()->key( recordId );
	if( !hhId.isEmpty() )
	{
		d->fSource.mappings()->remove( hhId );
	}
}

bool IDMapping::rollback()
{
	FUNCTIONSETUP;
	
	return d->fSource.rollback();
}

void IDMapping::setLastSyncedDate( const QDateTime &dateTime )
{
	FUNCTIONSETUP;
	
	d->fSource.setLastSyncedDate( dateTime );
}

void IDMapping::setLastSyncedPC( const QString &pc )
{
	FUNCTIONSETUP;
	
	d->fSource.setLastSyncedPC( pc );
}

void IDMapping::storeHHCategory( const QString &hhRecordId
                               , const QString &category )
{
	FUNCTIONSETUP;

	if( containsHHId( hhRecordId ) )
	{
		d->fSource.setHHCategory( hhRecordId, category );
	}
}

void IDMapping::storePCCategories( const QString &pcRecordId
                                 , const QStringList &categories )
{
	FUNCTIONSETUP;
	
	if( containsPCId( pcRecordId ) )
	{
		d->fSource.setPCCategories( pcRecordId, categories );
	}
}

IDMapping& IDMapping::operator=( const IDMapping& other )
{
	if( this != &other )
	{
		d = other.d;
	}
	
	return *this;
}

bool IDMapping::remove()
{
	FUNCTIONSETUP;
	bool success = d->fSource.remove();
	return success && d->fSource.loadMapping(); // Make sure that we reset local structures.
}
