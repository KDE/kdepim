/* pilotDatabase.cc			KPilot
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <time.h> // Needed by pilot-link include

#include <pi-appinfo.h>

#include "pilotDatabase.moc"

PilotDatabase::PilotDatabase(QObject *o,const char *n) :
	QObject(o,n),
	fDBOpen(false)
{
	FUNCTIONSETUP;
}

/* virtual */ PilotDatabase::~PilotDatabase()
{
	FUNCTIONSETUP;
}

/* static */ void PilotDatabase::listAppInfo(const struct CategoryAppInfo *category)
{
#ifdef DEBUG
	FUNCTIONSETUP;

	for (int i = 0; i < 15; i++)
	{
		DEBUGKPILOT << fname
			<< ": Category #"
			<< i
			<< " has ID "
			<< (int) category->ID[i]
			<< " and name "
			<< (category->name[i][0] ? "*" : "-")
			<< category->name[i] << endl;
	}
#endif
}
