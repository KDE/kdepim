/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the abstract base class for databases, which is used both
** by local databases and by the serial databases held in the Pilot.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <time.h> // Needed by pilot-link include
#include <pi-appinfo.h>

#include <qstringlist.h>
#include <qtextcodec.h>

#include "pilotDatabase.h"
#include "pilotAppCategory.h"

static int creationCount = 0;
static QStringList *createdNames = 0L;

PilotDatabase::PilotDatabase(const QString &s) :
	fDBOpen(false),
	fName(s)
{
	FUNCTIONSETUP;
	creationCount++;
	if (!createdNames)
	{
		createdNames = new QStringList();
	}
	createdNames->append(s.isEmpty() ? CSL1("<empty>") : s);
}

/* virtual */ PilotDatabase::~PilotDatabase()
{
	FUNCTIONSETUP;
	creationCount--;
	if (createdNames)
	{
		createdNames->remove(fName.isEmpty() ? CSL1("<empty>") : fName);
	}
}

/* static */ int PilotDatabase::count()
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGDAEMON << fname << ": " << creationCount << " databases." << endl;
	if (createdNames)
	{
		DEBUGDAEMON << fname << ": "
			<< createdNames->join(CSL1(",")) << endl;
	}
#endif
	return creationCount;
}

/* virtual */ RecordIDList PilotDatabase::idList()
{
	RecordIDList l;

	for (unsigned int i = 0 ; ; i++)
	{
		PilotRecord *r = readRecordByIndex(i);
		if (!r) break;
		l.append(r->id());
		delete r;
	}

	return l;
}

/* virtual */ RecordIDList PilotDatabase::modifiedIDList()
{
	RecordIDList l;

	resetDBIndex();
	while(1)
	{
		PilotRecord *r = readNextModifiedRec();
		if (!r) break;
		l.append(r->id());
		delete r;
	}

	return l;
}

int PilotAppInfoBase::findCategory(const QString &selectedCategory, 
	bool unknownIsUnfiled, struct CategoryAppInfo *info)
{
	FUNCTIONSETUP;

	int currentCatID = -1;
	for (int i=0; i<MAX_CATEGORIES; i++)
	{
		if (!info->name[i][0]) continue;
		if (selectedCategory == 
			PilotAppCategory::codec()->toUnicode(info->name[i]))
		{
			currentCatID = i;
			break;
		}
	}

#ifdef DEBUG
	if (-1 == currentCatID)
	{
		DEBUGKPILOT << fname << ": Category name " 
			<< selectedCategory << " not found." << endl;
	}
	else
	{
		DEBUGKPILOT << fname << ": Matched category " << currentCatID << endl;
	}
#endif

	if ((currentCatID == -1) && unknownIsUnfiled)
		currentCatID = 0;
	return currentCatID;
}

void PilotAppInfoBase::dump() const
{
	PilotAppCategory::dumpCategories(*categoryInfo());
}
