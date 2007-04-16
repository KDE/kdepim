#ifndef _KPILOT_IDMAPPING_H
#define _KPILOT_IDMAPPING_H
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


#include "pi-macros.h"

#include <qstring.h>
#include <qdatetime.h>

class IDMapping
{
public:
	IDMapping();
	
	IDMapping( const QString &conduit );
	
	IDMapping( const IDMapping &m );
	
	IDMapping operator=( const IDMapping &m );
	
	void setUid( const QString &uid );
	
	void setPid( recordid_t uid );
	
	void setLastSyncTime( const QDateTime &datetime );
	
	QString conduit() const;
	
	QString uid() const;
	
	recordid_t pid() const;
	
	QDateTime lastSyncTime() const;

private:
	QString fConduit;
	QString fUid;
	recordid_t fPid;
	QDateTime fLastSync;
};

#endif
