#ifndef RESOLUTIONTABLE_H
#define RESOLUTIONTABLE_H
/* resolutionTable.h			KPilot
**
** Copyright (C) 2003 by Reinhold Kainhofer
**
** See the .cc file for an explanation of what this file is for.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include <qvaluelist.h>
#include "syncAction.h"

typedef enum eExistItems {
	eExistsPC=0x1, eExistsPalm=0x2, eExistsBackup=0x4,
	eExistsAll=eExistsPC|eExistsPalm|eExistsBackup
};

class ResolutionItem
{
public:
	enum eExistItems fExistItems;
	QString fEntries[3];
	QString fResolved;
	QString fName;
public:
	ResolutionItem() {}
	ResolutionItem(QString name, int ex, QString pc, QString palm, QString backup):fExistItems((eExistItems)ex),fName(name)
		{fEntries[0]=pc;fEntries[1]=palm; fEntries[2]=backup; /*fExistItems=(eExistItems)ex;*/ }
	~ResolutionItem() {}
};

/**
@author Reinhold Kainhofer
*/
class ResolutionTable : public QPtrList<ResolutionItem>
{
public:
	ResolutionTable():QPtrList<ResolutionItem>() {fResolution=SyncAction::eAskUser;};

	~ResolutionTable() {};

	SyncAction::eConflictResolution fResolution;
	QString labels[3];
	enum eExistItems fExistItems;
};

#endif

