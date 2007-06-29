/* idmapping.h			KPilot
**
** Copyright (C) 2004-2007 by Bertjan Broeksema
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

IDMapping::IDMapping( const QString &conduitName )
{
	Q_UNUSED( conduitName );
}

bool IDMapping::isValid( const DataProxy *proxy )
{
	Q_UNUSED( proxy );
	return false;
}

QString IDMapping::pcRecordId() 
{
	return QString( "Implement" );
}

recordid_t IDMapping::hhRecordId()
{
	return 0;
}

void IDMapping::setLastSyncedDate()
{
}

void IDMapping::setLastSyncedPC()
{
}

void IDMapping::save()
{
}

void IDMapping::setPCId()
{
}

void IDMapping::setHHId()
{
}

void IDMapping::map()
{
}

bool IDMapping::contains()
{
	return false;
}

