/* memofilehhdataproxy.cc			KPilot
**
** Copyright (C) 2008 by Jason 'vanRijn' Kasper <vR@movingparts.net>
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

#include "memofilehhdataproxy.h"

#include "memofilehhrecord.h"
#include "options.h"
#include "pilotMemo.h"
#include "pilotRecord.h"

class MemofileHHDataProxy::Private
{
public:
	Private() : fMemoInfo( 0L )
	{
	}

	PilotMemoInfo* fMemoInfo;
};


bool MemofileHHDataProxy::createDataStore()
{
	// TODO: Implement
	return false;
}

MemofileHHDataProxy::MemofileHHDataProxy( PilotDatabase *db ) : HHDataProxy( db )
	, d( new Private )
{
}

HHRecord* MemofileHHDataProxy::createHHRecord( PilotRecord *rec )
{
	QString category = fAppInfo->categoryName( rec->category() );
	return new MemofileHHRecord( rec, category );
}

PilotAppInfoBase* MemofileHHDataProxy::readAppInfo()
{
	FUNCTIONSETUP;

	if( fDatabase && fDatabase->isOpen() )
	{
		d->fMemoInfo = new PilotMemoInfo( fDatabase );

		return d->fMemoInfo;
	}

	return 0;
}
