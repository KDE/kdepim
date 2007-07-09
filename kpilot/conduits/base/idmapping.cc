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

IDMapping::IDMapping( const QString &userName, const QString &conduitName )
{
	#warning Not implemented!
	Q_UNUSED( conduitName );
	Q_UNUSED( userName );
}

bool IDMapping::isValid( const QList<QString> &hhIds )
{
	#warning Not implemented!
	Q_UNUSED( hhIds );
	return false;
}

QString IDMapping::recordId( const QString &id )
{
	#warning Not implemented!
	Q_UNUSED( id );
	return QString();
}

void IDMapping::setLastSyncedDate( const QDateTime &dateTime )
{
	Q_UNUSED( dateTime );
	#warning Not implemented!
}

void IDMapping::setLastSyncedPC( const QString &pc )
{
	Q_UNUSED( pc );
	#warning Not implemented!
}

bool IDMapping::commit()
{
	#warning Not implemented!
	return false;
}

bool IDMapping::rollback()
{
	#warning Not implemented!
	return false;
}

void IDMapping::map( const QString &hhRecordId, const QString &pcId )
{
	Q_UNUSED( hhRecordId );
	Q_UNUSED( pcId );
	#warning Not implemented!
}

void IDMapping::remove( const QString &recordId )
{
	Q_UNUSED( recordId );
	#warning Not implemented!
}

bool IDMapping::contains( const QString &hhRecordId )
{
	Q_UNUSED( hhRecordId );
	#warning Not implemented!
	return false;
}
