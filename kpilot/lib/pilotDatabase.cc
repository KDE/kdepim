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

#include <kglobal.h>

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

PilotAppInfoBase::PilotAppInfoBase(PilotDatabase *d) : fC(new struct CategoryAppInfo), fLen(0), fOwn(true)
{
	FUNCTIONSETUP;
	int appLen = MAX_APPINFO_SIZE;
	unsigned char buffer[MAX_APPINFO_SIZE];

	fLen = appLen = d->readAppBlock(buffer,appLen);
	unpack_CategoryAppInfo(fC, buffer, appLen);
} ;

PilotAppInfoBase::~PilotAppInfoBase()
{
	if (fOwn) delete fC;
} ;


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

// Eww. I don't know how to cleanly get the size of a field of
// a structure otherwise. Both of these constants _should_ be
// 16, which is checked in one of the test programs.
//
#define CATEGORY_NAME_SIZE (sizeof(((struct CategoryAppInfo *)0)->name[0]))
#define CATEGORY_COUNT     ( (sizeof(((struct CategoryAppInfo *)0)->name)) / CATEGORY_NAME_SIZE )

QString PilotAppInfoBase::category(unsigned int i)
{
	if (i>=CATEGORY_COUNT) return QString::null;
	return PilotAppCategory::codec()->toUnicode(categoryInfo()->name[i],CATEGORY_NAME_SIZE-1);
}

bool PilotAppInfoBase::setCategory(unsigned int i, const QString &s)
{
	if (i>=CATEGORY_COUNT) return false;
	int len = CATEGORY_NAME_SIZE - 1;
	QCString t = PilotAppCategory::codec()->fromUnicode(s,len);
	memset(categoryInfo()->name[i],0,CATEGORY_NAME_SIZE);
	qstrncpy(categoryInfo()->name[i],t,kMin(len,(int)CATEGORY_NAME_SIZE));
	return true;
}


