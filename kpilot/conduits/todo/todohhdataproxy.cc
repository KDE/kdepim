/* todohhdataproxy.cc			KPilot
**
** Copyright (C) 2008 by Bertjan Broeksema <b.broeksema@kdemail.net>
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

#include "todohhdataproxy.h"

#include "pilotRecord.h"
#include "todohhrecord.h"

bool TodoHHDataProxy::createDataStore()
{
	// TODO: Implement
	return false;
}

TodoHHDataProxy::TodoHHDataProxy( PilotDatabase *db ) : HHDataProxy( db )
{
}

HHRecord* TodoHHDataProxy::createHHRecord( PilotRecord *rec )
{
	QString category = fAppInfo->categoryName( rec->category() );
	return new TodoHHRecord( rec, category );
}

PilotAppInfoBase* TodoHHDataProxy::readAppInfo()
{
	if( fDatabase && fDatabase->isOpen() )
	{
		PilotToDoInfo* appInfo = new PilotToDoInfo( fDatabase );
		
		return appInfo;
	}

	return 0;
}

void TodoHHDataProxy::storeAppInfo()
{
	// NOTE: This function must be removed.
}
