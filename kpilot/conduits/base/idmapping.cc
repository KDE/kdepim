/* idmapping.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
** Copyright (C) 2007 by Jason "vanRijn" Kasper
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

#include "options.h"

IDMapping::IDMapping( const QString &userName, const QString &conduit )
	: fSource( userName, conduit )
{
	FUNCTIONSETUP;
	
	fSource.loadMapping();
}

bool IDMapping::isValid( const QList<QString> &ids ) const
{
	FUNCTIONSETUP;
	
	const QMap<QString, QString>* mappings = fSource.constMappings();
	
	bool equalSize = (mappings->size() == ids.size());
	
	if(equalSize)
	{
		bool idsInMapping = true;
		
		QList<QString>::const_iterator i;
		QList<QString> mIds;
		if( fSource.constMappings()->contains( *ids.constBegin() ) )
		{
			// The ids are hanhdeld ids.
			mIds = fSource.constMappings()->keys();
			for( i = mIds.constBegin(); i != mIds.constEnd(); ++i )
			{
				QString id = *i;
				idsInMapping = idsInMapping && ids.contains( id );
			}
		}
		else
		{
			// The ids are pc ids.
			mIds = fSource.constMappings()->values();
			for( i = mIds.constBegin(); i != mIds.constEnd(); ++i )
			{
				QString id = *i;
				idsInMapping = idsInMapping && ids.contains( id );
			}
		}
		
		return idsInMapping;
	}
	
	return false;
}

void IDMapping::map( const QString &hhRecordId, const QString &pcId )
{
	FUNCTIONSETUP;
	
	fSource.mappings()->insert( hhRecordId, pcId );
}

QString IDMapping::pcRecordId( const QString &id ) const
{
	FUNCTIONSETUP;
	
	return fSource.constMappings()->value( id );
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
