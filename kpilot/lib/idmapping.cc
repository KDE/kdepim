/*
** Copyright (C) 2006 Bertjan Broeksema <bbroeksema@bluebottle.com>
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

IDMapping::IDMapping()
{
}

IDMapping::IDMapping( const QString &conduit )
{
	fConduit = conduit;
	fPid = 0;
}

IDMapping::IDMapping( const IDMapping &m )
{
	fConduit = m.fConduit;
	fUid = m.fUid;
	fPid = m.fPid;
	fLastSync = m.fLastSync;
}

IDMapping IDMapping::operator=( const IDMapping &m )
{
	IDMapping local( m.fConduit );
	local.fUid = m.fUid;
	local.fPid = m.fPid;
	local.fLastSync = m.fLastSync;
	
	return local;
}

void IDMapping::setUid( const QString &uid )
{
	fUid = uid;
}

void IDMapping::setPid( recordid_t pid )
{
	fPid = pid;
}

void IDMapping::setLastSyncTime( const QDateTime &datetime )
{
	fLastSync = datetime;
}

QString IDMapping::conduit() const
{
	return fConduit;
}

QString IDMapping::uid() const
{
	return fUid;
}

recordid_t IDMapping::pid() const
{
	return fPid;
}

QDateTime IDMapping::lastSyncTime() const
{
	return fLastSync;
}
