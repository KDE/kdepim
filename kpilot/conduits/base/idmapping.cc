/* idmapping.h			KPilot
**
** Copyright (C) 2004-2007 by Bertjan Broeksema
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

bool IDMapping::isValid( const QList<QString> &ids )
{
	FUNCTIONSETUP;
	
	const QMap<QString, QString>* mappings = fSource.mappings();
	
	bool equalSize = (mappings->size() == ids.size());
	
	if(equalSize)
	{
		bool idsInMapping = true;
		
		QList<QString>::const_iterator i;
		QList<QString> mIds;
		if( fSource.mappings()->contains( *ids.constBegin() ) )
		{
			// The ids are hanhdeld ids.
			mIds = fSource.mappings()->keys();
			for( i = mIds.constBegin(); i != mIds.constEnd(); ++i )
			{
				QString id = *i;
				idsInMapping = idsInMapping && ids.contains( id );
			}
		}
		else
		{
			// The ids are pc ids.
			mIds = fSource.mappings()->values();
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

QString IDMapping::recordId( const QString &id )
{
	FUNCTIONSETUP;
	
	if( fSource.mappings()->contains( id ) )
	{
		return fSource.mappings()->value( id );
	}

	return fSource.mappings()->key( id );
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

void IDMapping::map( const QString &hhRecordId, const QString &pcId )
{
	FUNCTIONSETUP;
	
	fSource.mappings()->insert( hhRecordId, pcId );
}

void IDMapping::remove( const QString &recordId )
{
	FUNCTIONSETUP;
	
	if( fSource.mappings()->contains( recordId ) )
	{
		// The recordId is one of a handheld record.
		fSource.mappings()->remove( recordId );
	}
	else
	{
		// The recordId is one of a pc record.
		QString hhId = fSource.mappings()->key( recordId );
		if( !hhId.isEmpty() )
		{
			fSource.mappings()->remove( hhId );
		}
	}
}

bool IDMapping::contains( const QString &recordId )
{
	FUNCTIONSETUP;
	
	return fSource.mappings()->contains( recordId ) ||
		fSource.mappings()->values().contains( recordId );
}
