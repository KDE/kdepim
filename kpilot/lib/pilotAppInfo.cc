/* pilotAppInfo.cc		KPilot
**
** Copyright (C) 2005-2006 Adriaan de Groot <groot@kde.org>
**
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


#include "options.h"

#include <stdio.h>

#include "pilotAppInfo.h"

PilotAppInfoBase::PilotAppInfoBase(PilotDatabase *d) :
	fC( 0L ),
	fLen(0),
	fOwn(true)
{
	FUNCTIONSETUP;
	int appLen = Pilot::MAX_APPINFO_SIZE;
	unsigned char buffer[Pilot::MAX_APPINFO_SIZE];

	if (!d || !d->isOpen())
	{
		WARNINGKPILOT << "Bad database pointer." << endl;
		fLen = 0;
		KPILOT_DELETE( fC );
		return;
	}

	fC = new struct CategoryAppInfo;
	fLen = appLen = d->readAppBlock(buffer,appLen);
	unpack_CategoryAppInfo(fC, buffer, appLen);
}

PilotAppInfoBase::~PilotAppInfoBase()
{
	if (fOwn)
	{
		delete fC;
	}
}

bool PilotAppInfoBase::setCategoryName(unsigned int i, const QString &s)
{
	if ( (i>=Pilot::CATEGORY_COUNT) || // bad category number
		(!categoryInfo())) // Nowhere to write to
	{
		return false;
	}

	(void) Pilot::toPilot(s, categoryInfo()->name[i], Pilot::CATEGORY_SIZE - 1);
	return true;
}


